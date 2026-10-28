#include "Mjolnir.h"
#include "SmartRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Actors/Capabilities/Caller.h"
#include "Assets.h"
#include "settings.h"
#include "util.h"

using namespace Util;

Mjolnir::~Mjolnir()
{
    ResetCharge(enchMag, defaultEnchMag, false, true);
    spdlog::debug("leviathan destructed");
}
Mjolnir::Mjolnir(RE::TESBoundObject* a_object)
    : SmartRelicWeapon(a_object)
{
    Initialize();
}

bool Mjolnir::Initialize()
{
    bool found = true;
    auto dataHandler    = RE::TESDataHandler::GetSingleton();
    defaultEnch         = dataHandler->LookupForm<RE::EnchantmentItem> (0x84B, Config::KratosCombatESP);    // +30 frost damage, ff, touch
    if (defaultEnch) {
            spdlog::debug("Default Mjolnir charging enchantment is {}", defaultEnch->GetName());
    } else {
        spdlog::error("Can't find default Mjolnir enchantment spell"); found = false;
    }

    ThrowableWeaponDummyProjectile->data.gravity = 3.21f;

    const auto assets = Assets::GetSingleton();
    soundData = 
        SoundDefinition{
            .chargingLoop0 = assets->soundEffects.chargeLeviLoop,
            .throwingStart = assets->soundEffects.throwLevi,
            .throwingLoop0 = assets->soundEffects.throwingLeviLoop0,
            .throwingLoop1 = assets->soundEffects.throwingLeviLoop1,
            .callStart     = assets->soundEffects.callLevi,
            .arrivingStart = assets->soundEffects.arrivingLeviStart,
            .arrivingLoop0 = assets->soundEffects.arrivingLeviLoop0,
            .arrivingLoop1 = assets->soundEffects.arrivingLeviLoop1,
            .arrivingLoop2 = assets->soundEffects.arrivingLeviLoop2,
            .arrivingNear  = assets->soundEffects.arrivingLeviNear,
            .catching      = assets->soundEffects.catchLevi
        };

    return found;
}
void Mjolnir::Update()
{
    SmartRelicWeapon::Update();
}

void Mjolnir::SetState(RelicWeaponState::Type a_type)
{
    switch (a_type)
    {
    case RelicWeaponState::Type::kArriving:
        SmartRelicWeapon::SetState(std::make_unique<MjolnirArrivingState>(*this));
        break;
    case RelicWeaponState::Type::kHoming:
        SmartRelicWeapon::SetState(std::make_unique<MjolnirHomingState>(*this));
        break;

    default:
        break;
    }
}
void Mjolnir::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        soundData.PauseAllLoopingSounds();
    } else {
        soundData.ContinueAllLoopingSounds();
    }
}
bool Mjolnir::PreThrow()
{
    ThrowableWeaponDummyProjectile->data.speed *= Config::LeviThrowSpeedMult;
    return true;
}
void Mjolnir::PostThrow()
{
    auto rHandBone = throwerParent->GetRHandBone();
    if (!rHandBone) {spdlog::error("Mjolnir::Throw - RHandBone is null"); return;}

    auto throwerActor = throwerParent ? throwerParent->GetActor() : nullptr;
    if (!throwerActor) {spdlog::error("Mjolnir::Throw - thrower actor is null"); return;}

    if (!projectile) {spdlog::error("Mjolnir::Throw - projectile is null"); return;}
    auto& projectileRTD = projectile->GetProjectileRuntimeData();

    {
        {
            gravity = 3.21f;
            gravity /= std::max(std::powf(throwingChargeDuration + 1.f, 3.f), 0.01f);
            gravity = std::max(gravity, 0.5f);
        }

        RE::EnchantmentItem* enchantItem = nullptr;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
            enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);
        else
            enchantItem = nullptr;

        _isLastThrowCharged = enchantItem != nullptr;

        const auto thrower = throwerParent->GetCapabilityAs<Thrower>();
        if (thrower && thrower->IsThrowing(ThrowType::kPowerThrow))
            isPenetrating = true;
        else 
            isPenetrating = false;

    //    if (isPenetrating) return;

        throwerActor->SetGraphVariableBool("bLeviInCatchRange", false);

        SetThrowState(ThrowState::kThrown);
        if (stuckedBone)   stuckedBone.reset();
        if (stuckedActor)  stuckedActor.reset();
        lastHitActors.clear();
        lastHitForms.clear();
    }
}

