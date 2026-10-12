#include "Leviathan.h"
#include "RelicManager.h"
#include "Actors/Capabilities/Caller.h"

using namespace Util;

LeviathanAxe::~LeviathanAxe()
{
    ResetCharge(enchMag, defaultEnchMag, false, true);
    spdlog::debug("leviathan destructed");
}
LeviathanAxe::LeviathanAxe(RE::TESBoundObject* a_object)
    : SmartRelicWeapon(a_object)
{
}

bool LeviathanAxe::Initialize()
{
    bool found = true;
    auto dataHandler    = RE::TESDataHandler::GetSingleton();
    LeviProjBaseL       = dataHandler->LookupForm<RE::BGSProjectile>   (0x815, Config::KratosCombatESP);
    LeviProjBaseH       = dataHandler->LookupForm<RE::BGSProjectile>   (0x816, Config::KratosCombatESP);
    LeviProjBaseA       = dataHandler->LookupForm<RE::BGSProjectile>   (0x822, Config::KratosCombatESP);
    SpellLeviProjL      = dataHandler->LookupForm<RE::SpellItem>       (0x811, Config::KratosCombatESP);
    SpellLeviProjH      = dataHandler->LookupForm<RE::SpellItem>       (0x813, Config::KratosCombatESP);
    SpellCatchLevi      = dataHandler->LookupForm<RE::SpellItem>       (0x81D, Config::KratosCombatESP);
    SpellLeviProjA      = dataHandler->LookupForm<RE::SpellItem>       (0x823, Config::KratosCombatESP);
    EnchCharge          = dataHandler->LookupForm<RE::EnchantmentItem> (0x84B, Config::KratosCombatESP);    // +30 frost damage, ff, touch
    if (LeviProjBaseL && LeviProjBaseH && LeviProjBaseA)
            spdlog::debug("Leviathan Axe projectiles are {}, {} and {}", LeviProjBaseL->GetName(), LeviProjBaseH->GetName(), LeviProjBaseA->GetName());
    else     {spdlog::error("Can't find Leviathan Axe projectiles"); found = false;}
    if (SpellLeviProjL && SpellLeviProjH)   
            spdlog::debug("Leviathan Axe throwing spells are {} and {}", SpellLeviProjL->GetName(), SpellLeviProjH->GetName());
    else     {spdlog::error("Can't find Leviathan Axe projectile spells"); found = false;}
    if (SpellLeviProjA)
            spdlog::debug("Leviathan Axe calling spell is {}", SpellLeviProjA->GetName());
    else     {spdlog::error("Can't find Leviathan Axe calling spell"); found = false;}
    if (SpellCatchLevi) {
            EffCatchLevi = SpellCatchLevi->effects[0]->baseEffect;
            spdlog::debug("Leviathan Axe catching spell is {}", SpellCatchLevi->GetName()); EffCatchLevi = SpellCatchLevi->effects[0]->baseEffect;}
    else     {spdlog::error("Can't find Leviathan Axe catching spell"); found = false;}
    if (EnchCharge) {           
            spdlog::debug("Default Leviathan Axe charging enchantment is {}", EnchCharge->GetName());}
    else     {spdlog::error("Can't find default Leviathan Axe enchantment spell"); found = false;}

    return found;
}
void LeviathanAxe::Update()
{
    SmartRelicWeapon::Update();
}

