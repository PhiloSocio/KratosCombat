#include "Mjolnir.h"
#include "Weapons/ThrownSmartWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Assets.h"
#include "settings.h"
#include "util.h"

using namespace Util;

Mjolnir::Mjolnir(RE::TESBoundObject* a_object)
    : SmartRelicWeapon(a_object)
{
    Initialize();
}
Mjolnir::~Mjolnir()
{
    ResetCharge(enchMag, defaultEnchMag, false, true);
    spdlog::debug("mjolnir destructed");
}

bool Mjolnir::Initialize()
{
    bool found = true;
    auto dataHandler    = RE::TESDataHandler::GetSingleton();
    defaultEnch         = dataHandler->LookupForm<RE::EnchantmentItem> (0x84C, Config::KratosCombatESP);    // +30 shock damage, ff, touch
    if (defaultEnch) {
        spdlog::debug("Default Mjolnir charging enchantment is {}", defaultEnch->GetName());
    } else {
        spdlog::error("Can't find default Mjolnir enchantment spell"); found = false;
    }

    if (ThrowableWeaponDummyProjectile)
        ThrowableWeaponDummyProjectile->data.gravity = 3.21f;
    else
        found = false;

    const auto assets = Assets::GetSingleton();
    if (!assets) return false;
    soundData =
        SoundDefinition{
            .chargingLoop0 = assets->soundEffects.chargeMjolnir,
            .throwingStart = assets->soundEffects.throwLevi,
            .throwingLoop0 = assets->soundEffects.throwingLeviLoop0,
            .throwingLoop1 = assets->soundEffects.throwingLeviLoop1,
            .callStart     = assets->soundEffects.fingerSnap,
            .arrivingStart = assets->soundEffects.arrivingLeviStart,
            .arrivingLoop0 = assets->soundEffects.arrivingLeviLoop0,
            .arrivingLoop1 = assets->soundEffects.arrivingLeviLoop1,
            .arrivingLoop2 = assets->soundEffects.arrivingLeviLoop2,
            .arrivingNear  = assets->soundEffects.arrivingLeviNear,
            .catching      = assets->soundEffects.catchLevi
        };

    return found;
}
void Mjolnir::SetState(ThrownSmartWeapon& a_weapon, RelicWeaponState::Type a_type)
{
    switch (a_type) {
    case RelicWeaponState::Type::kArriving:
        a_weapon.SetState(std::make_unique<MjolnirArrivingState>(a_weapon));
        break;
    case RelicWeaponState::Type::kHoming:
        a_weapon.SetState(std::make_unique<MjolnirHomingState>(a_weapon));
        break;
    case RelicWeaponState::Type::kNone:
    default:
        a_weapon.SetState(std::unique_ptr<RelicWeaponState>{});
        break;
    }
}
bool Mjolnir::PreThrow(ThrownWeapon& a_weapon, RE::Projectile::LaunchData& a_launchData)
{
    a_weapon.ThrowableWeaponDummyProjectile->data.speed *= Config::LeviThrowSpeedMult;
    a_weapon.aeroDragCoeff = 4.e-4f;
    return true;
}
void Mjolnir::PostThrow(ThrownWeapon& a_weapon)
{
    auto throwerActor = a_weapon.GetThrowerActor();
    if (!throwerActor) {spdlog::error("Mjolnir::Throw - thrower actor is null"); return;}

    a_weapon.gravity = 3.21f;
    a_weapon.gravity /= std::max(std::powf(a_weapon.throwingChargeDuration + 1.f, 3.f), 0.01f);
    a_weapon.gravity = std::max(a_weapon.gravity, 0.5f);

    a_weapon.isLastThrowCharged = a_weapon.ench != nullptr;
    a_weapon.trailData.trailOverride.baseColorOverride = a_weapon.trailData.GetColorByIndex(Config::TrailColorIndexMjolnir);

    const auto thrower = a_weapon.throwerParent->GetCapabilityAs<Thrower>();
    a_weapon.isPenetrating = thrower && thrower->IsThrowing(ThrowType::kPowerThrow);
    throwerActor->SetGraphVariableBool("bLeviInCatchRange", false);
    a_weapon.SetThrowState(ThrowState::kThrown);
    auto weapon = a_weapon.As<ThrownSmartWeapon>();
    if (!weapon) return;
    weapon->stuckedBone.reset();
    weapon->stuckedActor.reset();
    weapon->lastHitActors.clear();
    weapon->lastHitForms.clear();
}
bool Mjolnir::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_stage, const uint8_t a_coolDown)
{
    if (!GetWielder() || !GetWielder()->IsValid()) {
        spdlog::debug("charger is invalid");
        return false;
    }

    auto assets   = Assets::GetSingleton();
    auto AnArchos = GetWielder()->GetActor();
    ench = weap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, weap) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto mjolnirDam  = damage;

    if (weap) {
        const auto weapBone = GetWielder()->GetWeaponBone();

        if (auto handEffect = assets->VFXeffects.handShock; handEffect)
            AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
        if (auto soundEffect = assets->soundEffects.chargeMjolnirEnd; soundEffect)
            ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
        if (assets->vanillaSpells.thunderbolt || assets->kittyTailSpells.lightningFlood) {
            RE::ProjectileHandle pHandle, pHandle2;
            auto origin = weapBone->world.translate + RE::NiPoint3(0.f, 0.f, 2000.f);
            auto origin2 = weapBone->world.translate;
            auto pRot = RE::Projectile::ProjectileRot(PI2, 0.f);
            auto pRot2 = RE::Projectile::ProjectileRot(-PI2, 0.f);
#ifdef OVERPOWERED
            RE::Projectile::LaunchData lData(AnArchos, origin, pRot, assets->kittyTailSpells.lightningFlood ? assets->kittyTailSpells.lightningFlood : assets->vanillaSpells.thunderbolt);
#else
            RE::Projectile::LaunchData lData(AnArchos, origin, pRot, assets->kittyTailSpells.lightningFlood && a_stage == 3u ? assets->kittyTailSpells.lightningFlood : assets->vanillaSpells.thunderbolt);
#endif
            RE::Projectile::LaunchData lData2(AnArchos, origin2, pRot2, assets->vanillaSpells.lightningBolt);
            lData.noDamageOutsideCombat = true;
            lData2.noDamageOutsideCombat = true;
#ifdef OVERPOWERED
            RE::Projectile::Launch(&pHandle, lData);
#else
            if (a_stage == 3u) RE::Projectile::Launch(&pHandle, lData);
#endif
            RE::Projectile::Launch(&pHandle2, lData2);
            ObjectUtil::Actor::SendAnimationEvent(AnArchos, "weaponSwing");
            if (a_stage >= 2u) {
                auto combatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(AnArchos, 333.f, true);
                uint8_t launchedProjectileCount = 0;
                const uint8_t projectileCount = a_stage * 2u;
                const float angleStep = TWO_PI / projectileCount;
                for (auto& target : combatTargets) {
                    RE::ProjectileHandle pHandle3;
                    auto targetPos = target->GetNodeByName("NPC Spine2 [Spn2]") ? target->GetNodeByName("NPC Spine2 [Spn2]")->world.translate : (target->GetPosition() + (target->GetBoundMax() + target->GetBoundMin()) / 2.f);
                    auto direction = targetPos - origin2;
                    direction.Unitize();
                    RE::Projectile::ProjectileRot pRot3(MathUtil::Algebra::VectorToPitchYaw(direction));
                    RE::Projectile::LaunchData lData3(AnArchos, origin2, pRot3, assets->vanillaSpells.lightningBolt);
                    lData3.autoAim = false;
                    lData3.alwaysHit = true;
                    lData3.desiredTarget = target;
                    RE::Projectile::Launch(&pHandle3, lData3);
                    launchedProjectileCount++;
                    if (launchedProjectileCount >= projectileCount) break; //  limit the projectiles count
                }
                for (float i = launchedProjectileCount * angleStep; launchedProjectileCount < projectileCount && i < 6.28f; i += angleStep) {
                    RE::ProjectileHandle pHandle3;
                    auto origin3 = weapBone->world.translate;
                    RE::Projectile::ProjectileRot pRot3(PI8, i);
                    RE::Projectile::LaunchData lData3(AnArchos, origin3, pRot3, assets->vanillaSpells.lightningBolt);
                    lData3.autoAim = false;
                    lData3.alwaysHit = true;    //  otherwise it will not hit even colliding with a target
                    RE::Projectile::Launch(&pHandle3, lData3);
                    launchedProjectileCount++;
                    if (launchedProjectileCount >= projectileCount) break; //  limit the projectiles count
                }
            }
        }

        if (enchBase && 
            (enchBase->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier) || 
             enchBase->HasArchetype(RE::EffectSetting::Archetype::kValueModifier))) {
            if (ench == defaultEnch) {
                spdlog::debug("mjolnir is already charged with {}, buffing now...", ench->GetName());
                ResetCharge(enchMag, defaultEnchMag, false, true);
                ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 100.f * (float)a_stage, false, false);
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 100.f * (float)a_stage);
                auto& enchCost = ench->data.costOverride;
                auto& enchAmount = ench->data.chargeOverride;
                auto& mag = enchEffect->effectItem.magnitude;
                const float defMag = mag;
                enchAmount = 200.f * (float)a_stage;
                mag = a_magnitude * ((float)a_stage / 3.f) * (mjolnirDam / 2.f);
                enchCost = mag;
                enchMag = nullptr;
                if (mag > GetWielder()->GetDamageMult() * weap->attackDamage) {
                    mag = GetWielder()->GetDamageMult() * weap->attackDamage;
                    spdlog::warn("mjolnir's charge damage is too high, capped to: {}", mag);
                } else spdlog::debug("magnitude buffed from {} to: {}", defMag, mag);
                _isCharged = true;
                chargeHitCount = a_chargeHitCount;
            } else {
                spdlog::debug("mjolnir's enchantment is: {}", ench->GetName());
                ResetCharge(enchMag, defaultEnchMag, false, true);
                auto& mag = enchEffect->effectItem.magnitude;
                const float defMag = mag;
                enchMag = &mag;
                defaultEnchMag = mag;
                mag = a_magnitude * ((float)a_stage / 3.f) * (mjolnirDam / 2.f);
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 100.f * (float)a_stage);
                _isCharged = true;
                chargeHitCount = a_chargeHitCount;
                spdlog::debug("magnitude buffed from {} to: {}", defMag, mag);
            }