bool Mjolnir::Call(Caller* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    return SmartRelicWeapon::Call(a_caller, a_justDestroy, a_delay);
}
void Mjolnir::Catch(const bool a_justDestroy)
{
    SmartRelicWeapon::Catch(a_justDestroy);
}
void Mjolnir::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_stage, const uint8_t a_coolDown)
{
    if (!GetWielder() || !GetWielder()->IsValid()) {
        spdlog::debug("charger is invalid");
        return;
    }

    auto assets     = Assets::GetSingleton();
    auto AnArchos   = GetWielder()->GetActor();
    const auto rHandBone = wielder->GetRHandBone();

    if (a_stage != 0u) {
        if (auto handEffect = assets->VFXeffects.handFrost; handEffect)
            AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, rHandBone, false);
        if (auto soundEffect = assets->soundEffects.chargeLevi; soundEffect)
            ObjectUtil::Sound::PlaySound(soundEffect, rHandBone, 5.f);
        return;
    }

    auto ench       = weap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, weap) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto leviDam  = damage;
    const auto weapBone = wielder->GetWeaponBone();

    if (weap && chargeHitCount <= 0) {
        if (enchBase) {
            spdlog::debug("levi's enchantment is: {}", ench->GetName());
            if (enchBase->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)
             || enchBase->HasArchetype(RE::EffectSetting::Archetype::kValueModifier)) {
                ResetCharge(enchMag, defaultEnchMag, false, true);
                auto& projectileDamage = enchEffect->effectItem.magnitude;
                enchMag = &projectileDamage;
                defaultEnchMag = projectileDamage;
                projectileDamage *= a_magnitude;
                chargeHitCount = a_chargeHitCount;
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                _isCharged = true;
                spdlog::debug("magnitude buffing from {} to: {}", projectileDamage / a_magnitude, projectileDamage);

                if (auto handEffect = assets->VFXeffects.handFrostBright; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                else spdlog::warn("can't found hand effect for levi charge!");

                if (auto soundEffect = assets->soundEffects.chargeLeviEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
            } else spdlog::debug("levi's enchantment is not expected archetype.");
#ifdef EXPERIMENTAL_CHARGE_LEVI
        } else {
            spdlog::debug("levi not has any enchantment, levi is enchanting...");
            if (defaultEnch) {
                ench = defaultEnch;
                enchEffect  = ench ? ench->effects[0] : nullptr;
                enchBase     = enchEffect ? enchEffect->baseEffect : nullptr;
                if (enchBase) {
                    ResetCharge(enchMag, defaultEnchMag, false, true);
                    ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 300.f, false, false);
                    ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                    _isCharged = true;

                //  AnArchos->GetActorRuntimeData().emotionType = RE::EmotionType::kAnger;
                //  AnArchos->GetActorRuntimeData().emotionValue = 100;

                    auto& enchCost = ench->data.costOverride;
                    auto& enchAmount = ench->data.chargeOverride;
                    auto& projectileDamage = enchEffect->effectItem.magnitude;
                    enchAmount = 500.f;
                    projectileDamage = a_magnitude * leviDam / 2;
                    enchCost = projectileDamage;
                    enchMag = nullptr;

                    chargeHitCount = a_chargeHitCount;
                    spdlog::info("levi charge frost damage buff is: {}", projectileDamage);

                    if (auto handEffect = assets->VFXeffects.handFrostBright; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                    else spdlog::warn("can't found hand effect for levi charge!");

                    if (auto soundEffect = assets->soundEffects.chargeLeviEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
                } else spdlog::error("can't find frost enchantment's base!!");
            } else spdlog::error("can't find frost enchantment!!");
#endif
        }
    } else spdlog::error("can't found levi for charging!!");
}
void Mjolnir::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    auto assets = Assets::GetSingleton();
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
}