void LeviathanAxe::SetState(RelicWeaponState::Type a_type)
{
    switch (a_type)
    {
    case RelicWeaponState::Type::kArriving:
        {
            auto previous = dynamic_cast<LeviathanArrivingState*>(currentState.get());
            RE::NiPoint3 startPoint; GetPosition(startPoint);
            if (isPenetrating && previous) {
                SmartRelicWeapon::SetState(std::make_unique<LeviathanArrivingState>(*previous, startPoint));
            } else {
                SmartRelicWeapon::SetState(std::make_unique<LeviathanArrivingState>(*this, startPoint, &callerParent->GetRHandBone()));
                spdlog::debug("Levi call is started");
            }
        }
        break;
    case RelicWeaponState::Type::kHoming:
        SmartRelicWeapon::SetState(std::make_unique<LeviathanHomingState>(*this));
        break;

    default:
        break;
    }
}
void LeviathanAxe::OnMenuOpenCloseEvent(const bool a_opening)
{
    if (a_opening) {
        soundData.PauseAllLoopingSounds();
    } else {
        soundData.ContinueAllLoopingSounds();
    }
}
bool LeviathanAxe::PreThrow()
{
    ThrowableWeaponDummyProjectile->data.speed = Config::ThrowSpeed;
    return true;
}
void LeviathanAxe::PostThrow()
{
    auto rHandBone = throwerParent->GetRHandBone();
    if (!rHandBone) {spdlog::error("LeviathanAxe::Throw - RHandBone is null"); return;}

    auto throwerActor = throwerParent ? throwerParent->GetActor() : nullptr;
    if (!throwerActor) {spdlog::error("LeviathanAxe::Throw - thrower actor is null"); return;}

    if (!projectile) {spdlog::error("LeviathanAxe::Throw - projectile is null"); return;}
    auto& projectileRTD = projectile->GetProjectileRuntimeData();

    {   //  calculate damage
        auto& projectileDamage = projectileRTD.weaponDamage;
        float throwChargeDamageMult = std::sqrtf(throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        projectileDamage *= throwChargeDamageMult;

        if (ThrowableWeaponDummyProjectile) {
        //  //  ThrowableWeaponDummyProjectile->defaultWeaponSource = weap;
            auto& pbData = ThrowableWeaponDummyProjectile->data;
            pbData.speed = !isPenetrating ? Config::ThrowSpeed * std::clamp(throwChargeDamageMult / 2.f, 1.f, 1.25f) : pbData.speed * 0.7f;
            pbData.force = projectileDamage;
            pbData.gravity = 3.21f;
        } else spdlog::warn("not found Levi throwing effect!");

        if (!isPenetrating) {
            soundData.PlayThrowingSounds(rHandBone);

            gravity = 3.21f;
            gravity /= (std::powf(throwingChargeDuration + 1.f, 3.f));
            gravity = std::max(gravity, 0.5f);
        }

        RE::EnchantmentItem* enchantItem = nullptr;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(throwerActor) > 0.f)
            enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(throwerActor);
        else
            enchantItem = nullptr;

        _isLastThrowCharged = enchantItem != nullptr;

        if (isPenetrating) return;
/*
        if (Config::IsAdvancedThrowingInstalled) {
            ResetCharge(enchMag, defaultEnchMag, true);
            throwerParent->SetSkipEquipAnim(true);
            ObjectUtil::Actor::UnEquipItem(throwerActor, false, false, true, true, throwerParent->GetSkipEquipAnim(), true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, throwerActor);
            spdlog::debug("Leviathan unequipped after throwing");
        } else {
    //        WeaponIdentify::isLeviathanAxe = false;
    //        WeaponIdentify::isRelic = false;
        //    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        //    throwerActor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            throwerParent->SetSkipEquipAnim(true);
            throwerParent->SetUnequipWhenAnimEnds(true);
        }
*/
        throwerActor->SetGraphVariableBool("bLeviInCatchRange", false);

        isAxeCalled = false;
        isAxeThrowed = true;
        SetThrowState(ThrowState::kThrown);
            spdlog::info("Leviathan Axe throwed, raw damage is: {}", projectileDamage);
        if (stuckedBone)   stuckedBone    = nullptr;
        if (stuckedActor)  stuckedActor   = nullptr;
        lastHitActors.clear();
        lastHitForms.clear();
        if (throwerActor->HasSpell(SpellCatchLevi)) throwerActor->RemoveSpell(SpellCatchLevi);
    }
}

bool LeviathanAxe::Call(Caller* a_caller, const bool a_justDestroy, std::optional<float> a_delay)
{
    return SmartRelicWeapon::Call(a_caller, a_justDestroy, a_delay);
}
void LeviathanAxe::Catch(const bool a_justDestroy)
{
    SmartRelicWeapon::Catch(a_justDestroy);
}
void LeviathanAxe::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_stage, const uint8_t a_coolDown)
{
    if (!GetWielder() || !GetWielder()->IsValid()) return;

    auto assets     = Assets::GetSingleton();
    auto AnArchos   = GetWielder()->GetActor();
    const auto rHandBone = wielder->GetRHandBone();

    if (a_stage != -1u) {
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
            if (EnchCharge) {
                ench = EnchCharge;
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
void LeviathanAxe::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    auto assets = Assets::GetSingleton();
    if (a_justReset) {
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), weap) == EnchCharge) {
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), weap);
            _isCharged = false;
        }
    } else {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), weap) == EnchCharge)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), weap);
            }
            _isCharged = false;
        } else if (!a_justCheck) {chargeHitCount -= 1;}
    }
}