#ifdef EXPERIMENTAL_CHARGE_LEVI
        } else {
            spdlog::debug("mjolnir not has any damage enchantment, mjolnir is enchanting...");
            if (defaultEnch) {
                ench = defaultEnch;
                enchEffect  = ench ? ench->effects[0] : nullptr;
                enchBase    = enchEffect ? enchEffect->baseEffect : nullptr;
                if (enchBase) {
                    ResetCharge(enchMag, defaultEnchMag, false, true);
                    ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 100.f * (float)a_stage, false, false);
                    ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 100.f * (float)a_stage);
                    _isCharged = true;

                    auto& enchCost = ench->data.costOverride;
                    auto& enchAmount = ench->data.chargeOverride;
                    auto& mag = enchEffect->effectItem.magnitude;
                    enchAmount = 200.f * (float)a_stage;
                    mag = a_magnitude * ((float)a_stage / 3.f) * (mjolnirDam / 2.f);
                    enchCost = mag;
                    enchMag = nullptr;

                    chargeHitCount = a_chargeHitCount;
                    spdlog::info("mjolnir charge shock damage buff is: {}", mag);
                } else spdlog::error("can't find shock enchantment's base!!");
            } else spdlog::error("can't find shock enchantment!!");
#endif
        }
    } else spdlog::error("can't found mjolnir for charging!!");
    return _isCharged;
}
bool Mjolnir::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    if (a_justReset) {
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), weap) == defaultEnch) {
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), weap);
            _isCharged = false;
        }
    } else {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), weap) == defaultEnch)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), weap);
            }
            _isCharged = false;
        } else if (!a_justCheck) {chargeHitCount -= 1;}
    }
    if (GetWielder() && GetWielder()->IsValid())
        GetWielder()->GetActor()->SetGraphVariableBool("IsWeaponCharged", _isCharged);
    return _isCharged;
}
