#include "MainKratosCombat.h"
//#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;
using tState = LeviathanAxe::ThrowState;
using tStateM = Mjolnir::ThrowState;

static void DelayedCast(RE::Actor* a_caster, RE::SpellItem* a_spell, RE::NiNode* a_target, const float a_delaySec)
{
    if (!a_spell) return;
    if (!a_caster) return;
    int delay = static_cast<int>(a_delaySec * 1000.f);
    spdlog::debug("draupnir's call start...");
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    if (!a_spell) return;
    if (!a_caster) return;
    spdlog::debug("draupnir's call triggered!");
//  a_caster->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(a_spell, false, nullptr, 1.f, false, 10.f, a_caster);
    
    if (!a_target) return;
    RE::ProjectileHandle pHandle;
    RE::NiPoint3 startPoint = a_target->world.translate + RE::NiPoint3(0.f, 0.f, 128.f);
    RE::Projectile::ProjectileRot pRot = {a_caster->GetAimAngle(), a_caster->GetAimHeading()};
    RE::Projectile::LaunchSpell(&pHandle, a_caster, a_spell, startPoint, pRot);
    spdlog::debug("draupnir's call end!");
}
static void SkipEquipAnimationDuring(int a_durationMS, RE::Actor* a_actor, int a_load3dDelayMS = (int)(*g_deltaTimeRealTime * 1000.f), const bool a_skip3DLoading = false)
{
    if (a_actor) {
        ObjectUtil::Actor::SkipEquipAnimation(a_actor, true, a_load3dDelayMS, a_skip3DLoading);
        if (a_durationMS < (int)(*g_deltaTimeRealTime * 2.f * 1000.f)) a_durationMS = (int)(*g_deltaTimeRealTime * 2.f * 1000.f);
        std::jthread DisableEquipAnim([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(a_durationMS));
            ObjectUtil::Actor::SkipEquipAnimation(a_actor, _skipEquipAnim, _load3Ddelay, _skipLoad3D);
        });
        DisableEquipAnim.detach();
    }
}
static void ResetEquipAnimationAfter(int a_delayMS, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton())
{
    if (a_delayMS == 0) {ObjectUtil::Actor::SkipEquipAnimation(a_actor, _skipEquipAnim, _load3Ddelay, _skipLoad3D); return;}
    if (a_delayMS < (int)(*g_deltaTimeRealTime * 2.f * 1000.f)) a_delayMS = (int)(*g_deltaTimeRealTime * 2.f * 1000.f);
    std::jthread DisableEquipAnim([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(a_delayMS));
        ObjectUtil::Actor::SkipEquipAnimation(a_actor, _skipEquipAnim, _load3Ddelay, _skipLoad3D);
    });
    DisableEquipAnim.detach();
}
#pragma region Weapon_Identification
void WeaponIdentify::Initialize(RE::Actor* a_actor)
{
    lastEquippedRelic = Kratos::Relic::kNone;
    lastThrownRelic = Kratos::Relic::kNone;
    if (a_actor) {
        auto kratos = Kratos::GetSingleton();
        auto invChanges = a_actor->GetInventoryChanges();
        auto entries = invChanges->entryList;
        spdlog::info("Kratos's special weapons initializing...");
        for (auto entry : *entries) {
            if (entry && entry->object) {
                if (entry->object->IsWeapon()) {
#ifdef NEW_WEAPON_REGISTER_METHOD
#else
                    if (kratos->gLeviathanAxeFormID && entry->object->GetLocalFormID() == kratos->gLeviathanAxeFormID->value && entry->object->GetFile()->fileName == Config::registeredLeviathanFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::LeviathanAxeKWD);
                            spdlog::info("{} is your Leviathan Axe", weap->GetName());
                        }
                    } else if (kratos->gBladeOfChaosFormID && entry->object->GetLocalFormID() == kratos->gBladeOfChaosFormID->value && entry->object->GetFile()->fileName == Config::registeredBladeOfChaosFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            spdlog::info("{} is your Blade of Chaos", weap->GetName());
                        }
                    } else if (kratos->gBladeOfChaosFormID && entry->object->GetLocalFormID() == kratos->gBladeOfChaosFormID->value+1 && entry->object->GetFile()->fileName == Config::registeredLBladeOfChaosFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            spdlog::info("{} is your left Blade of Chaos", weap->GetName());
                        }
                    } else if (kratos->gDraupnirSpearFormID && entry->object->GetLocalFormID() == kratos->gDraupnirSpearFormID->value && entry->object->GetFile()->fileName == Config::registeredDraupnirFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::DraupnirSpearKWD);
                            spdlog::info("{} is your Draupnir Spear", weap->GetName());
                        }
                    } else if (kratos->gBladeOfOlympusFormID && entry->object->GetLocalFormID() == kratos->gBladeOfOlympusFormID->value && entry->object->GetFile()->fileName == Config::registeredBladeOfOlympusFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfOlympusKWD);
                            spdlog::info("{} is your Blade of Olympus", weap->GetName());
                        }
                    } else if (kratos->gMjolnirFormID && entry->object->GetLocalFormID() == kratos->gMjolnirFormID->value && entry->object->GetFile()->fileName == Config::registeredMjolnirFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::MjolnirKWD);
                            spdlog::info("{} is your Mjolnir", weap->GetName());
                        }
                    }
#ifdef TRIDENT
                    else if (kratos->gTridentFormID && entry->object->GetLocalFormID() == kratos->gTridentFormID->value && entry->object->GetFile()->fileName == Config::registeredTridentFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::TridentKWD);
                            spdlog::info("{} is your Trident", weap->GetName());
                        }
                    }
#endif
#endif
                } else if (entry->object->IsArmor()) {
#ifdef NEW_WEAPON_REGISTER_METHOD
#else
                    if (entry->object->GetLocalFormID() == kratos->gGuardianShieldFormID->value && entry->object->GetFile()->fileName == Config::registeredGuardianShieldFileName) {
                        if (auto shield = entry->object->As<RE::TESObjectARMO>(); shield) {
                            shield->RemoveKeywords(Config::SpecialKWDs);
                            shield->AddKeyword(Config::GuardianShieldKWD);
                            spdlog::info("{} is your Guardian Shield", shield->GetName());
                        }
                    }
#endif
                }
            } else spdlog::info("entry is not an object");
        } spdlog::info("Kratos's special weapons initialized.");
    }
}
void WeaponIdentify::ResetRegistrations()
{
    Config::registeredLeviathanID      = 105;
    Config::registeredBladeOfChaosID   = 105;
    Config::registeredLBladeOfChaosID  = 105;
    Config::registeredDraupnirID       = 105;
    Config::registeredBladeOfOlympusID = 105;
    Config::registeredMjolnirID        = 105;
    Config::registeredTridentID        = 105;
    Config::registeredGuardianShieldID = 105;
    Config::registeredLeviathanFileName      = "Not Registered";
    Config::registeredBladeOfChaosFileName   = "Not Registered";
    Config::registeredLBladeOfChaosFileName  = "Not Registered";
    Config::registeredDraupnirFileName       = "Not Registered";
    Config::registeredBladeOfOlympusFileName = "Not Registered";
    Config::registeredMjolnirFileName        = "Not Registered";
    Config::registeredTridentFileName        = "Not Registered";
    Config::registeredGuardianShieldFileName = "Not Registered";
}
void WeaponIdentify::WeaponIdentifier(RE::Actor* a_actor, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectWEAP* a_LHandWeapon, RE::TESObjectARMO* a_shield)
{
    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
    Config::SpecialShield->value = (uint8_t)Kratos::Shield::kNone;
    auto RelicName = "not a Relic";
//  auto address = reinterpret_cast<std::uintptr_t>(a_RHandWeapon);
    auto kratos = Kratos::GetSingleton();
    if (a_RHandWeapon) {
        const auto equippedWeaponFile = a_RHandWeapon->GetFile()->fileName;
        const FormID equippedWeaponID = a_RHandWeapon->GetLocalFormID();
        spdlog::debug("registering the {:8x} from {}", equippedWeaponID, equippedWeaponFile);
        if (kratos->IsInRage(a_actor) && !kratos->IsWantFinishRage())
            kratos->EndRage(kratos->GetLastTriggeredRageType(), false, false, false, a_actor);
        if (Config::LeviathanAxeKWD && a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {
            isLeviathanAxe = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Leviathan Axe";
            LeviathanAxe = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kLeviathanAxe;
            Config::registeredLeviathanFileName = equippedWeaponFile;
            Config::registeredLeviathanID = equippedWeaponID;
            kratos->gLeviathanAxeFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kLeviathanAxe;

            auto Levi = LeviathanAxe::GetSingleton();
            Levi->data.weap     = LeviathanAxe;
            Levi->data.ench     = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            Levi->data.poison   = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            Levi->data.damage   = static_cast<float>(LeviathanAxe->attackDamage);
#ifdef EXPERIMENTAL_LEVIATHAN_MODEL
            Levi->LeviProjBaseL->model = LeviathanAxe->model;
            Levi->LeviProjBaseH->model = LeviathanAxe->model;
            Levi->LeviProjBaseA->model = LeviathanAxe->model;
#endif
            if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {
                spdlog::debug("Levi is throwable");
				const auto root = a_actor->Get3D();
                auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
                Levi->data.weaponModel = weapon3D ? weapon3D->Clone() : nullptr;
                Levi->data.weaponModel->local = RE::NiTransform();
                Levi->data.weaponModel->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
                Levi->SetThrowState(tState::kThrowable);
                Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                kratos->SetIsCanCallAxe(a_actor, false);
            } else spdlog::warn("Levi is not equipped for real");

            if (Levi->SpellCatchLevi && a_actor->AsMagicTarget()->HasMagicEffect(Levi->EffCatchLevi)) {a_actor->RemoveSpell(Levi->SpellCatchLevi);}

            if (Levi->LeviathanAxeProjectileA 
            && (Levi->GetThrowState() == tState::kArriving
             || Levi->GetThrowState() == tState::kArrived)) 
                Levi->Catch(true);
        }
        else if (Config::BladeOfChaosKWD && a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {
            isBladeOfChaos = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Chaos";
            BladeOfChaos = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kBladeOfChaos;
            Config::registeredBladeOfChaosFileName = equippedWeaponFile;
            if (a_LHandWeapon && (a_LHandWeapon->HasKeyword(Config::BladeOfChaosKWD) || a_LHandWeapon->GetFile() == a_RHandWeapon->GetFile())) Config::registeredLBladeOfChaosFileName = a_LHandWeapon->GetFile()->fileName;
            Config::registeredBladeOfChaosID = equippedWeaponID;
            if (a_LHandWeapon && (a_LHandWeapon->HasKeyword(Config::BladeOfChaosKWD) || a_LHandWeapon->GetFile() == a_RHandWeapon->GetFile())) Config::registeredLBladeOfChaosID = a_LHandWeapon->GetLocalFormID();
            kratos->gBladeOfChaosFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kBladeOfChaos;
        }
        else if (Config::DraupnirSpearKWD && a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {
            isDraupnirSpear = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Draupnir Spear";
            DraupnirSpear = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kDraupnirSpear;
            Config::registeredDraupnirFileName = equippedWeaponFile;
            Config::registeredDraupnirID = equippedWeaponID;
            kratos->gDraupnirSpearFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kDraupnirSpear;
        }
        else if (Config::BladeOfOlympusKWD && a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {
            isBladeOfOlympus = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Olympus";
            BladeOfOlympus = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kBladeOfOlympus;
            Config::registeredBladeOfOlympusFileName = equippedWeaponFile;
            Config::registeredBladeOfOlympusID = equippedWeaponID;
            kratos->gBladeOfOlympusFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kBladeOfOlympus;
        }
        else if (Config::MjolnirKWD && a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {
            isMjolnir = true;
            isRelic = true;
            isThor = true;
            RelicName = "the Mjolnir";
            Mjolnir = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kMjolnir;
            Config::registeredMjolnirFileName = equippedWeaponFile;
            Config::registeredMjolnirID = equippedWeaponID;
            kratos->gMjolnirFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kMjolnir;

            auto mjolnir = Mjolnir::GetSingleton();
            mjolnir->data.weap      = Mjolnir;
            mjolnir->data.ench      = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            mjolnir->data.poison    = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            mjolnir->data.damage    = static_cast<float>(Mjolnir->attackDamage);
            mjolnir->MjolnirProjBaseT->model = Mjolnir->model;
            mjolnir->MjolnirProjBaseA->model = Mjolnir->model;
            if (WeaponIdentify::Mjolnir->HasWorldModel()) {
                spdlog::debug("Mjolnir is throwable");
                mjolnir->SetThrowState(tStateM::kThrowable);
                mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, true);
                kratos->SetIsCanCallMjolnir(a_actor, false);
            } else spdlog::warn("Mjolnir is not equipped for real");

            if (mjolnir->SpellCatchMjolnir && a_actor->AsMagicTarget()->HasMagicEffect(mjolnir->EffCatchMjolnir)) {a_actor->RemoveSpell(mjolnir->SpellCatchMjolnir);}

            if (mjolnir->MjolnirProjectileA 
            && (mjolnir->GetThrowState() == tStateM::kArriving
             || mjolnir->GetThrowState() == tStateM::kArrived)) 
                mjolnir->Catch(true);
        }
#ifdef TRIDENT
        else if (Config::TridentKWD && a_RHandWeapon->HasKeyword(Config::TridentKWD)) {
            isTrident = true;
            isRelic = true;
            isPoseidon = true;
            RelicName = "the Trident";
            Trident = a_RHandWeapon;
            Config::SpecialWeapon->value = 3U;//(uint8_t)Kratos::Relic::kTrident;
            Config::registeredTridentFileName = equippedWeaponFile;
            Config::registeredTridentID = equippedWeaponID;
            kratos->gTridentFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kTrident;

            auto trident = Trident::GetSingleton();
            trident->data.weap      = Trident;
            trident->data.ench      = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            trident->data.poison    = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            trident->data.damage    = static_cast<float>(Trident->attackDamage);
            trident->TridentProjBaseL->model = Trident->model;
            if (WeaponIdentify::Trident->HasWorldModel()) {
                trident->isTridentThrowable = true;
                spdlog::debug("Trident is throwable");
            } else spdlog::warn("Trident is not equipped for real");
        }
#endif
        spdlog::info("{} is {}", a_RHandWeapon->GetName(), RelicName);
    } if (a_shield) {
        const auto equippedShieldFile = a_shield->GetFile()->fileName;
        RelicName = "not a Relic";
        if (Config::GuardianShieldKWD && a_shield->HasKeyword(Config::GuardianShieldKWD)) {
            isGuardianShield = true;
            isKratos = true;
            Config::SpecialShield->value = (uint8_t)Kratos::Shield::kGuardianShield;
            Config::registeredGuardianShieldFileName = equippedShieldFile;
            RelicName = "the Guardian Shield";
            GuardianShield = a_shield;
            kratos->gGuardianShieldFormID->value = a_shield->formID;
        }
        spdlog::info("{} is {}", a_shield->GetName(), RelicName);
    }
    a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
}
void WeaponIdentify::SpecialityCheck(RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectWEAP* a_LHandWeapon, RE::TESObjectARMO* a_shield, const Kratos::Relic a_relic, const Kratos::Shield a_specialShield)
{
    if (Config::SpecialKWDs.size() < 6) {spdlog::error("check the special equipment keyword list!"); return;}
    if (a_RHandWeapon) {
        switch (a_relic)
        {
        case Kratos::Relic::kNone:
            if (a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
            }
            break;
        case Kratos::Relic::kLeviathanAxe:
            if (!isLeviathanAxe) {
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::LeviathanAxeKWD);
            } break;
        case Kratos::Relic::kBladeOfChaos:
            if (!isBladeOfChaos) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::BladeOfChaosKWD);
                if (a_LHandWeapon && !a_LHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                    a_LHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                    a_LHandWeapon->AddKeyword(Config::BladeOfChaosKWD);
                }
            } break;
        case Kratos::Relic::kDraupnirSpear:
            if (!isDraupnirSpear) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::DraupnirSpearKWD);
            } break;
        case Kratos::Relic::kBladeOfOlympus:
            if (!isBladeOfOlympus) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::BladeOfOlympusKWD);
            } break;
        case Kratos::Relic::kMjolnir:
            if (!isMjolnir) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::MjolnirKWD);
            } break;
#ifdef TRIDENT
        case Kratos::Relic::kTrident:
            if (!isTrident) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::TridentKWD);
            } break;
#endif
        default:
            break;
        }
    }
    if (a_shield) {
        switch (a_specialShield)
        {
        case Kratos::Shield::kNone:
            if (isGuardianShield) {
                if (a_RHandWeapon->HasKeyword(Config::GuardianShieldKWD)) {Config::registeredGuardianShieldID = 105; Config::registeredGuardianShieldFileName = "Not Registered";}
                a_shield->RemoveKeywords(Config::SpecialKWDs);
            } break;
        case Kratos::Shield::kGuardianShield:
            if (!isGuardianShield) {
                a_shield->RemoveKeywords(Config::SpecialKWDs);
                a_shield->AddKeyword(Config::GuardianShieldKWD);
            } break;
        case Kratos::Shield::kDauntlessShield:
            break;
        case Kratos::Shield::kStoneWallShield:
            break;
        case Kratos::Shield::kShatterStarShield:
            break;
        case Kratos::Shield::kOnslaughtShield:
            break;

        default:
            break;
        }
    }
}
void WeaponIdentify::WeaponCheck(const bool a_specialityCheck)
{
    isLeviathanAxe  = false;
    isBladeOfChaos  = false;
    isDraupnirSpear = false;
    isBladeOfOlympus = false;
    isMjolnir = false;
    isTrident = false;
    isRelic = false;
    isKratos = false;
    isThor = false;

    auto AnArchos = PlayerCharacter::GetSingleton();
    if      (!AnArchos || !AnArchos->AsActorValueOwner()) {spdlog::warn("can't found the player!"); return;}
    if      (AnArchos->GetNodeByName("NPC R Finger20 [RF20]"))  {RHandBone = AnArchos->GetNodeByName("NPC R Finger20 [RF20]");}
    else if (AnArchos->GetNodeByName("NPC R MagicNode [RMag]")) {RHandBone = AnArchos->GetNodeByName("NPC R MagicNode [RMag]");}
    else if (AnArchos->GetNodeByName("NPC R Hand [RHnd]"))      {RHandBone = AnArchos->GetNodeByName("NPC R Hand [RHnd]");}
    else    spdlog::warn("can't found right hand bone!");

    if      (AnArchos->GetNodeByName("NPC L Finger20 [LF20]"))  {LHandBone = AnArchos->GetNodeByName("NPC L Finger20 [LF20]");}
    else if (AnArchos->GetNodeByName("NPC L MagicNode [LMag]")) {LHandBone = AnArchos->GetNodeByName("NPC L MagicNode [LMag]");}
    else if (AnArchos->GetNodeByName("NPC L Hand [LHnd]"))      {LHandBone = AnArchos->GetNodeByName("NPC L Hand [LHnd]");}
    else    spdlog::warn("can't found left hand bone!");

    if  (AnArchos->GetNodeByName("Weapon"))                         {WeaponBone = AnArchos->GetNodeByName("Weapon");}
    if  (AnArchos->GetNodeByName("Shield"))                         {ShieldBone = AnArchos->GetNodeByName("Shield");}
    //  spdlog::debug("Right hand bone is {}", RHandBone->name);

    auto pcSkillArchery = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kArchery);
    auto pcSkill1Handed = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kOneHanded);
    auto pcDamageMult   = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAttackDamageMult);
    DamageMult  = 1.f + (pcSkill1Handed / 120) + (pcSkillArchery / 80);
    DamageMult  *= pcDamageMult;

    RE::TESObjectWEAP* RHandWeapon = nullptr;
    RE::TESObjectWEAP* LHandWeapon = nullptr;
    RE::TESObjectARMO* shield = nullptr;
    auto rObj = AnArchos->GetEquippedObject(false);
    auto lObj = AnArchos->GetEquippedObject(true);
    if (rObj) {
        EquippedObjR = rObj->As<TESBoundObject>();
        if (rObj->IsWeapon()) {
            RHandWeapon = rObj->As<TESObjectWEAP>();
        }
    } if (lObj) {
        EquippedObjL = lObj->As<TESBoundObject>();
        if (lObj->IsArmor()) {
            shield = lObj->As<TESObjectARMO>();
        } else if (lObj->IsWeapon()) {
            LHandWeapon = lObj->As<TESObjectWEAP>();
        }
    } if (!lObj && !rObj) {
        isBarehanded = true;
    } else isBarehanded = false;

    if (RHandWeapon || LHandWeapon || shield) {
        if (!RHandWeapon && !LHandWeapon) isBarehanded = true;
        else isBarehanded = false;
        if (a_specialityCheck) {
            SpecialityCheck(RHandWeapon, LHandWeapon, shield, static_cast<Kratos::Relic>((uint8_t)Config::SpecialWeapon->value), static_cast<Kratos::Shield>((uint8_t)Config::SpecialShield->value));
        }
        WeaponIdentifier(AnArchos, RHandWeapon, LHandWeapon, shield);
    } else {
        Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        Config::SpecialShield->value = (uint8_t)Kratos::Shield::kNone;
        AnArchos->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
    }
}
bool WeaponIdentify::IsRelic(RE::BGSProjectile *a_baseProj, Kratos::Relic a_relic)
{
    if (!a_baseProj) return false;
    auto Levi = LeviathanAxe::GetSingleton();
    auto leviA = Levi->LeviProjBaseA;
    auto leviL = Levi->LeviProjBaseL;
    auto leviH = Levi->LeviProjBaseH;
    auto draup = Draupnir::DraupnirSpearProjBaseL;
    auto dcall = Draupnir::DraupnirsCallProjBaseL;
    auto mjolnir = Mjolnir::GetSingleton();
    auto mjolnirT = mjolnir->MjolnirProjBaseT;
    auto mjolnirA = mjolnir->MjolnirProjBaseA;
#ifdef TRIDENT
    auto trident = Trident::GetSingleton();
    auto tridentL = trident->TridentProjBaseL;
    auto tridentC = trident->TridentCallProjBaseL;
#endif

    switch (a_relic)
    {
    case Kratos::Relic::kNone:
#ifdef TRIDENT
        return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH || a_baseProj == draup || a_baseProj == dcall || a_baseProj == mjolnirT || a_baseProj == mjolnirA || a_baseProj == tridentL || a_baseProj == tridentC);
#else
        return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH || a_baseProj == draup || a_baseProj == dcall || a_baseProj == mjolnirT || a_baseProj == mjolnirA);
#endif
    case Kratos::Relic::kLeviathanAxe:
        return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH);
    case Kratos::Relic::kBladeOfChaos:
        return false;
    case Kratos::Relic::kDraupnirSpear:
        return (a_baseProj == draup || a_baseProj == dcall);
    case Kratos::Relic::kBladeOfOlympus:
        return false;
    case Kratos::Relic::kMjolnir:
        return (a_baseProj == mjolnirT || a_baseProj == mjolnirA);
#ifdef TRIDENT
    case Kratos::Relic::kTrident:
        return (a_baseProj == tridentL || a_baseProj == tridentC);
#endif
    default:
        return false;
    }
}
#pragma endregion
#pragma region Kratos
void Kratos::Update(RE::Actor* a_actor, const float a_delta)
{
    if (IsInRage()) RestoreRage(a_actor, -(*values.rageDamageAmount * (*g_deltaTimeRealTime)));

    LeviathanAxe::GetSingleton()->Update(a_delta);
    Draupnir::Update(a_delta);
    Mjolnir::GetSingleton()->Update(a_delta);
#ifdef TRIDENT
    Trident::GetSingleton()->Update(a_delta);
#endif
}
bool Kratos::Initialize()
{
    if (auto AnArchos = RE::PlayerCharacter::GetSingleton(); AnArchos) {
        AnArchos->GetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);    //  Taking default values
        AnArchos->GetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);     //  Taking default values
        AnArchos->GetGraphVariableBool("Skip3DLoading", _skipLoad3D);            //  Taking default values
    } else spdlog::warn("WEIRD, the player character can't found");

//    SetIsCanCallAxe();
//    SetIsCanCallMjolnir();

    auto dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {spdlog::error("can't get data handler!!"); return false;}
    bool found = true;
    SpellAxeThrownState     = dataHandler->LookupForm<RE::SpellItem>(0x81B, Config::KratosCombatESP);
    SpellMjolnirThrownState = dataHandler->LookupForm<RE::SpellItem>(0x844, Config::KratosCombatESP);
    SpellSpartanRage        = dataHandler->LookupForm<RE::SpellItem>(0x80F, Config::KratosCombatESP);
    SpellStrenghtBuff       = dataHandler->LookupForm<RE::SpellItem>(0x829, Config::KratosCombatESP);
    SpellLeviChargeCD       = dataHandler->LookupForm<RE::SpellItem>(0x827, Config::KratosCombatESP);
    SpellMjolnirChargeCD    = dataHandler->LookupForm<RE::SpellItem>(0x848, Config::KratosCombatESP);
    SpellAxeCallButton      = dataHandler->LookupForm<RE::SpellItem>(0x803, Config::KratosCombatESP);
    SpellAimButton          = dataHandler->LookupForm<RE::SpellItem>(0x805, Config::KratosCombatESP);
    SpellRunicButton        = dataHandler->LookupForm<RE::SpellItem>(0x809, Config::KratosCombatESP);
    SpellFinisherButton     = dataHandler->LookupForm<RE::SpellItem>(0x807, Config::KratosCombatESP);
    SpellMidDistButton      = dataHandler->LookupForm<RE::SpellItem>(0x80B, Config::KratosCombatESP);
    SpellLongDistButton     = dataHandler->LookupForm<RE::SpellItem>(0x80D, Config::KratosCombatESP);
    soundEffect.catchLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x2398A, "Skyrim.esm");
    soundEffect.callLevi    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x7D013, "Skyrim.esm");
    soundEffect.chargeLevi  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EDD5, "Skyrim.esm");
    soundEffect.chargeLeviLoop  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3E5CB, "Skyrim.esm");
    soundEffect.chargeLeviEndT  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EDD5, "Skyrim.esm");
    soundEffect.chargeLeviEnd   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EAC6, "Skyrim.esm");
    soundEffect.catchMjolnir    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x2398A, "Skyrim.esm");
    soundEffect.callMjolnir     = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x7D013, "Skyrim.esm");
    soundEffect.fingerSnap      = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x84A, Config::KratosCombatESP);
    soundEffect.chargeMjolnir   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3F205, "Skyrim.esm");
    soundEffect.chargeMjolnirEnd= dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EAC6, "Skyrim.esm");
    vanillaSpell.frostCloak     = dataHandler->LookupForm<RE::SpellItem>(0x3AEA2, "Skyrim.esm");
    vanillaSpell.fireCloak      = dataHandler->LookupForm<RE::SpellItem>(0x3AE9F, "Skyrim.esm");
    vanillaSpell.lightningBolt  = dataHandler->LookupForm<RE::SpellItem>(0x2DD29, "Skyrim.esm");
    vanillaSpell.lightningCloak = dataHandler->LookupForm<RE::SpellItem>(0x3AEA3, "Skyrim.esm");
    vanillaSpell.thunderbolt    = dataHandler->LookupForm<RE::SpellItem>(0x10F7EE, "Skyrim.esm");
    vanillaSpell.sparks         = dataHandler->LookupForm<RE::SpellItem>(0x2DD2A, "Skyrim.esm");
    kittyTailSpell.magesFury    = dataHandler->LookupForm<RE::SpellItem>(0x809, "StormCalling.esl");
    kittyTailSpell.streak       = dataHandler->LookupForm<RE::SpellItem>(0x80A, "StormCalling.esl");
    kittyTailSpell.teslaSphere  = dataHandler->LookupForm<RE::SpellItem>(0x833, "StormCalling.esl");
    kittyTailSpell.tonitrus     = dataHandler->LookupForm<RE::SpellItem>(0x955, "StormCalling.esl");
    kittyTailSpell.maelstrom    = dataHandler->LookupForm<RE::SpellItem>(0xA04, "StormCalling.esl");
    kittyTailSpell.lightningFlood   = dataHandler->LookupForm<RE::SpellItem>(0x80E, "StormCalling.esl");
    VFXeffect.handFrost         = dataHandler->LookupForm<RE::BGSArtObject>(0x42854, "Skyrim.esm");
    VFXeffect.handFrostBright   = dataHandler->LookupForm<RE::BGSArtObject>(0x334B9, "Skyrim.esm");
    VFXeffect.handShock         = dataHandler->LookupForm<RE::BGSArtObject>(0x58E9B, "Skyrim.esm");
    VFXeffect.handShockCloak    = dataHandler->LookupForm<RE::BGSArtObject>(0x5B1BD, "Skyrim.esm");
    VFXeffect.handFlame     = dataHandler->LookupForm<RE::BGSArtObject>(0x1B211, "Skyrim.esm");
    VFXeffect.frostCloak    = dataHandler->LookupForm<RE::BGSArtObject>(0x4253F, "Skyrim.esm");
    VFXeffect.fireCloak     = dataHandler->LookupForm<RE::BGSArtObject>(0x2ACD7, "Skyrim.esm");
    action.normalAttack     = dataHandler->LookupForm<RE::BGSAction>(0x13005, "Skyrim.esm");
    action.powerAttack      = dataHandler->LookupForm<RE::BGSAction>(0x13383, "Skyrim.esm");
    action.dualPowerAttack  = dataHandler->LookupForm<RE::BGSAction>(0x2E2F7, "Skyrim.esm");

    gLeviathanAxeFormID     = dataHandler->LookupForm<RE::TESGlobal>(0x839, Config::KratosCombatESP);
    gBladeOfChaosFormID     = dataHandler->LookupForm<RE::TESGlobal>(0x83A, Config::KratosCombatESP);
    gDraupnirSpearFormID    = dataHandler->LookupForm<RE::TESGlobal>(0x83B, Config::KratosCombatESP);
    gBladeOfOlympusFormID   = dataHandler->LookupForm<RE::TESGlobal>(0x83C, Config::KratosCombatESP);
    gMjolnirFormID          = dataHandler->LookupForm<RE::TESGlobal>(0x83E, Config::KratosCombatESP);
#ifdef TRIDENT
    gTridentFormID          = dataHandler->LookupForm<RE::TESGlobal>(0x800, Config::TridentModESP);
#endif
    gGuardianShieldFormID   = dataHandler->LookupForm<RE::TESGlobal>(0x83D, Config::KratosCombatESP);

    if (!SpellAxeThrownState || !SpellSpartanRage || !SpellStrenghtBuff || !SpellMjolnirChargeCD || !VFXeffect.handFrost)
        {spdlog::error("Check Kratos's spell addresses"); found = false;}

    values.rage = &Config::RageAmount->value;
    values.rageLimit = &Config::RageLimit->value;
    values.rageBuffAmount = &Config::RageBuffRate->value;
    values.rageDamageAmount = &Config::RageDamageRate->value;

    EffectAxeThrownState= SpellAxeThrownState ? SpellAxeThrownState->effects[0]->baseEffect : nullptr;
    EffectSpartanRage   = SpellSpartanRage ? SpellSpartanRage->effects[0]->baseEffect : nullptr;
    EffectStrenghtBuff  = SpellStrenghtBuff ? SpellStrenghtBuff->effects[0]->baseEffect : nullptr;
    EffectLeviChargeCD  = SpellLeviChargeCD ? SpellLeviChargeCD->effects[0]->baseEffect : nullptr;
    EffectMjolnirChargeCD   = SpellMjolnirChargeCD ? SpellMjolnirChargeCD->effects[0]->baseEffect : nullptr;
    EffectAimButton     = SpellAimButton ? SpellAimButton->effects[0]->baseEffect : nullptr;
    EffectAxeCallButton = SpellAxeCallButton ? SpellAxeCallButton->effects[0]->baseEffect : nullptr;
    EffRunicButton      = SpellRunicButton  ? SpellRunicButton ->effects[0]->baseEffect : nullptr;
    EffFinisherButton   = SpellFinisherButton ? SpellFinisherButton->effects[0]->baseEffect : nullptr;
    EffMidDistButton    = SpellMidDistButton ? SpellMidDistButton->effects[0]->baseEffect : nullptr;
    EffLongDistButton   = SpellLongDistButton ? SpellLongDistButton->effects[0]->baseEffect : nullptr;
    found = EffectAxeThrownState && EffectSpartanRage && EffectStrenghtBuff && EffectLeviChargeCD && EffectAimButton && EffectAxeCallButton
    && EffRunicButton && EffFinisherButton;
    if (found) {
        spellID.aim = EffectAimButton->GetFormID();
        spellID.call = EffectAxeCallButton->GetFormID();
        spellID.runic = EffRunicButton->GetFormID();
        spellID.finisher = EffFinisherButton->GetFormID();
        spellID.midDist = EffMidDistButton->GetFormID();
        spellID.longDist = EffLongDistButton->GetFormID();
        spellID.leviChargeCoolDown = EffectLeviChargeCD->GetFormID();
        spellID.AxeThrownState = EffectAxeThrownState->GetFormID();
        spellID.strenghtBuff = EffectStrenghtBuff->GetFormID();
        spellID.spartanRage = EffectSpartanRage->GetFormID();
    } return found;
}
float Kratos::CalcRageDamageOrBuffAmount(const float a_amount, const float a_mult)
{
    if (a_amount > 0.f) {
        if (IsInRage()) return -(*values.rageDamageAmount * a_mult * (5.f + a_amount / 300.f));
        else return (*values.rageBuffAmount * a_mult * (1.f + sqrtf(a_amount) / 100.f));
    } return (*values.rageDamageAmount * a_mult * (a_amount - 1.f) / 5.f);
}
bool Kratos::IsEquipped(const Kratos::Relic a_relic) const
{
    switch (a_relic) {
    case Relic::kNone:
        return WeaponIdentify::isRelic;
    case Relic::kLeviathanAxe:
        return WeaponIdentify::isLeviathanAxe;
    case Relic::kBladeOfChaos:
        return WeaponIdentify::isBladeOfChaos;
    case Relic::kDraupnirSpear:
        return WeaponIdentify::isDraupnirSpear;
    case Relic::kBladeOfOlympus:
        return WeaponIdentify::isBladeOfOlympus;
    case Relic::kMjolnir:
        return WeaponIdentify::isMjolnir;
    default:
        break;
    } return false;
}
bool Kratos::IsCanCallAxe(RE::Actor* a_actor) const {if (a_actor && SpellAxeThrownState) return a_actor->HasSpell(SpellAxeThrownState); return false;}
bool Kratos::IsCanCallMjolnir(RE::Actor* a_actor) const {if (a_actor && SpellMjolnirThrownState) return a_actor->HasSpell(SpellMjolnirThrownState); return false;}
bool Kratos::IsCanRage(RE::Actor* a_actor) const
{
    return (*values.rage > *values.rageDamageAmount * 6.f) && !_isInRage;
}
bool Kratos::IsInRage(RE::Actor* a_actor)
{
    if (a_actor) {
        a_actor->GetGraphVariableBool("IsInRage", _isInRage);
    } return _isInRage;
}
bool Kratos::IsCanCharge(RE::Actor* a_actor, Kratos::Relic a_relic) const
{
    bool isCan = false;
    auto magicTarget = a_actor ? a_actor->GetMagicTarget() : nullptr;
    auto cdLeviEffect = SpellLeviChargeCD ? SpellLeviChargeCD->effects[0] : nullptr;
    auto cdLeviBaseEffect = cdLeviEffect ? cdLeviEffect->baseEffect : nullptr;
    auto cdMjolnirEffect = SpellMjolnirChargeCD ? SpellMjolnirChargeCD->effects[0] : nullptr;
    auto cdMjolnirBaseEffect = cdMjolnirEffect ? cdMjolnirEffect->baseEffect : nullptr;
    if (magicTarget) {
        switch  (a_relic) {
        case Relic::kLeviathanAxe:
            if (cdLeviBaseEffect) isCan = !magicTarget->HasMagicEffect(cdLeviBaseEffect);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
            break;
        case Relic::kMjolnir:
            if (cdMjolnirBaseEffect) isCan = !magicTarget->HasMagicEffect(cdMjolnirBaseEffect);
            break;
        default:
            break;
        }
    } return isCan;
}
bool Kratos::IsShieldOpened() const {return _isShieldOpened;}
Kratos::Relic Kratos::GetEquippedRelic() const
{
    if (!WeaponIdentify::isRelic) return Relic::kNone;
    if (WeaponIdentify::isLeviathanAxe) return Relic::kLeviathanAxe;
    else if (WeaponIdentify::isBladeOfChaos) return Relic::kBladeOfChaos;
    else if (WeaponIdentify::isDraupnirSpear) return Relic::kDraupnirSpear;
    else if (WeaponIdentify::isBladeOfOlympus) return Relic::kBladeOfOlympus;
    else if (WeaponIdentify::isMjolnir) return Relic::kMjolnir;
    return Relic::kNone;
}
Kratos::Relic Kratos::GetLastEquippedRelic() const
{
    if (!WeaponIdentify::isRelic) return Relic::kNone;
    if (WeaponIdentify::isLeviathanAxe) return Relic::kLeviathanAxe;
    else if (WeaponIdentify::isBladeOfChaos) return Relic::kBladeOfChaos;
    else if (WeaponIdentify::isDraupnirSpear) return Relic::kDraupnirSpear;
    else if (WeaponIdentify::isBladeOfOlympus) return Relic::kBladeOfOlympus;
    return Relic::kNone;
}
Kratos::Relic Kratos::GetNextWeaponToCall() const
{
    Kratos::Relic ret = Kratos::Relic::kNone;
    if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kLeviathanAxe) {
        if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) ret = Kratos::Relic::kMjolnir;
#ifdef TRIDENT
        else if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) ret = Kratos::Relic::kTrident;
#endif
        else ret = Kratos::Relic::kLeviathanAxe;
    }
    else if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kMjolnir) {
        if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) ret = Kratos::Relic::kLeviathanAxe;
#ifdef TRIDENT
        else if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) ret = Kratos::Relic::kTrident;
#endif
        else ret = Kratos::Relic::kMjolnir;
    }
#ifdef TRIDENT
    else if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kTrident) {
        if (Trident::GetSingleton()->isTridentThrowable && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) ret = Kratos::Relic::kLeviathanAxe;
        else if (Trident::GetSingleton()->isTridentThrowable && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) ret = Kratos::Relic::kMjolnir;
        else ret = Kratos::Relic::kTrident;
    }
#endif
    else {spdlog::warn("Can't found any weapon for ready to calling! Trying to call Levi"); ret = Kratos::Relic::kLeviathanAxe;}
    return ret;
}
Kratos::Rage Kratos::GetRageType() const
{
    switch ((Kratos::Rage)Config::RageType)
    {
    case Kratos::Rage::kFury:   return Kratos::Rage::kFury;
    case Kratos::Rage::kValor:  return Kratos::Rage::kValor;
    case Kratos::Rage::kWrath:  return Kratos::Rage::kWrath;
    case Kratos::Rage::kLegacy: return Kratos::Rage::kLegacy;
    } return Kratos::Rage::kFury;
}
Kratos::Rage Kratos::GetLastTriggeredRageType() const
{
    return _lastTriggeredRage;
}
void Kratos::DoKratosAction(const Kratos::Action a_action, RE::Actor* a_actor)
{
    if (a_actor)
        switch (a_action)
        {
        case Kratos::Action::kRage:
            if (!IsInRage(a_actor)) StartRage((Rage)Config::RageType, true, a_actor);
            else EndRage(GetLastTriggeredRageType());
            break;
        case Kratos::Action::kWeaponCharge:
            if (!IsInRage(a_actor)) {
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kWeaponCharge);
                a_actor->NotifyAnimationGraph("DoKratosAction");
            }
            break;
        case Kratos::Action::kSpecialIdle:
            if (!IsInRage(a_actor)) {
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kSpecialIdle);
                a_actor->NotifyAnimationGraph("DoKratosAction");
            }
            break;

        default:
            break;
        }
}
void Kratos::SetIsCanCallAxe(RE::Actor* a_actor, const bool a_isCan)
{
    if (a_isCan && !IsCanCallAxe(a_actor)) a_actor->AddSpell(SpellAxeThrownState);
    else if (!a_isCan && IsCanCallAxe(a_actor)) a_actor->RemoveSpell(SpellAxeThrownState);
}
void Kratos::SetIsCanCallMjolnir(RE::Actor* a_actor, const bool a_isCan)
{
    if (a_isCan && !IsCanCallMjolnir(a_actor)) a_actor->AddSpell(SpellMjolnirThrownState);
    else if (!a_isCan && IsCanCallMjolnir(a_actor)) a_actor->RemoveSpell(SpellMjolnirThrownState);
}
void Kratos::SetIsCanRage(const bool a_isCan, RE::Actor* a_actor)
{
    if (a_actor) {
        if (a_isCan) _isInRage = false;
        else _isInRage = true;
        a_actor->SetGraphVariableBool("IsInRage", _isInRage);
    }
}
void Kratos::SetIsCanCharge(RE::Actor* a_actor, const bool a_isCan, Kratos::Relic a_relic)
{
    auto magicTarget = a_actor->GetMagicTarget();
    auto magicCaster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
    if (a_isCan && !IsCanCharge(a_actor, a_relic) && magicTarget) {
        auto casterHandle = a_actor->GetHandle();

        switch (a_relic) {
        case Relic::kLeviathanAxe:
            magicTarget->DispelEffect(SpellLeviChargeCD, casterHandle);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
            break;
        case Relic::kMjolnir:
            magicTarget->DispelEffect(SpellMjolnirChargeCD, casterHandle);
            break;
        default:
            break;
        }
    }
    else if (!a_isCan && IsCanCharge(a_actor, a_relic) && magicCaster) {
        switch (a_relic) {
        case Relic::kLeviathanAxe:
            magicCaster->CastSpellImmediate(SpellLeviChargeCD, false, nullptr, 1.f, false, 1.f, a_actor);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
            break;
        case Relic::kMjolnir:
            magicCaster->CastSpellImmediate(SpellMjolnirChargeCD, false, nullptr, 1.f, false, 1.f, a_actor);
            break;
        default:
            break;
        }
    }
}
void Kratos::RestoreRage(RE::Actor* a_actor, const float a_value, const bool a_justRestore)
{
    if (a_actor) {
        *values.rage += a_value;
        if (*values.rage < 0.f) {*values.rage = 0.f; if (!a_justRestore) EndRage(GetLastTriggeredRageType(), false, true, true, a_actor);}
        else if (*values.rage > *values.rageLimit) *values.rage = *values.rageLimit;
    }
}
void Kratos::OpenShield(RE::Actor* a_actor)
{
    if (const auto shieldModel = WeaponIdentify::ShieldBone; shieldModel) {
        if (auto controllers = shieldModel->GetControllers(); controllers) {
            if (auto manager = controllers->AsNiControllerManager(); manager) {
                auto startSequence = manager->GetSequenceByName("Begin");
                auto endSequence = manager->GetSequenceByName("Loop");
                if (startSequence && endSequence) {
                    a_actor->PlayAnimation(manager, endSequence, startSequence);
                    _isShieldOpened = true;
                    spdlog::debug("{} is opened", shieldModel->name);
                } else spdlog::debug("can't find the sequences for {}", shieldModel->name);
            } else spdlog::debug("can't find the manager for {}", shieldModel->name);
        } else spdlog::debug("can't find the controllers for {}", shieldModel->name);
    } else spdlog::debug("can't find the shield model");
}
void Kratos::CloseShield(RE::Actor* a_actor)
{
    if (a_actor && a_actor->GetEquippedObject(true) && a_actor->GetEquippedObject(true)->IsArmor() && a_actor->GetEquippedObject(true)->As<RE::TESObjectREFR>()) {
        if (auto shieldModel = a_actor->GetEquippedObject(true)->As<RE::TESObjectREFR>()->Get3D(); shieldModel) {
            if (auto controllers = shieldModel->GetControllers(); controllers) {
                if (auto manager = controllers->AsNiControllerManager(); manager) {
                    auto currentSequence = manager->activeSequences;
                    auto startSequence = manager->GetSequenceByName("End");
                    auto endSequence = manager->GetSequenceByName("Idle");
                    if (startSequence && endSequence) {
                        a_actor->PlayAnimation(manager, endSequence, startSequence);
                        _isShieldOpened = false;
                        spdlog::debug("{} is closed", shieldModel->name);
                    } else spdlog::debug("can't find the sequences for {}", shieldModel->name);
                } else spdlog::debug("can't find the manager for {}", shieldModel->name);
            } else spdlog::debug("can't find the controllers for {}", shieldModel->name);
        } else spdlog::debug("can't find the shield model");
    }
}
void Kratos::StartRage(const Kratos::Rage a_rage, const bool a_justAnim, RE::Actor* a_actor)
{
    if (a_actor) {
        if ((*values.rage - *values.rageDamageAmount * 6.f) < 0.f) return;
        a_actor->SetGraphVariableInt("iRageType", Config::RageType);
        if (WeaponIdentify::EquippedObjR) _LastEquippedObjectR = WeaponIdentify::EquippedObjR;
        if (WeaponIdentify::EquippedObjL) _LastEquippedObjectL = WeaponIdentify::EquippedObjL;
        _lastTriggeredRage = a_rage;
        switch (a_rage)
        {
        case Kratos::Rage::kFury:
            if (a_justAnim) {
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kRage);
                a_actor->NotifyAnimationGraph("DoKratosAction");
                return;
            }
            ObjectUtil::Actor::CastSpell(SpellSpartanRage, a_actor, a_actor, a_actor, 3.f);
            if (VFXeffect.fury) a_actor->ApplyArtObject(VFXeffect.fury, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
            if (WeaponIdentify::EquippedObjR)
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
            if (WeaponIdentify::EquippedObjL && WeaponIdentify::EquippedObjL != WeaponIdentify::GuardianShield)
                ObjectUtil::Actor::UnEquipItem(a_actor, true, false, false, false, true, true);
            if (WeaponIdentify::GuardianShield && !WeaponIdentify::isGuardianShield)
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::GuardianShield, true);

            ResetEquipAnimationAfter(100, a_actor);
            break;
        case Kratos::Rage::kValor:
            if (a_justAnim) {
                _gettingHittedInValor = false;
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kRage);
                a_actor->NotifyAnimationGraph("DoKratosAction");
                RestoreRage(a_actor, -(*values.rageDamageAmount * 10.f), true);
            }
            if (VFXeffect.valor) a_actor->ApplyArtObject(VFXeffect.valor, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
            break;
        case Kratos::Rage::kWrath:
            if (VFXeffect.wrath) a_actor->ApplyArtObject(VFXeffect.wrath, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
            return;
        case Kratos::Rage::kLegacy:
            if (WeaponIdentify::BladeOfOlympus) {
            //    if (auto mCaster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); mCaster && SpellSpartanRage) {
            //        mCaster->CastSpellImmediate(SpellSpartanRage, false, a_actor, 1.f, false, 2.f, a_actor);
            //    }
                if (VFXeffect.legacy) a_actor->ApplyArtObject(VFXeffect.legacy, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::BladeOfOlympus);
                ResetEquipAnimationAfter(100, a_actor);
            }
            break;
        default:
            break;
        }
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "weaponSwing");
        a_actor->SetGraphVariableBool("IsInRage", true);
    }
}
void Kratos::EndRage(const Kratos::Rage a_rage, const bool a_fromAnnotation, const bool a_playAnim, const bool a_justAnim, RE::Actor* a_actor)
{
    if (a_actor) {
        _isWantFinishRage = true;
        switch (a_rage)
        {
        case Kratos::Rage::kFury:
            if (a_playAnim) {
                a_actor->NotifyAnimationGraph("DoKratosAction");
                _isWantFinishRage = false;
                if (a_justAnim) return;
            }
            if (auto mTarget = a_actor->GetMagicTarget(); mTarget) {
                auto aHandle = a_actor->GetHandle();
                mTarget->DispelEffect(SpellSpartanRage, aHandle);
                ResetEquipAnimationAfter(0, a_actor);
                RestoreRage(a_actor, -(*values.rageDamageAmount * 6.f), true);
                if (_LastEquippedObjectR)
                    ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectR, true);
                if (_LastEquippedObjectL)
                    ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectL, true);
                ResetEquipAnimationAfter(100, a_actor);
            }
            break;
        case Kratos::Rage::kValor:
            if (a_fromAnnotation) {
                ObjectUtil::Actor::SendAnimationEvent(a_actor, "weaponSwing");
                a_actor->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, (*values.rageBuffAmount * 10.f));
                if (_gettingHittedInValor) ObjectUtil::Actor::CastSpell(SpellStrenghtBuff, a_actor, a_actor, a_actor);
                _gettingHittedInValor = false;
            }
            break;
        case Kratos::Rage::kWrath:
            break;
        case Kratos::Rage::kLegacy:
            if (a_playAnim) {
                a_actor->NotifyAnimationGraph("DoKratosAction");
                _isWantFinishRage = false;
                if (a_justAnim) return;
            }
            ResetEquipAnimationAfter(0, a_actor);
            if (_LastEquippedObjectR)
                ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectR, true);
            else if (WeaponIdentify::EquippedObjR) 
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
            if (_LastEquippedObjectL)
                ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectL, true);
            ResetEquipAnimationAfter(100, a_actor);
            break;

        default:
            break;
        }
        a_actor->SetGraphVariableBool("IsInRage", false);
        _isWantFinishRage = false;
        _LastEquippedObjectR = nullptr;
        _LastEquippedObjectL = nullptr;
    }
}
#pragma endregion
#pragma region Leviathan_Axe
LeviathanAxe* LeviathanAxe::GetSingleton()              {static LeviathanAxe singleton; return &singleton;}
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
void LeviathanAxe::Update(const float a_delta) {
    if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsEquipped(Kratos::Relic::kLeviathanAxe)) {
        if (kratos->IsChargingThrow()) {
            data.throwingChargeDuration += a_delta;
        //    if (data.throwingChargeDuration >= 2.f) ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviEndT, WeaponIdentify::RHandBone, 5.f);
        }
    }
    if (projectileUpdate.IsTimeToUpdate()) {
        if (data.model && data.proj &&/* data.proj->Is3DLoaded() &&*/ data.proj->Get3D2() && data.model == data.proj->Get3D2()) {
            auto& rtData = data.proj->GetProjectileRuntimeData();
            const RE::BSFixedString rotatingBoneName = "Cylinder02";
            auto animatedBone = data.model->GetObjectByName(rotatingBoneName);
            auto leviModel = data.weaponModel;
            if (animatedBone && leviModel) {
                auto animatedNode = animatedBone->AsNode();
                auto leviNode = leviModel->AsNode();
                if (animatedNode && leviNode && rtData.livingTime > 0.f) {
                //    animatedNode->AttachChild(leviNode, false);
                    projectileUpdate.Done();
                    spdlog::debug("levi model changed! living time: {}", rtData.livingTime);
                } else spdlog::warn("animated node or levinode null");
            } else spdlog::warn("animated bone or leviModel null");
        } else spdlog::warn("proj or proj->Get3D2() null");
    }
    if (Config::DrawTrails) {
        static RE::NiAVObject* cachedRoot = nullptr;
        static RE::NiAVObject* rotatingBone = nullptr;
        auto rootBone = data.model;

        if (cachedRoot != rootBone) {
            cachedRoot = rootBone;
            rotatingBone = nullptr;
        }

        if (!rotatingBone && rootBone) {
            rotatingBone = rootBone->GetObjectByName("Cylinder02");
        }
        auto bone = rotatingBone ? rotatingBone : rootBone;
        if (bone) {
            if (trailUpdate.IsTimeToUpdate()) {
#ifdef PRECISION
                if (bone && (APIs::precision || APIs::Request())) {
                    trailUpdate.Done();
                    auto trailNode = RE::NiNode::Create(0);
                    float length = bone->worldBound.radius;
                    float tipOffset = length;
		            trailNode->local.translate += RE::NiPoint3(0.f, tipOffset, 0.f);
                    trailNode->local.rotate = bone->local.rotate;
                    if (auto node = bone->AsNode(); node) node->AttachChild(trailNode, true);
                    PRECISION_API::TrailOverride trailOverride(2.f, RE::NiColorA(), 2.f, Config::TrailModelPathLevi.data());
                    APIs::precision->AddAttackTrail(trailNode, RE::PlayerCharacter::GetSingleton()->GetHandle(), RE::PlayerCharacter::GetSingleton()->GetParentCell(), LastLeviProjectile, std::nullopt);
                //    APIs::precision->AddAttackCollision(RE::PlayerCharacter::GetSingleton()->GetHandle(), collisionDefinition, LastMjolnirProjectile);
                }
#else
                float particleMult = _isCharged ? 2.f : 1.f;
                float length = bone->worldBound.radius;//60.f;
            //    spdlog::debug("Leviathan Axe projectile length is {}", length);
                float tipOffset = length;// / 3.f;
                data.trailTransform = bone->world * bone->local;
                data.trailTransform.translate = data.trailTransform * RE::NiPoint3(0.f, 0.f, tipOffset);

                auto projTrail = RE::NiPointer<RE::BSTempEffectParticle>(RE::BSTempEffectParticle::Spawn(RE::PlayerCharacter::GetSingleton()->GetParentCell(), 10.f, Config::TrailModelPathLevi.data(), data.trailTransform.rotate, data.trailTransform.translate, 0.01f * length, 7, nullptr));
                if (data.projTrails.size() < particleMult * Config::TrailParticleCount) {
                    data.projTrails.insert(data.projTrails.begin(), projTrail);
                }
            //    if (data.projTrails.size() > particleMult * Config::TrailParticleCount) {
            //    //    data.projTrails.pop_back();
            //        trailUpdate.Done();
            //    }
            }
        }
        if (!data.projTrails.empty()) {
            uint8_t trailIndex(1);
            data.projTrails.erase(
                std::remove_if(data.projTrails.begin(), data.projTrails.end(),
                    [a_delta, &trailIndex, this](RE::NiPointer<RE::BSTempEffectParticle> projTrail) {
                        if (projTrail) {
                            if (projTrail->age >= projTrail->lifetime) {
                                projTrail->~BSTempEffectParticle();
                                projTrail.~NiPointer();
                            //    spdlog::debug("{}. projectile trail removed! age: {} lifetime: {}", trailIndex, projTrail->age, projTrail->lifetime);
                                return true; // Mark for removal
                            } else {
                                if (projTrail->age > 1.f) projTrail->age += a_delta;    //  projTrail->lifetime can't be changed, it is always 1.f;
                            //    spdlog::debug("{}. projectile trail age is {}", trailIndex, projTrail->age);
                            }
                        //    trailIndex++;
                        }
                        return false; // Keep in the vector
                    }),
                data.projTrails.end());
            for (auto& projTrail : data.projTrails) {
                auto particleObject = projTrail ? projTrail->particleObject : nullptr;
                auto fadeNode = particleObject ? particleObject->AsFadeNode() : nullptr;
                auto trailRoot = fadeNode ? fadeNode->GetObjectByName("TrailRoot"sv) : nullptr;
                auto trailRootNode = trailRoot ? trailRoot->AsNode() : nullptr;
                if (trailRootNode && !trailRootNode->GetChildren().empty()) {
                    trailRootNode->world = data.trailTransform;
                //    trailRootNode->local = ObjectUtil::Node::GetLocalTransform(trailRootNode, data.trailTransform);
                //    spdlog::debug("{}. projectile trail transform updated! age: {} lifetime: {}", trailIndex, projTrail->age, projTrail->lifetime);
                } trailIndex++;
#endif
            }
        }
    }
}
tState LeviathanAxe::GetThrowState()     const          {return throwState;}
void LeviathanAxe::SetThrowState(ThrowState a_state)    {throwState = a_state;}
void LeviathanAxe::GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor)
{
    if (!a_actor) return;
    auto stuckedLevi =  LastLeviProjectile;
    if (!stuckedLevi) stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : LeviathanAxeProjectileH);
    if (stuckedLevi) {// && !(stuckedLevi->GetProjectileRuntimeData().flags & (1 << 25))) {
        a_point = stuckedLevi->data.location;
    } else spdlog::debug("we can't get leviathan's proj!");

    if (data.stuckedBone) {
        a_point = data.stuckedBone->world.translate;
        data.stuckedBone = nullptr;

        if (data.stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(data.stuckedActor, data.model);
#else
            data.stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("levi stucked actor's extra arrows removed");
            data.stuckedActor = nullptr;
        } else spdlog::debug("levi not stucked anybody");
    } else spdlog::debug("levi not stucked any bone");

    if (GetThrowState() == tState::kThrowable) {
        if (auto backWeaponSheathe = a_actor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            a_point = backWeaponSheathe->world.translate;
        }
    } else {
        auto pcCell = a_actor->parentCell;
        auto bound = pcCell->GetRuntimeData().cellData.exterior->worldX;
        auto pcPos = a_actor->GetPosition();
        float dist = pcPos.GetDistance(a_point);
        if (dist > 36000.f) {   // ~42000 is limit
            spdlog::info("levi too far from you! ({} m)", (int)dist / 100);
            auto dir = a_point - pcPos;
            dir.Unitize();
            a_point = pcPos + dir * 36000.f;
        }
    }
    data.projState = ProjectileState::kNone;
}
void LeviathanAxe::Throw(const bool a_isVertical, const bool justContinue, const bool isHoming, RE::Actor* a_actor)
{
    if (!a_actor) {spdlog::warn("LeviathanAxe::Throw - a_actor is null"); return;}

    bool isVertical = a_isVertical;
    bool isThrowAttack = false;
    bool isPowerThrowAttack = false;
    if (Config::IsAdvancedThrowingInstalled) {
        a_actor->GetGraphVariableBool("bIsThrowing", isThrowAttack);
        a_actor->GetGraphVariableBool("bIsPowerThrowing", isPowerThrowAttack);
        isVertical = isPowerThrowAttack;
    }
    const auto leviThrowSpell = (a_isVertical || isVertical ? SpellLeviProjH : SpellLeviProjL);
//  auto leviBaseProj = (isVertical ? LeviProjBaseH : LeviProjBaseL);
    if (leviThrowSpell && (WeaponIdentify::isLeviathanAxe || justContinue)) 
    {   //  calculate damage
        const auto leviProjEff = leviThrowSpell->effects[0];
        auto& leviProjEffSetting = leviProjEff->effectItem;
        auto& mag = leviProjEffSetting.magnitude;
        const auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
        mag = leviDamage * WeaponIdentify::DamageMult;
        bool isPowerThrow; a_actor->GetGraphVariableBool("IsPowerThrowing", isPowerThrow);
        if (isVertical || isPowerThrow) {mag *= 1.5f; data.yAngle = 1.57f;}
        else data.yAngle = 0.35f;
        float throwChargeDamageMult = std::sqrtf(data.throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        mag *= throwChargeDamageMult;

    if (const auto leviProjBaseEff = leviProjEff->baseEffect; leviProjBaseEff && leviProjBaseEff->data.projectileBase) {
    //      leviProjBaseEff->data.projectileBase->SetModel(WeaponIdentify::LeviathanAxe->GetModel());
    //  //  leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
    //  //  leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
        leviProjBaseEff->data.projectileBase->data.force = mag;
    } else spdlog::warn("not found Levi throwing effect!");

        if (!justContinue) {
            data.gravity = 2.f / (std::powf(data.throwingChargeDuration + 1.f, 3.f));
        }

        auto kratos = Kratos::GetSingleton();
        //  set the launch data
        auto origin = justContinue ? data.position : WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {a_actor->GetAimAngle(), a_actor->GetAimHeading()};
        if (data.model && (justContinue/* || isHoming*/)) a_actor->Unk_A0(data.model, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, leviThrowSpell);

        lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(a_actor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor);
        //  throw the projectile
        RE::Projectile::Launch(&pHandle, lData);
        data.proj = pHandle.get().get();

    //    projectileUpdate.RegisterForUpdate(1.01, false);
        trailUpdate.RegisterForUpdate(0.01, false);

        if (isHoming) {
            if (justContinue) {
                homingLevi.proj = homingLevi.isBoomerang || !homingLevi.targets.empty() ? data.proj : nullptr;
            } else {
                std::vector<RE::Actor *> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(a_actor, Config::HProjectileTargetRange, true);
                homingLevi = HomingLeviathan(data.proj, nearCombatTargets, a_actor, Config::HProjectileLoopCount, Config::ReturnHProjectileAfterLoops);
            }
        }
        if (justContinue) return;

        if (Config::IsAdvancedThrowingInstalled && (isThrowAttack || isPowerThrowAttack)) {
            ResetCharge(data.enchMag, data.defaultEnchMag, true);
            WeaponIdentify::skipEquipAnim = true;
            ObjectUtil::Actor::UnEquipItem(a_actor, false, false, true, true, WeaponIdentify::skipEquipAnim, false);
            ResetEquipAnimationAfter(100, a_actor);
            spdlog::debug("Leviathan unequipped after throwing");
        } else {
            WeaponIdentify::isLeviathanAxe = false;
            WeaponIdentify::isRelic = false;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
            a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            WeaponIdentify::skipEquipAnim = true;
            WeaponIdentify::unequipWhenAnimEnds = true;
        }

        a_actor->SetGraphVariableBool("bLeviInCatchRange", false);

        kratos->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false, Kratos::Relic::kLeviathanAxe);
        kratos->SetIsCanCallAxe(a_actor);
        isAxeCalled = false;
        isAxeThrowed = true;
        SetThrowState(ThrowState::kThrown);
        WeaponIdentify::EquippedObjR = nullptr;
        WeaponIdentify::lastThrownRelic = Kratos::Relic::kLeviathanAxe;
            spdlog::info("Leviathan Axe throwed, raw damage is: {}", mag);
        //  reset the last throw's traces
        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        data.lastHitActors.clear();
        data.lastHitForms.clear();
        if (a_actor->HasSpell(SpellCatchLevi)) a_actor->RemoveSpell(SpellCatchLevi);
    } else spdlog::info("Leviathan Axe is not equipped for throwing!");
}
void LeviathanAxe::Call(const bool a_justDestroy, const bool a_justContinue, RE::Actor* a_actor)
{
    if (a_actor && data.weap) {
        spdlog::debug("Levi is calling...");
        trailUpdate.Done();
        data.model = nullptr;
        auto stuckedLevi =  LastLeviProjectile ? LastLeviProjectile : nullptr;
        if (!stuckedLevi)   stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
        if (stuckedLevi) {
            if (!a_justContinue) data.position = stuckedLevi->data.location;
            auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
            auto& pFlags = runtimeData.flags;
            if (!(pFlags & (1 << 25))) {
                pFlags |= (1 << 25);
            } else  spdlog::debug("levi is already destroyed");

            if (a_justDestroy) {
                isAxeCalled = false;
                isAxeThrowed = false;
                if (data.stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
                    ObjectUtil::Projectile::DeleteAnExtraArrow(data.stuckedActor, data.model);
#else
                    data.stuckedActor->RemoveExtraArrows3D();
#endif
                    spdlog::debug("levi stucked actor's extra arrows removed");
                    data.stuckedActor = nullptr;
                } else spdlog::debug("levi not stucked anybody");
                    return;
            }
        } else {spdlog::debug("Stucked Levi is nullptr!");}

        if (auto AnArchos = PlayerCharacter::GetSingleton(); !a_justDestroy && AnArchos && SpellLeviProjA) {
            isAxeCalled = true;
            isAxeThrowed = false;

            const auto leviDamage = (float)(data.weap->attackDamage);
            float mag = leviDamage * WeaponIdentify::DamageMult;
            const auto leviProjEff = SpellLeviProjA->effects[0];
            auto& leviProjEffSetting = leviProjEff->effectItem;
            leviProjEffSetting.magnitude = mag * 0.25f;


            RE::NiPoint3 startPoint = data.position;
            RE::NiPoint3  targetPoint = WeaponIdentify::RHandBone ? WeaponIdentify::RHandBone->world.translate : AnArchos->GetPosition();
            if (!a_justContinue) {
                GetPosition(startPoint, AnArchos);
                arrivingLevi.arrivingRelativeAngleZ = 0.5f;
                a_actor->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingLevi.arrivingRelativeAngleZ);
            //    RE::NiMatrix3 handRot = WeaponIdentify::RHandBone->world.rotate;
            //    RE::NiPoint3 palmDir    = handRot * RE::NiPoint3(backVec);
            //    RE::NiPoint3 handForward= handRot * RE::NiPoint3(upVec);
            //    palmDir.Unitize();
            //    handForward.Unitize();
                RE::NiPoint3 p0 = arrivingLevi.startPosition;
                RE::NiPoint3 p3 = WeaponIdentify::RHandBone ? WeaponIdentify::RHandBone->world.translate : AnArchos->GetPosition();
                RE::NiPoint3 linearArrivingDir = p3 - p0;
                linearArrivingDir.Unitize();
                const float distanceFromStart = p0.GetDistance(p3);
            //    RE::NiPoint3 p1 = p0 + linearArrivingDir * distanceFromStart / 3.f;
            //    RE::NiPoint3 p2 = p3 + palmDir * (distanceFromStart / 3.f + 50.f) + handForward * (distanceFromStart / 3.f + 50.f);
            //    float charZ = AnArchos->GetPosition().z;
            //    if (p2.z < charZ) p2.z = charZ;
            //    float segmentCount = Config::ArrivalTime / *g_deltaTimeRealTime;
            //    data.arrivingRoute = MathUtil::Algebra::DrawAndMeasureBezier(p0, p1, p2, p3, (int)segmentCount);
            }
            RE::ProjectileHandle pHandle;
            RE::Projectile::ProjectileRot pRot(MathUtil::Algebra::VectorToPitchYaw(data.lastOrientation));
            RE::Projectile::LaunchData lData(AnArchos, startPoint, pRot, SpellLeviProjA);

            lData.noDamageOutsideCombat = true; //  can be an option
            lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
            lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
#endif
            if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
                lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

            WeaponIdentify::isBarehanded = false;
            RE::Projectile::Launch(&pHandle, lData);
            SetThrowState(ThrowState::kArriving);
            auto kratos = Kratos::GetSingleton();
            if (GetThrowState() == tState::kThrowable) kratos->SetIsCanCallAxe(a_actor, false);

            trailUpdate.RegisterForUpdate(0.01, false);
            
            if (a_justContinue) {
                arrivingLevi.proj = pHandle.get().get();
                arrivingLevi.linearDistanceFromLastCallPos = startPoint.GetDistance(targetPoint);
            //    arrivingLevi.timeToArrive = arrivingLevi.linearDistanceFromLastCallPos / arrivingLevi.speed;
            } else {
                arrivingLevi = ArrivingLeviathan(this, pHandle.get().get(), a_actor, WeaponIdentify::RHandBone, startPoint);
                spdlog::debug("Levi call is started");
            }

            spdlog::info("Levi is arriving...");
        } else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
}
void LeviathanAxe::Catch(const bool a_justDestroy, RE::Actor* a_actor)
{
    if (LeviathanAxeProjectileA) {
        if (APIs::precision || APIs::Request()) {
            APIs::precision->RemoveProjectileCollision(a_actor->GetHandle(), collisionDefinition);
        }

        trailUpdate.Done();
        data.model = nullptr;
        auto& runtimeData = LeviathanAxeProjectileA->GetProjectileRuntimeData();
        runtimeData.flags |= (1 << 25);                                     //  set as destroyed, RE::Projectile::Flags::kDestroyed
        if (a_justDestroy) return;
    }
    if (a_actor && !WeaponIdentify::isLeviathanAxe) {
        a_actor->SetGraphVariableBool("bLeviInCatchRange", true);
        if (EffCatchLevi && SpellCatchLevi && !a_actor->AsMagicTarget()->HasMagicEffect(EffCatchLevi)) {
            a_actor->AddSpell(SpellCatchLevi);
        }

        SetThrowState(ThrowState::kArrived);

        auto kratos = Kratos::GetSingleton();

        if (auto handEffect = kratos->VFXeffect.handFrost; handEffect) 
            a_actor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);

        if (auto catchSound = kratos->soundEffect.catchLevi; catchSound) 
            ObjectUtil::Sound::PlaySound(catchSound, WeaponIdentify::RHandBone, 2.f);

        if (WeaponIdentify::LeviathanAxe) {
            WeaponIdentify::unequipWhenAnimEnds = false;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
            a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            kratos->DoKratosAction(Kratos::Action::kWeaponCharge);
            ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::LeviathanAxe, WeaponIdentify::skipEquipAnim);//, 1U, true, false, false, true);
            ResetEquipAnimationAfter(100, a_actor);
            WeaponIdentify::WeaponCheck();
            if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
        } else spdlog::warn("you not have the leviathan axe");

        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        data.lastHitActors.clear();
        data.lastHitForms.clear();

        isAxeCalled = false;
        std::jthread delayedCast([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            if (GetThrowState() == tState::kThrowable) kratos->SetIsCanCallAxe(a_actor, false);
        });
        delayedCast.detach();
    }
}
void LeviathanAxe::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_coolDown)
{
    auto kratos     = Kratos::GetSingleton();
    auto AnArchos   = RE::PlayerCharacter::GetSingleton();
    auto ench       = data.weap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, data.weap) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto leviDam  = data.damage;

    if (data.weap && chargeHitCount <= 0) {
        if (enchBase) {
            spdlog::debug("levi's enchantment is: {}", ench->GetName());
            if (enchBase->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier)
             || enchBase->HasArchetype(RE::EffectSetting::Archetype::kValueModifier)) {
                ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                auto& mag = enchEffect->effectItem.magnitude;
                data.enchMag = &mag;
                data.defaultEnchMag = mag;
                mag *= a_magnitude;
                chargeHitCount = a_chargeHitCount;
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                kratos->chargeData.lastBuffedLevi = data.weap->formID;
                kratos->SetIsCanCharge(AnArchos, false);
                _isCharged = true;
                spdlog::debug("magnitude buffing from {} to: {}", mag / a_magnitude, mag);

                auto weapBone = WeaponIdentify::WeaponBone;
                if (auto handEffect = kratos->VFXeffect.handFrostBright; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                else spdlog::warn("can't found hand effect for levi charge!");

                if (auto soundEffect = kratos->soundEffect.chargeLeviEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
            } else spdlog::debug("levi's enchantment is not expected archetype.");
#ifdef EXPERIMENTAL_CHARGE_LEVI
        } else {
            spdlog::debug("levi not has any enchantment, levi is enchanting...");
            if (EnchCharge) {
                ench = EnchCharge;
                enchEffect  = ench ? ench->effects[0] : nullptr;
                enchBase     = enchEffect ? enchEffect->baseEffect : nullptr;
                if (enchBase) {
                    ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                    ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 300.f, false, false);
                    ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 300.f);
                    kratos->chargeData.lastChargedLevi = data.weap->formID;
                    _isCharged = true;

                //  AnArchos->GetActorRuntimeData().emotionType = RE::EmotionType::kAnger;
                //  AnArchos->GetActorRuntimeData().emotionValue = 100;

                    auto& enchCost = ench->data.costOverride;
                    auto& enchAmount = ench->data.chargeOverride;
                    auto& mag = enchEffect->effectItem.magnitude;
                    enchAmount = 500.f;
                    mag = a_magnitude * leviDam / 2;
                    enchCost = mag;
                    data.enchMag = nullptr;

                    chargeHitCount = a_chargeHitCount;
                    kratos->SetIsCanCharge(AnArchos, false);
                    spdlog::info("levi charge frost damage buff is: {}", mag);

                    auto weapBone = WeaponIdentify::WeaponBone;
                    if (auto handEffect = kratos->VFXeffect.handFrostBright; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, weapBone);
                    else spdlog::warn("can't found hand effect for levi charge!");

                    if (auto soundEffect = kratos->soundEffect.chargeLeviEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, weapBone, 5.f);
                } else spdlog::error("can't find frost enchantment's base!!");
            } else spdlog::error("can't find frost enchantment!!");
#endif
        }
    } else spdlog::error("can't find levi for charging!!");
}
void LeviathanAxe::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    auto kratos = Kratos::GetSingleton();
    if (a_justReset) {
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), data.weap) == EnchCharge) {
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), data.weap);
            _isCharged = false;
        }
    } else if (kratos->IsEquipped(Kratos::Relic::kLeviathanAxe)) {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), data.weap) == EnchCharge)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), data.weap);
            }
            kratos->SetIsCanCharge(RE::PlayerCharacter::GetSingleton());
            _isCharged = false;
        } else if (!a_justCheck) {chargeHitCount -= 1;}
    }
}
void LeviathanAxe::SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical)
{
    if  (a_vertical) {
        a_matrix.entry[0][0] = 0.02f;   //  const
        a_matrix.entry[0][1] = -0.54f;  //  same
        a_matrix.entry[0][2] = 0.84f;   //  
        a_matrix.entry[1][0] = 0.01f;   //  const
        a_matrix.entry[1][1] = 0.84f;   //  
        a_matrix.entry[1][2] = 0.54f;   //  
        a_matrix.entry[2][0] = -1.f;     // const
        a_matrix.entry[2][1] = 0.0f;     // const same
        a_matrix.entry[2][2] = 0.02f;   //  const
    } else {
        a_matrix.entry[0][0] = 0.79f;   //  
        a_matrix.entry[0][1] = -0.54f;  //  same
        a_matrix.entry[0][2] = 0.29f;   //  
        a_matrix.entry[1][0] = 0.51f;   //  
        a_matrix.entry[1][1] = 0.84f;   //  same
        a_matrix.entry[1][2] = 0.19f;   //  
        a_matrix.entry[2][0] = -0.34f;  //  const
        a_matrix.entry[2][1] = 0.0f;     // const same
        a_matrix.entry[2][2] = 0.93f;   //  const
    }

//  vertical axe turn
//  [0.018283, 0.011742, -0.999764]
//  [-0.540305, 0.841469, 0.000002]
//  [0.841270, 0.540178, 0.021729]
//
//  lateral axe turn
//  [0.790722, 0.507721, -0.342020]
//  [-0.540305, 0.841469, 0.000002]
//  [0.287801, 0.184794, 0.939693]
}
void LeviathanAxe::SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical)
{
//  a_angles.x = asin(a_direction.z);
    a_angles.z = atan2(a_direction.x, a_direction.y);
    if  (a_vertical) {
        a_angles.x += Config::MinAxeStuckAngle; //  const
    //  a_angles.y = ;           //= -88.6f;        //  const
    //  a_angles.z = ;           //+= -32.7f;    // 
    } else {
    //  a_angles.x = 0.f;                               //  const
    //  a_angles.y = ;           //= -20.f;     //  const
        a_angles.z += Config::MinAxeStuckAngle; //+= -32.7f;     // 
    }
}
void LeviathanAxe::TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical)
{
    auto rightDir   = a_direction.UnitCross(upVec);
    auto leftDir     = a_direction.UnitCross(downVec);
    auto upDir      = a_direction.UnitCross(rightDir);
    auto downDir     = a_direction.UnitCross(leftDir);

    a_position -= (a_direction * a_offset);
    if (a_vertical) {
        a_position += (downDir * a_offset);
    } else {
        a_position += (leftDir * a_offset);
    }
}
bool LeviathanAxe::IsArriving(RE::Projectile* a_proj) const {return a_proj == arrivingLevi.proj;}
bool LeviathanAxe::IsHoming(RE::Projectile* a_proj) const {return a_proj == homingLevi.proj;}
void LeviathanAxe::StartChargingThrow(RE::Actor* a_actor)
{
    if (auto kratos = Kratos::GetSingleton(); a_actor && kratos) {
        ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviLoop, WeaponIdentify::RHandBone, 5.f, &kratos->_soundHandle);
        a_actor->ApplyArtObject(kratos->VFXeffect.handFrostBright, 5.f, nullptr, false, false, WeaponIdentify::RHandBone);
        kratos->SetIsChargingThrow(true);
    }
}
#pragma endregion
#pragma region Blades_of_Chaos
float BladeOfChaos::GetScorchingSpeed()
{
    RE::PlayerCharacter::GetSingleton()->GetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
    return _fScorchingSpeed;
}
void BladeOfChaos::SetScorchingSpeed(const float a_speed, const bool a_forced)
{
    _fScorchingSpeed = a_speed;
    if (!a_forced && _fScorchingSpeed > 1.f) _fScorchingSpeed = 1.f;

    RE::PlayerCharacter::GetSingleton()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    WeaponIdentify::WeaponBone->AsNode()->GetChildren().back()->GetUserData()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    RE::PlayerCharacter::GetSingleton()->GetEquippedEntryData(false)->object->As<RE::TESObjectREFR>()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    WeaponIdentify::BladeOfChaos->As<RE::WeaponAnimationGraphManagerHolder>()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    reinterpret_cast<RE::IAnimationGraphManagerHolder*>(WeaponIdentify::WeaponBone)->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
}
void BladeOfChaos::BuffScorchingSpeed(const float a_buff, const bool a_forced)
{
    _fScorchingSpeed += a_buff;
    if (!a_forced && _fScorchingSpeed > 1.f) _fScorchingSpeed = 1.f;

    RE::PlayerCharacter::GetSingleton()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    WeaponIdentify::WeaponBone->AsNode()->GetChildren().back()->GetUserData()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    RE::PlayerCharacter::GetSingleton()->GetEquippedEntryData(false)->object->As<RE::TESObjectREFR>()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    WeaponIdentify::BladeOfChaos->As<RE::TESObjectREFR>()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
//    reinterpret_cast<RE::IAnimationGraphManagerHolder*>(WeaponIdentify::WeaponBone)->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
}
void BladeOfChaos::DeBuffScorchingSpeed()
{
    if (AsyncUtil::GameTime::GetEngineTime() >= (_lastChargeTime + 0.4f + (float)(_fScorchingSpeed == 1.f))) {
        BuffScorchingSpeed(-0.04f);
    }
}
bool BladeOfChaos::IsQueueEnd()
{
    if (GetScorchingSpeed() < 0.34f || AsyncUtil::GameTime::GetEngineTime() > (_lastChargeTime + 4.f)) {
        return true;
    }
    DeBuffScorchingSpeed();
    return false;
}
#pragma endregion
#pragma region Draupnir_Spear
bool Draupnir::Initialize()
{
    bool found = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    DraupnirSpearProjBaseL      = dataHandler->LookupForm<RE::BGSProjectile>(0x802, Config::DraupnirModESP);
    DraupnirsCallProjBaseL      = dataHandler->LookupForm<RE::BGSProjectile>(0x818, Config::DraupnirModESP);
    SpellDraupnirProjL          = dataHandler->LookupForm<RE::SpellItem>    (0x800, Config::DraupnirModESP);
    SpellDraupnirsCallProjL     = dataHandler->LookupForm<RE::SpellItem>    (0x805, Config::DraupnirModESP);
    DraupnirExplosion           = dataHandler->LookupForm<RE::BGSExplosion> (0x809, Config::DraupnirModESP);
    DraupnirStuckedFX           = dataHandler->LookupForm<RE::BGSArtObject> (0x819, Config::DraupnirModESP);
    if (DraupnirSpearProjBaseL)
            spdlog::debug("Draupnir Spear projectile is {}", DraupnirSpearProjBaseL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile"); found = false;}
    if (DraupnirsCallProjBaseL)
            spdlog::debug("Draupnir's call projectile is {}", DraupnirsCallProjBaseL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile"); found = false;}
    if (SpellDraupnirProjL)
            spdlog::debug("Draupnir Spear projectile spell is {}", SpellDraupnirProjL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile spell"); found = false;}
    if (SpellDraupnirsCallProjL)
            spdlog::debug("Draupnir's call spell is {}", SpellDraupnirsCallProjL->GetName());
    else     {spdlog::error("Can't find Draupnir Spear projectile spell"); found = false;}
    if (DraupnirExplosion)
            spdlog::debug("Draupnir Spear explosion is {}", DraupnirExplosion->GetName());
    else     {spdlog::error("Can't find Draupnir Spear explosion"); found = false;}

    DefaultDraupnirModel = DraupnirSpearProjBaseL->model;

    return found;
}
void Draupnir::Update(const float a_delta)
{
    if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsEquipped(Kratos::Relic::kDraupnirSpear)) {
        if (kratos->IsChargingThrow()) {
            data.throwingChargeDuration += a_delta;
        //    if (data.throwingChargeDuration >= 2.f) ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviEndT, WeaponIdentify::RHandBone, 5.f);
        }
    }
    if (explosionsStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextExplosionTime) {
            RE::ProjectileHandle pHandle;
            TriggerExplosions(explosionDelay, 15.f, &pHandle);
        }
    } if (draupnirRainStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextLaunchTime) {
            RainOfDraupnir();
        }
    }
}
void Draupnir::Throw()
{
    const auto AnArchos = PlayerCharacter::GetSingleton();
//  auto& runtimeData = AnArchos->GetActorRuntimeData();
    if (SpellDraupnirProjL && WeaponIdentify::DraupnirSpear && WeaponIdentify::isDraupnirSpear)
    {
        const auto effDraupnir = SpellDraupnirProjL->effects[0];
        auto& draupnirProjEffSetting = effDraupnir->effectItem;
        auto& mag = draupnirProjEffSetting.magnitude;
        const auto draupnirDamage = (float)(WeaponIdentify::DraupnirSpear->attackDamage);
        mag = draupnirDamage * WeaponIdentify::DamageMult;
        float throwChargeDamageMult = std::sqrtf(data.throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        mag *= throwChargeDamageMult;

        if (const auto draupnirBaseEffect = effDraupnir->baseEffect; draupnirBaseEffect && draupnirBaseEffect->data.projectileBase) {
            draupnirBaseEffect->data.projectileBase->data.force = mag * 0.5f;
        }

        data.gravity = 2.f / (std::powf(data.throwingChargeDuration + 1.f, 3.f));

        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
        RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);
        lData.weaponSource = WeaponIdentify::DraupnirSpear;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

    //    DraupnirSpearProjBaseL->model = DefaultDraupnirModel;
        RE::Projectile::Launch(&pHandle, lData);
    //  auto effBaseDraupnir = effDraupnir->baseEffect;
    //  if (effBaseDraupnir) {
    //      effBaseDraupnir->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
    //      effBaseDraupnir->data.associatedForm = WeaponIdentify::LeviathanAxe;
    //  } else spdlog::warn("not found throwing effect!");
    //  DraupnirSpearProjBaseL->data.defaultWeaponSource = WeaponIdentify::DraupnirSpear;
    //  AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(SpellDraupnirProjL, false, nullptr, 1.f, false, mag, AnArchos);
    } else  spdlog::info("Draupnir Spear is not equipped for throwing");
}
void Draupnir::MeleeThrow()
{
    const auto AnArchos = PlayerCharacter::GetSingleton();
//  auto& runtimeData = AnArchos->GetActorRuntimeData();
    if (SpellDraupnirProjL && WeaponIdentify::DraupnirSpear && WeaponIdentify::isDraupnirSpear && DraupnirsCallProjBaseL)
    {
        const auto effDraupnir = SpellDraupnirProjL->effects[0];
        auto& leviProjEffSetting = effDraupnir->effectItem;
        leviProjEffSetting.magnitude = 0.f;

        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot{};
        origin = WeaponIdentify::WeaponBone->world.translate;
        AnArchos->Unk_A0(WeaponIdentify::WeaponBone, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);
        lData.weaponSource = WeaponIdentify::DraupnirSpear;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

    //    DraupnirSpearProjBaseL->model = DraupnirsCallProjBaseL->model;
        RE::Projectile::Launch(&pHandle, lData);
        if (pHandle.get().get()) {
            MeleeHitProjectileIDs.emplace_back(pHandle.get().get()->formID);
            spdlog::debug("melee thrown draupnir form id: {}", pHandle.get().get()->formID);
        }
    //    DraupnirSpearProjBaseL->model = DefaultDraupnirModel;
    } else  spdlog::info("Draupnir Spear is not equipped for throwing");
}
void Draupnir::ArtilleryOfTheAncients(const float a_delay, const float a_duration)
{
    if (SpellDraupnirProjL) {
        nextLaunchDelay = a_delay;
        nextLaunchTime = AsyncUtil::GameTime::GetEngineTime();
        lastLaunchTime = AsyncUtil::GameTime::GetEngineTime() + a_duration;
        draupnirRainStarted = true;
        spdlog::info("rain of draupnir triggered");
    }
}
void Draupnir::RainOfDraupnir()
{
    if (nextLaunchTime > lastLaunchTime) {
        draupnirRainStarted = false;
        spdlog::info("rain of draupnir done");
            return;
    }
    //  launch draupnir at a random top position of the nearest target
    const auto AnArchos = PlayerCharacter::GetSingleton();
    const std::vector<RE::Actor*> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(AnArchos, 666.f);
    const bool isThereNearTarget = (!nearCombatTargets.empty() && nearCombatTargets.front()) ? true : false;
    RE::NiPoint3 origin = isThereNearTarget ? nearCombatTargets.front()->GetPosition() : (AnArchos->GetPosition() + MathUtil::Algebra::GetForwardVector(AnArchos) * 360.f);
    origin = origin + RE::NiPoint3(MathUtil::Algebra::GenerateRandomFloat(-100.f, 100.f), MathUtil::Algebra::GenerateRandomFloat(-100.f, 100.f), 2000.f);
    RE::ProjectileHandle pHandle;
    RE::Projectile::ProjectileRot pRot{};
    pRot.x = PI2 + MathUtil::Algebra::GenerateRandomFloat(-0.018f, 0.018f);
//    spdlog::debug("aim angle: {} heading angle: {}", pRot.x, pRot.z);
    RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);
    lData.weaponSource = WeaponIdentify::DraupnirSpear;
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

    RE::Projectile::Launch(&pHandle, lData);

    nextLaunchTime = AsyncUtil::GameTime::GetEngineTime() + nextLaunchDelay;
}
void Draupnir::Call(const float a_damage, const float a_force)  //detonate throwed draupnir spears like ragnarok
{
    if (WeaponIdentify::DraupnirSpear && SpellDraupnirsCallProjL && DraupnirExplosion) {
        DraupnirExplosion->data.force = a_force * explosionMagnitude;
        StartExplosions(Config::DraupnirExplosionsInterval);
    }
}
void Draupnir::AddSpearHit(RE::NiNode* bone, RE::Actor* actor)
{
    if (bone) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectile] = spearHits.front();
            if (oldProjectile) oldProjectile->GetProjectileRuntimeData().flags |= (1 << 25);
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(bone, actor, nullptr);
    }
}
void Draupnir::AddSpearHit(RE::Projectile* a_proj)
{
    if (a_proj) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectile] = spearHits.front();
            if (oldProjectile) oldProjectile->GetProjectileRuntimeData().flags |= (1 << 25);
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(nullptr, nullptr, a_proj);
    }
}
void Draupnir::StartExplosions(const float a_delay)
{
    if (spearHits.empty() && explosionsStarted) {
        spdlog::info("there is no thrown explodable draupnir spear");
        explosionsStarted = false;
        return;
    }
    explosionDelay = a_delay;
    nextExplosionTime = AsyncUtil::GameTime::GetEngineTime();// + a_delay;
    explosionsStarted = true;
    spdlog::info("draupnir explosions triggered");
}
inline void Draupnir::TriggerExplosions(float a_delay, float a_force, RE::ProjectileHandle* a_pHandle)
{
    if (spearHits.empty()) {
        if (AsyncUtil::GameTime::GetEngineTime() > timeToDoneExplosions) {
            explosionsStarted = false;
            currentHitIndex = 0;
            SetExplosionMagnitude(1.f);
            spdlog::info("draupnir explosion chain finished");
        } //spdlog::info("all draupnir explosions done");
    } else {
        auto& [bone, target, proj] = spearHits.front();
        currentHitIndex++;
        if (bone && target && a_pHandle) {
            TriggerExplosionAtLocation(bone, a_pHandle, target);
            target->RemoveExtraArrows3D();
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else if (proj && a_pHandle) {
            TriggerExplosionAtLocation(proj, a_pHandle);
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else spdlog::info("{}. draupnir explosion is empty!", currentHitIndex);
        spearHits.erase(spearHits.begin());
        nextExplosionTime = AsyncUtil::GameTime::GetEngineTime() + a_delay;
        timeToDoneExplosions = nextExplosionTime + 0.3f;
    }
}
inline void Draupnir::TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle)
{
    RE::Projectile::LaunchData lData(RE::PlayerCharacter::GetSingleton(), a_proj->data.location, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProjL);
    RE::Projectile::Launch(a_pHandle, lData);
    a_proj->GetProjectileRuntimeData().flags |= (1 << 25);
//    a_proj->GetProjectileRuntimeData().explosionTimer = 0;    //  this method just works but explodes at the first hit position, not current 3d position.
}
inline void Draupnir::TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target)
{
    RE::Projectile::LaunchData lData(RE::PlayerCharacter::GetSingleton(), a_bone->world.translate, RE::Projectile::ProjectileRot(), SpellDraupnirsCallProjL);
    lData.desiredTarget = a_target;
    RE::Projectile::Launch(a_pHandle, lData);
    auto damage = ObjectUtil::Actor::GetWeaponAttackDamage(RE::PlayerCharacter::GetSingleton(), a_target, WeaponIdentify::DraupnirSpear);
    damage /= ((1.f + (float)(RE::PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty)) / 5.f);
    damage *= explosionMagnitude;
    FenixUtils::stagger(std::clamp(damage/10.f, 0.1f, 100.f), a_target, RE::PlayerCharacter::GetSingleton());
    if (auto targetAVO = a_target->AsActorValueOwner(); targetAVO)
        targetAVO->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damage);
}
void Draupnir::StartChargingThrow(RE::Actor* a_actor)
{
    if (auto kratos = Kratos::GetSingleton(); a_actor && kratos) {
    //    ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviLoop, WeaponIdentify::RHandBone, 5.f, &kratos->_soundHandle);
        a_actor->ApplyArtObject(kratos->VFXeffect.handFlame, 5.f, nullptr, false, false, WeaponIdentify::RHandBone);
        kratos->SetIsChargingThrow(true);
    }
}
#pragma endregion
#pragma region Mjolnir
Mjolnir* Mjolnir::GetSingleton()                {static Mjolnir singleton; return &singleton;}
bool Mjolnir::Initialize()
{
    bool found = true;
/**/    auto dataHandler = RE::TESDataHandler::GetSingleton();
    MjolnirProjBaseT    = dataHandler->LookupForm<RE::BGSProjectile>    (0x840, Config::KratosCombatESP);
    MjolnirProjBaseA    = dataHandler->LookupForm<RE::BGSProjectile>    (0x841, Config::KratosCombatESP);
    SpellMjolnirProjT   = dataHandler->LookupForm<RE::SpellItem>        (0x842, Config::KratosCombatESP);
    SpellMjolnirProjA   = dataHandler->LookupForm<RE::SpellItem>        (0x846, Config::KratosCombatESP);
    SpellCatchMjolnir   = dataHandler->LookupForm<RE::SpellItem>        (0x81D, Config::KratosCombatESP);
    EnchCharge          = dataHandler->LookupForm<RE::EnchantmentItem>  (0x84C, Config::KratosCombatESP);   // +30 shock damage, ff, touch
    if (MjolnirProjBaseT && MjolnirProjBaseA)
            spdlog::debug("Mjolnir projectiles are {} and {}", MjolnirProjBaseT->GetName(), MjolnirProjBaseA->GetName());
    else     {spdlog::error("Can't find Mjolnir projectiles"); found = false;}
    if (SpellMjolnirProjT)
            spdlog::debug("Mjolnir throwing spell is {}", SpellMjolnirProjT->GetName());
    else     {spdlog::error("Can't find Mjolnir projectile spells"); found = false;}
    if (SpellMjolnirProjA)
            spdlog::debug("Mjolnir calling spell is {}", SpellMjolnirProjA->GetName());
    else     {spdlog::error("Can't find Mjolnir calling spell"); found = false;}
    if (SpellCatchMjolnir) {
            EffCatchMjolnir = SpellCatchMjolnir->effects[0]->baseEffect;
            spdlog::debug("Mjolnir catching spell is {}", SpellCatchMjolnir->GetName()); EffCatchMjolnir = SpellCatchMjolnir->effects[0]->baseEffect;}
    else     {spdlog::error("Can't find Mjolnir catching spell"); found = false;}
    if (EnchCharge) {           
            spdlog::debug("Default Mjolnir charging enchantment is {}", EnchCharge->GetName());}
    else     {spdlog::error("Can't find default Mjolnir enchantment spell"); found = false;}

    return found;
}
void Mjolnir::Update(const float a_delta) {
    if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsEquipped(Kratos::Relic::kMjolnir)) {
        if (kratos->IsChargingThrow()) {
            data.throwingChargeDuration += a_delta;
        //    if (data.throwingChargeDuration >= 2.f) ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviEndT, WeaponIdentify::RHandBone, 5.f);
        }
    }
    if (callUpdate.IsTimeToUpdate()) {
        Call();
    }
    if (false && Config::DrawTrails) {
        if (auto rootBone = data.model; rootBone) {
            if (trailUpdate.IsTimeToUpdate()) {
                auto bone = rootBone;
#ifdef PRECISION
                if (bone && (APIs::precision || APIs::Request())) {
                    trailUpdate.Done();
                    auto trailNode = RE::NiNode::Create(0);
                    if (auto node = bone->AsNode(); node) node->AttachChild(trailNode, true);
                //    PRECISION_API::TrailOverride trailOverride;//(2.f, RE::NiColorA(), 2.f, Config::TrailModelPathLevi.data());
                //    trailOverride.meshOverride = Config::TrailModelPathLevi.data();
                    APIs::precision->AddAttackTrail(trailNode, RE::PlayerCharacter::GetSingleton()->GetHandle(), RE::PlayerCharacter::GetSingleton()->GetParentCell(), LastMjolnirProjectile, std::nullopt);
                //    APIs::precision->AddAttackCollision(RE::PlayerCharacter::GetSingleton()->GetHandle(), collisionDefinition, LastLeviProjectile);
                }
            }
#else
                float particleMult = _isCharged ? 2.f : 1.f;
                float length = bone->worldBound.radius;//60.f;
            //    spdlog::debug("Mjolnir projectile length is {}", length);
                float tipOffset = length;// / 3.f;
                data.trailTransform = bone->world * bone->local;
                data.trailTransform.translate = data.trailTransform * RE::NiPoint3(0.f, 0.f, tipOffset);

                auto projTrail = RE::NiPointer<RE::BSTempEffectParticle>(RE::BSTempEffectParticle::Spawn(RE::PlayerCharacter::GetSingleton()->GetParentCell(), 10.f, Config::TrailModelPathMjolnir.data(), data.trailTransform.rotate, data.trailTransform.translate, 0.01f * length, 7, nullptr));
                if (data.projTrails.size() < particleMult * Config::TrailParticleCount) {
                    data.projTrails.insert(data.projTrails.begin(), projTrail);
                }
            //    if (data.projTrails.size() > particleMult * Config::TrailParticleCount) {
            //    //    data.projTrails.pop_back();
            //        trailUpdate.Done();
            //    }
            }
        }
        if (!data.projTrails.empty()) {
            uint8_t trailIndex(1);
            data.projTrails.erase(
                std::remove_if(data.projTrails.begin(), data.projTrails.end(),
                    [a_delta, &trailIndex, this](RE::NiPointer<RE::BSTempEffectParticle> projTrail) {
                        if (projTrail) {
                            if (projTrail->age >= projTrail->lifetime) {
                                projTrail->~BSTempEffectParticle();
                                projTrail.~NiPointer();
                            //    spdlog::debug("{}. projectile trail removed! age: {} lifetime: {}", trailIndex, projTrail->age, projTrail->lifetime);
                                return true; // Mark for removal
                            } else {
                                if (projTrail->age > 1.f) projTrail->age += a_delta;    //  projTrail->lifetime can't be changed, it is always 1.f;
                            //    spdlog::debug("{}. projectile trail age is {}", trailIndex, projTrail->age);
                            }
                        //    trailIndex++;
                        }
                        return false; // Keep in the vector
                    }),
                data.projTrails.end());
            for (auto& projTrail : data.projTrails) {
                auto particleObject = projTrail ? projTrail->particleObject : nullptr;
                auto fadeNode = particleObject ? particleObject->AsFadeNode() : nullptr;
                auto trailRoot = fadeNode ? fadeNode->GetObjectByName("TrailRoot"sv) : nullptr;
                auto trailRootNode = trailRoot ? trailRoot->AsNode() : nullptr;
                if (trailRootNode && !trailRootNode->GetChildren().empty()) {
                    trailRootNode->world = data.trailTransform;
                //    trailRootNode->local = ObjectUtil::Node::GetLocalTransform(trailRootNode, data.trailTransform);
                //    spdlog::debug("{}. projectile trail transform updated! age: {} lifetime: {}", trailIndex, projTrail->age, projTrail->lifetime);
                } trailIndex++;
#endif
            }
        }
    }
}
tStateM Mjolnir::GetThrowState()     const      {return throwState;}
void Mjolnir::SetThrowState(ThrowState a_state) {throwState = a_state;}
bool Mjolnir::GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor)
{
    bool result = false;
    if (!a_actor) return result;
    auto thrownMjolnir =  LastMjolnirProjectile;
//    if (!thrownMjolnir) thrownMjolnir = MjolnirProjectileT;
    if (thrownMjolnir) {
        a_point = thrownMjolnir->data.location;
        result = true;
    } else {
        spdlog::debug("we can't get thrown Mjolnir proj!");
        a_point = data.position;
        result = false;
    }

    if (GetThrowState() == tStateM::kThrowable) {
        if (auto backWeaponSheathe = a_actor->GetNodeByName("WeaponMace"); backWeaponSheathe) {
            a_point = backWeaponSheathe->world.translate;
        }
    } else {
        auto pcCell = a_actor->parentCell;
        auto bound = pcCell->GetRuntimeData().cellData.exterior->worldX;
        auto pcPos = a_actor->GetPosition();
        float dist = pcPos.GetDistance(a_point);
        if (dist > 36000.f) {   // ~42000 is limit
            spdlog::info("Mjolnir too far from you! ({} m)", (int)dist / 100);
            auto dir = a_point - pcPos;
            dir.Unitize();
            a_point = pcPos + dir * 36000.f;
        }
    }
    return result;
}
void Mjolnir::Throw(const bool justContinue, const bool a_isVertical, const bool isHoming, RE::Actor* a_actor)
{
    if (!a_actor) {spdlog::warn("Mjolnir::Throw - a_actor is null"); return;}

    bool isVertical = a_isVertical;
    bool isThrowAttack = false;
    bool isPowerThrowAttack = false;
    if (Config::IsAdvancedThrowingInstalled) {
        a_actor->GetGraphVariableBool("bIsThrowing", isThrowAttack);
        a_actor->GetGraphVariableBool("bIsPowerThrowing", isPowerThrowAttack);
        isVertical = isPowerThrowAttack;
    }
    const auto MjolnirThrowSpell = (isVertical ? SpellMjolnirProjT : SpellMjolnirProjT);
    if (MjolnirThrowSpell && (WeaponIdentify::isMjolnir || justContinue)) 
    {   //  calculate damage
        const auto MjolnirProjEff = MjolnirThrowSpell->effects[0];
        auto& MjolnirProjEffSetting = MjolnirProjEff->effectItem;
        auto& mag = MjolnirProjEffSetting.magnitude;
        const auto MjolnirDamage = static_cast<float>(data.weap->attackDamage);
        mag = MjolnirDamage * WeaponIdentify::DamageMult;
        bool isPowerThrow; a_actor->GetGraphVariableBool("IsPowerThrowing", isPowerThrow);
        if (isVertical || isPowerThrow) {mag *= 1.5f; data.yAngle = 1.57f;}
        else data.yAngle = 0.f;
        float throwChargeDamageMult = std::sqrtf(data.throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        mag *= throwChargeDamageMult;

        if (const auto MjolnirProjBaseEff = MjolnirProjEff->baseEffect; MjolnirProjBaseEff) {
            MjolnirProjBaseEff->data.projectileBase->SetModel(data.weap->GetModel());
            MjolnirProjBaseEff->data.projectileBase->data.speed = justContinue ? MjolnirProjBaseEff->data.projectileBase->data.speed * 0.8f : Config::ThrowSpeed * 0.7f;
            MjolnirProjBaseEff->data.projectileBase->data.force = mag * 1.5f;
        } else spdlog::warn("not found mjolnir throwing effect!");

        if (!justContinue) {
            data.gravity = 3.f / (std::powf(data.throwingChargeDuration + 1.f, 3.f));
        }

        auto kratos = Kratos::GetSingleton();
        //  set the launch data
        auto origin = justContinue ? data.position : WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = RE::Projectile::ProjectileRot(a_actor->GetAimAngle(), a_actor->GetAimHeading());
        if (data.model && (justContinue/* || isHoming*/)) pRot = MathUtil::Algebra::VectorToPitchYaw(data.lastOrientation);//a_actor->Unk_A0(data.model, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, MjolnirThrowSpell);

        lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(a_actor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor);
        //  throw the projectile
        RE::Projectile::Launch(&pHandle, lData);

        trailUpdate.RegisterForUpdate(0.01f, false);

        if (isHoming) {
            if (justContinue) {
                homingMjolnir.proj = homingMjolnir.isBoomerang || !homingMjolnir.targets.empty() ? pHandle.get().get() : nullptr;
            } else {
                std::vector<RE::Actor *> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(a_actor, Config::HProjectileTargetRange, true);
                homingMjolnir = HomingMjolnir(pHandle.get().get(), nearCombatTargets, a_actor, Config::HProjectileLoopCount, Config::ReturnHProjectileAfterLoops);
            }
        }
        if (justContinue) return;
        if (Config::IsAdvancedThrowingInstalled && (isThrowAttack || isPowerThrowAttack)) {
            ResetCharge(data.enchMag, data.defaultEnchMag, true);
            WeaponIdentify::skipEquipAnim = true;
            ObjectUtil::Actor::UnEquipItem(a_actor, false, false, true, true, WeaponIdentify::skipEquipAnim, false);
            ResetEquipAnimationAfter(100, a_actor);
            spdlog::debug("Mjolnir unequipped after throwing");
        } else {
            WeaponIdentify::isMjolnir = false;
            WeaponIdentify::isRelic = false;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
            a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            WeaponIdentify::skipEquipAnim = true;
            WeaponIdentify::unequipWhenAnimEnds = true;
        }

        a_actor->SetGraphVariableBool("bLeviInCatchRange", false);


    //    kratos->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false, Kratos::Relic::kMjolnir);
        kratos->SetIsCanCallMjolnir(a_actor);
        isMjolnirCalled = false;
        isMjolnirArriving = false;
        isMjolnirThrowed = true;
        SetThrowState(ThrowState::kThrown);
        WeaponIdentify::EquippedObjR = nullptr;
        WeaponIdentify::lastThrownRelic = Kratos::Relic::kMjolnir;
            spdlog::info("Mjolnir throwed, raw damage is: {}", mag);
        //  reset the last throw's traces
        data.lastHitActors.clear();
        data.lastHitForms.clear();
        if (a_actor->HasSpell(SpellCatchMjolnir)) a_actor->RemoveSpell(SpellCatchMjolnir);
    } else spdlog::info("Mjolnir is not equipped for throwing!");
}
void Mjolnir::Call(const bool a_justDestroy, const bool a_justContinue, std::optional<float> a_delay, RE::Actor* a_actor)
{
    auto kratos = Kratos::GetSingleton();
    const bool isDelayed = a_delay.has_value() && a_delay != 0.f;
    const bool isDelayedConfig = Config::MjolnirArrivingDelay.has_value() && Config::MjolnirArrivingDelay != 0.f;
    if (!a_justContinue && (isDelayed || (!isDelayed && !isDelayedConfig))) {
        if (isDelayed) callUpdate.RegisterForUpdate(*a_delay, true);
        ObjectUtil::Sound::PlaySound(kratos->soundEffect.fingerSnap, WeaponIdentify::RHandBone, 5.f);
        isMjolnirCalled = true;
        isMjolnirArriving = false;
        RE::NiPoint3 pos;
        GetPosition(pos, a_actor);
        arrivingMjolnir = ArrivingMjolnir(a_actor, WeaponIdentify::RHandBone, pos);

        spdlog::debug("Mjolnir is calling...");
    }
    if (a_delay.has_value() && a_delay != 0.f && (isDelayed || isDelayedConfig)) {
        spdlog::debug("waiting the delay {} seconds...", a_delay.has_value() ? *a_delay : 0.f);
    } else if (a_actor && data.weap) {
        trailUpdate.Done();
        data.model = nullptr;
        auto thrownMjolnir =  LastMjolnirProjectile;
        if (!thrownMjolnir)   thrownMjolnir = MjolnirProjectileT;
        if (thrownMjolnir) {
            data.position = thrownMjolnir->data.location;
            auto& runtimeData = thrownMjolnir->GetProjectileRuntimeData();
            auto& pFlags = runtimeData.flags;
            if (!(pFlags & (1 << 25))) {
                pFlags |= (1 << 25);
            } else  spdlog::debug("Mjolnir is already destroyed");

            if (a_justDestroy) {
                isMjolnirArriving = false;
                isMjolnirThrowed = false;
                    return;
            }
        } else {spdlog::debug("thrown Mjolnir is nullptr!");}

        if (auto AnArchos = PlayerCharacter::GetSingleton(); !a_justDestroy && AnArchos && SpellMjolnirProjA) {
            isMjolnirArriving = true;
            isMjolnirThrowed = false;

            const auto MjolnirDamage = (float)(data.weap->attackDamage);
            float mag = MjolnirDamage * WeaponIdentify::DamageMult;
            const auto MjolnirProjEff = SpellMjolnirProjA->effects[0];
            auto& MjolnirProjEffSetting = MjolnirProjEff->effectItem;
            MjolnirProjEffSetting.magnitude = mag * 0.25f;
            if (const auto MjolnirProjBaseEff = MjolnirProjEff->baseEffect; MjolnirProjBaseEff) {
                MjolnirProjBaseEff->data.projectileBase->SetModel(data.weap->GetModel());
            } else spdlog::warn("not found Mjolnir throwing effect!");

            RE::NiPoint3 startPoint = data.position;
            GetPosition(startPoint, AnArchos);
            RE::NiPoint3  targetPoint = WeaponIdentify::RHandBone ? WeaponIdentify::RHandBone->world.translate : AnArchos->GetPosition();

            RE::ProjectileHandle pHandle;
            RE::Projectile::ProjectileRot pRot(MathUtil::Algebra::VectorToPitchYaw(data.lastOrientation));
            RE::Projectile::LaunchData lData(AnArchos, startPoint, pRot, SpellMjolnirProjA);

            lData.noDamageOutsideCombat = true; //  can be an option
            lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
            lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
#endif
            if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
                lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

            WeaponIdentify::isBarehanded = false;
            data.model = nullptr;
            RE::Projectile::Launch(&pHandle, lData);
            trailUpdate.RegisterForUpdate(0.01f, false);

            if (a_justContinue) {
                arrivingMjolnir.proj = pHandle.get().get();
                arrivingMjolnir.linearDistanceFromLastCallPos = startPoint.GetDistance(targetPoint);
            //    arrivingMjolnir.timeToArrive = arrivingLevi.linearDistanceFromLastCallPos / arrivingLevi.speed;
            } else {
                arrivingMjolnir = ArrivingMjolnir(this, pHandle.get().get(), a_actor, WeaponIdentify::RHandBone, startPoint);
            }

            SetThrowState(ThrowState::kArriving);
            if (GetThrowState() == tStateM::kThrowable) kratos->SetIsCanCallMjolnir(a_actor, false);
            spdlog::info("Mjolnir is arriving...");
        } else {spdlog::warn("WEIRD SpellMjolnirProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the Mjolnir for calling!!");}
}
void Mjolnir::Catch(const bool a_justDestroy, RE::Actor* a_actor)
{
    if (MjolnirProjectileA) {
        trailUpdate.Done();
        data.model = nullptr;
        auto& runtimeData = MjolnirProjectileA->GetProjectileRuntimeData();
        runtimeData.flags |= (1 << 25);                                     //  set as destroyed, RE::Projectile::Flags::kDestroyed
        if (a_justDestroy) return;
    }
    if (a_actor && !WeaponIdentify::isMjolnir) {
        a_actor->SetGraphVariableBool("bLeviInCatchRange", true);
        if (EffCatchMjolnir && SpellCatchMjolnir && !a_actor->AsMagicTarget()->HasMagicEffect(EffCatchMjolnir)) {
            a_actor->AddSpell(SpellCatchMjolnir);
        }

        SetThrowState(ThrowState::kArrived);

        auto kratos = Kratos::GetSingleton();

        if (auto handEffect = kratos->VFXeffect.handShock; handEffect) 
            a_actor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);

        if (auto catchSound = kratos->soundEffect.catchMjolnir; catchSound) 
            ObjectUtil::Sound::PlaySound(catchSound, WeaponIdentify::RHandBone, 2.f);

        if (WeaponIdentify::Mjolnir) {
            WeaponIdentify::unequipWhenAnimEnds = false;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
            a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            kratos->DoKratosAction(Kratos::Action::kWeaponCharge);
            ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::Mjolnir, WeaponIdentify::skipEquipAnim);//, 1U, true, false, false, true);
            ResetEquipAnimationAfter(100, a_actor);
            if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
        } else spdlog::warn("you not have the Mjolnir");

        data.lastHitActors.clear();
        data.lastHitForms.clear();

        isMjolnirCalled = false;
        std::jthread delayedCast([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            if (GetThrowState() == tStateM::kThrowable) kratos->SetIsCanCallMjolnir(a_actor, false);
        });
        delayedCast.detach();
    }
}
void Mjolnir::Charge(const uint8_t a_chargeHitCount, const float a_magnitude, const uint8_t a_stage, const uint8_t a_coolDown)
{
    auto kratos     = Kratos::GetSingleton();
    auto AnArchos   = RE::PlayerCharacter::GetSingleton();
    auto ench       = data.weap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, data.weap) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto mjolnirDam  = data.damage;

    if (data.weap) {
        if (auto handEffect = kratos->VFXeffect.handShock; handEffect) AnArchos->ApplyArtObject(handEffect, a_chargeHitCount * 2, nullptr, false, false, WeaponIdentify::WeaponBone);
        if (auto soundEffect = kratos->soundEffect.chargeMjolnirEnd; soundEffect) ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::WeaponBone, 5.f);
        if (kratos && (kratos->vanillaSpell.thunderbolt || kratos->kittyTailSpell.lightningFlood)) {
            RE::ProjectileHandle pHandle, pHandle2;
            auto origin = WeaponIdentify::WeaponBone->world.translate + RE::NiPoint3(0.f, 0.f, 2000.f);
            auto origin2 = WeaponIdentify::WeaponBone->world.translate;
            auto pRot = RE::Projectile::ProjectileRot(PI2, 0.f);
            auto pRot2 = RE::Projectile::ProjectileRot(-PI2, 0.f);
#ifdef OVERPOWERED
            RE::Projectile::LaunchData lData(AnArchos, origin, pRot, kratos->kittyTailSpell.lightningFlood ? kratos->kittyTailSpell.lightningFlood : kratos->vanillaSpell.thunderbolt);
#else
            RE::Projectile::LaunchData lData(AnArchos, origin, pRot, kratos->kittyTailSpell.lightningFlood && a_stage == 3u ? kratos->kittyTailSpell.lightningFlood : kratos->vanillaSpell.thunderbolt);
#endif
            RE::Projectile::LaunchData lData2(AnArchos, origin2, pRot2, kratos->vanillaSpell.lightningBolt);
            lData.noDamageOutsideCombat, lData2.noDamageOutsideCombat = true;
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
                    RE::Projectile::LaunchData lData3(AnArchos, origin2, pRot3, kratos->vanillaSpell.lightningBolt);
                    lData3.autoAim = false;
                    lData3.alwaysHit = true;
                    lData3.desiredTarget = target;
                    RE::Projectile::Launch(&pHandle3, lData3);
                    launchedProjectileCount++;
                    if (launchedProjectileCount >= projectileCount) break; //  limit the projectiles count
                }
                for (float i = launchedProjectileCount * angleStep; launchedProjectileCount < projectileCount && i < 6.28f; i += angleStep) {
                    RE::ProjectileHandle pHandle3;
                    auto origin3 = WeaponIdentify::WeaponBone->world.translate;
                    RE::Projectile::ProjectileRot pRot3(PI8, i);
                    RE::Projectile::LaunchData lData3(AnArchos, origin3, pRot3, kratos->vanillaSpell.lightningBolt);
                    lData3.autoAim = false;
                    lData3.alwaysHit = true;    //  otherwise it will not hit even colliding with a target
                    RE::Projectile::Launch(&pHandle3, lData3);
                    launchedProjectileCount++;
                    if (launchedProjectileCount >= projectileCount) break; //  limit the projectiles count
                }
            }
        }

        if (enchBase && (enchBase->HasArchetype(RE::EffectSetting::Archetype::kDualValueModifier) || enchBase->HasArchetype(RE::EffectSetting::Archetype::kValueModifier))) {
            if (ench == EnchCharge) {
                spdlog::debug("mjolnir is already charged with {}, buffing now...", ench->GetName());
                ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 100.f * (float)a_stage, false, false);
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 100.f * (float)a_stage);
                auto& enchCost = ench->data.costOverride;
                auto& enchAmount = ench->data.chargeOverride;
                auto& mag = enchEffect->effectItem.magnitude;
                const float defMag = mag;
                enchAmount = 200.f * (float)a_stage;
                mag = a_magnitude * ((float)a_stage / 3.f) * (mjolnirDam / 2.f);
                enchCost = mag;
                data.enchMag = nullptr;
                if (mag > WeaponIdentify::DamageMult * data.weap->attackDamage) {
                    mag = WeaponIdentify::DamageMult * data.weap->attackDamage;
                    spdlog::warn("mjolnir's charge damage is too high, capped to: {}", mag);
                } else spdlog::debug("magnitude buffed from {} to: {}", defMag, mag);
                kratos->chargeData.lastBuffedMjolnir = data.weap->formID;
                _isCharged = true;
            //    if (a_stage == 3u) {
                    chargeHitCount = a_chargeHitCount;
                //    kratos->SetIsCanCharge(AnArchos, false, Kratos::Relic::kMjolnir);
            //    }
            } else {
                spdlog::debug("mjolnir's enchantment is: {}", ench->GetName());
                ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                auto& mag = enchEffect->effectItem.magnitude;
                const float defMag = mag;
                data.enchMag = &mag;
                data.defaultEnchMag = mag;
                mag = a_magnitude * ((float)a_stage / 3.f) * (mjolnirDam / 2.f);
                ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 100.f * (float)a_stage);
                kratos->chargeData.lastBuffedMjolnir = data.weap->formID;
                _isCharged = true;
            //    if (a_stage == 3u) {
                    chargeHitCount = a_chargeHitCount;
                //    kratos->SetIsCanCharge(AnArchos, false, Kratos::Relic::kMjolnir);
            //    }
                spdlog::debug("magnitude buffed from {} to: {}", defMag, mag);
            }
#ifdef EXPERIMENTAL_CHARGE_LEVI
        } else {
            spdlog::debug("mjolnir not has any damage enchantment, mjolnir is enchanting...");
            if (EnchCharge) {
                ench = EnchCharge;
                enchEffect  = ench ? ench->effects[0] : nullptr;
                enchBase    = enchEffect ? enchEffect->baseEffect : nullptr;
                if (enchBase) {
                    ResetCharge(data.enchMag, data.defaultEnchMag, false, true);
                    ObjectUtil::Enchantment::EnchantEquippedWeapon(AnArchos, ench, 100.f * (float)a_stage, false, false);
                    ObjectUtil::Enchantment::ChargeEquippedWeapon(AnArchos, 100.f * (float)a_stage);
                    kratos->chargeData.lastChargedMjolnir = data.weap->formID;
                    _isCharged = true;

                //  AnArchos->GetActorRuntimeData().emotionType = RE::EmotionType::kAnger;
                //  AnArchos->GetActorRuntimeData().emotionValue = 100;

                    auto& enchCost = ench->data.costOverride;
                    auto& enchAmount = ench->data.chargeOverride;
                    auto& mag = enchEffect->effectItem.magnitude;
                    enchAmount = 200.f * (float)a_stage;
                    mag = a_magnitude * ((float)a_stage / 3.f) * (mjolnirDam / 2.f);
                    enchCost = mag;
                    data.enchMag = nullptr;

                //    if (a_stage == 3u) {
                        chargeHitCount = a_chargeHitCount;
                    //    kratos->SetIsCanCharge(AnArchos, false, Kratos::Relic::kMjolnir);
                //    }
                    spdlog::info("mjolnir charge shock damage buff is: {}", mag);
                } else spdlog::error("can't find shock enchantment's base!!");
            } else spdlog::error("can't find shock enchantment!!");
#endif
        }
    } else spdlog::error("can't find mjolnir for charging!!");
}
void Mjolnir::ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck, const bool a_justReset)
{
    auto kratos = Kratos::GetSingleton();
    if (a_justReset) {
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), data.weap) == EnchCharge) {
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), data.weap);
            _isCharged = false;
        }
    } else if (kratos->IsEquipped(Kratos::Relic::kMjolnir)) {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), data.weap) == EnchCharge)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), data.weap);
            }
            kratos->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), true, Kratos::Relic::kMjolnir);
            _isCharged = false;
        } else if (!a_justCheck) {chargeHitCount -= 1;}
    }
}
bool Mjolnir::IsArriving(RE::Projectile* a_proj) const {return a_proj == arrivingMjolnir.proj;}
bool Mjolnir::IsHoming(RE::Projectile* a_proj) const {return a_proj == homingMjolnir.proj;}
void Mjolnir::StartChargingThrow(RE::Actor* a_actor)
{
    if (auto kratos = Kratos::GetSingleton(); a_actor && kratos) {
        ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeMjolnir, WeaponIdentify::RHandBone, 5.f, &kratos->_soundHandle);
        a_actor->ApplyArtObject(kratos->VFXeffect.handShock, 5.f, nullptr, false, false, WeaponIdentify::RHandBone);
        kratos->SetIsChargingThrow(true);
    }
}
#pragma endregion
#ifdef TRIDENT
#pragma region Trident
bool Trident::Initialize()
{
    bool found = true;
    isTridentThrowable = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    TridentProjBaseL        = dataHandler->LookupForm<RE::BGSProjectile>(0x806, Config::TridentModESP);
    TridentCallProjBaseL    = dataHandler->LookupForm<RE::BGSProjectile>(0x803, Config::TridentModESP);
    TrishulsMightProjBaseL  = dataHandler->LookupForm<RE::BGSProjectile>(0x82A, "Supernova.esl");
    SpellTridentProjL       = dataHandler->LookupForm<RE::SpellItem>    (0x807, Config::TridentModESP);
    SpellTridentsCallProjL  = dataHandler->LookupForm<RE::SpellItem>    (0x804, Config::TridentModESP);
    SpellTrishulsMightProjL = dataHandler->LookupForm<RE::SpellItem>    (0x80F, "Supernova.esl");
    data.explosionThrow     = dataHandler->LookupForm<RE::BGSExplosion> (0x801, Config::TridentModESP);
    data.explosionCall      = dataHandler->LookupForm<RE::BGSExplosion> (0x809, Config::TridentModESP);
    if (TridentProjBaseL)
            spdlog::debug("Trident projectile is {}", TridentProjBaseL->GetName());
    else     {spdlog::error("Can't find Trident projectile"); found = false;}
    if (TridentCallProjBaseL)
            spdlog::debug("Trident's call projectile is {}", TridentCallProjBaseL->GetName());
    else     {spdlog::error("Can't find Trident projectile"); found = false;}
    if (SpellTridentProjL)
            spdlog::debug("Trident projectile spell is {}", SpellTridentProjL->GetName());
    else     {spdlog::error("Can't find Trident projectile spell"); found = false;}
    if (SpellTridentsCallProjL)
            spdlog::debug("Trident's call spell is {}", SpellTridentsCallProjL->GetName());
    else     {spdlog::error("Can't find Trident projectile spell"); found = false;}
    if (data.explosionThrow)
            spdlog::debug("Trident throw explosion is {}", data.explosionThrow->GetName());
    else     {spdlog::error("Can't find Trident throw explosion"); found = false;}
    if (data.explosionCall)
            spdlog::debug("Trident call explosion is {}", data.explosionCall->GetName());
    else     {spdlog::error("Can't find Trident call explosion"); found = false;}

    return found;
}
void Trident::Update(const float a_delta)
{
    if (tridentRainStarted) {
        if (AsyncUtil::GameTime::GetEngineTime() >= nextLaunchTime) {
            RainOfTrident();
        }
    }
}
void Trident::Throw(const bool justContinue, RE::Actor* a_actor)
{
    if (SpellTridentProjL && WeaponIdentify::Trident && (WeaponIdentify::isTrident || justContinue))
    {
        const auto TridentDamage = (float)(WeaponIdentify::Trident->attackDamage);
        const float mag = TridentDamage * WeaponIdentify::DamageMult;
        const auto effTrident = SpellTridentProjL->effects[0];
        auto& leviProjEffSetting = effTrident->effectItem;
        leviProjEffSetting.magnitude = mag / 2.f;
        data.damage = mag;
        if (data.explosionThrow) {
            data.explosionThrow->data.damage = data.damage;
        }

        auto origin = justContinue ? data.position : WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = RE::Projectile::ProjectileRot(a_actor->GetAimAngle(), a_actor->GetAimHeading());
        if (justContinue && data.model) a_actor->Unk_A0(data.model, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, SpellTridentProjL);
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
        lData.weaponSource = WeaponIdentify::Trident;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(a_actor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor);

        RE::Projectile::Launch(&pHandle, lData);
        if (justContinue) return;
        isTridentThrowable = false;
#ifdef EXPERIMENTAL_UNEQUIP_AFTER_THROW
        WeaponIdentify::skipEquipAnim = true;
        WeaponIdentify::unequipWhenAnimEnds = true;
#endif
        WeaponIdentify::EquippedObjR = nullptr;
        WeaponIdentify::lastThrownRelic = Kratos::Relic::kTrident;
            spdlog::info("Trident throwed, raw damage is: {}", mag);
        //  reset the last throw's traces
        data.lastHitActors.clear();
        data.lastHitForms.clear();
    } else  spdlog::info("Trident is not equipped for throwing");
}
void Trident::TrishulsMight(const float a_delay, const float a_duration)
{
    if (SpellTrishulsMightProjL) {
        nextLaunchDelay = a_delay;
        nextLaunchTime = AsyncUtil::GameTime::GetEngineTime();
        lastLaunchTime = AsyncUtil::GameTime::GetEngineTime() + a_duration;
        tridentRainStarted = true;
        spdlog::info("rain of trident triggered");

        isTridentThrowable = false;
#ifdef EXPERIMENTAL_UNEQUIP_AFTER_THROW
        WeaponIdentify::skipEquipAnim = true;
        WeaponIdentify::unequipWhenAnimEnds = true;
#endif
        WeaponIdentify::EquippedObjR = nullptr;
        WeaponIdentify::lastThrownRelic = Kratos::Relic::kTrident;
        data.lastHitActors.clear();
        data.lastHitForms.clear();
    }
}
void Trident::RainOfTrident()
{
    if (nextLaunchTime > lastLaunchTime) {
        tridentRainStarted = false;
        spdlog::info("rain of trident done");
            return;
    }
    //  launch trident's might projectile at a random top position of the nearest target
    const auto AnArchos = PlayerCharacter::GetSingleton();
    std::vector<RE::Actor*> nearCombatTargets;
    for (uint16_t distance = 400; distance < 6666 && nearCombatTargets.empty(); distance*=2)
    {
        nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(AnArchos, distance);
    }

    const bool isThereNearTarget = (!nearCombatTargets.empty() && nearCombatTargets.front()) ? true : false;
    RE::NiPoint3 origin = isThereNearTarget ? nearCombatTargets.front()->GetPosition() : (AnArchos->GetPosition() + MathUtil::Algebra::GetForwardVector(AnArchos) * 360.f);
    origin = origin + RE::NiPoint3(MathUtil::Algebra::GenerateRandomFloat(-150.f, 150.f), MathUtil::Algebra::GenerateRandomFloat(-150.f, 150.f), 3000.f);
    RE::ProjectileHandle pHandle;
    RE::Projectile::ProjectileRot pRot{};
    pRot.x = PI2 + MathUtil::Algebra::GenerateRandomFloat(-0.02f, 0.02f);
//    spdlog::debug("aim angle: {} heading angle: {}", pRot.x, pRot.z);
    RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellTrishulsMightProjL);
    lData.weaponSource = WeaponIdentify::DraupnirSpear;
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

    RE::Projectile::Launch(&pHandle, lData);

    nextLaunchTime = AsyncUtil::GameTime::GetEngineTime() + nextLaunchDelay;
}
void Trident::Call(const float a_damage, const float a_force, RE::Actor* a_actor, const bool a_justEquip)
{
    if (WeaponIdentify::Trident && !WeaponIdentify::isTrident) {
        ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::Trident, true, 1U, true, false, false, false, nullptr, a_justEquip);
        ResetEquipAnimationAfter(100, a_actor);
        if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
    }

    auto kratos = Kratos::GetSingleton();
    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, a_actor);

    if (!a_justEquip) {
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "weaponDraw");           //  loads the 3d model of the weapon
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "WeapEquip_Out");
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "WeapEquip_OutMoving");
        if (auto handEffect = kratos->VFXeffect.handFlame; handEffect) 
            a_actor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);

        if (auto catchSound = kratos->soundEffect.catchMjolnir; catchSound) 
            ObjectUtil::Sound::PlaySound(catchSound, WeaponIdentify::RHandBone, 2.f);

        if (data.explosionCall) {
            data.explosionCall->data.damage = data.damage;
            RE::ProjectileHandle pHandle;
            TriggerExplosion(data.explosionCall->data.force, &pHandle);
        }

        data.lastHitActors.clear();
        data.lastHitForms.clear();

        if (data.proj) {
            data.position = data.proj->data.location;
            auto& runtimeData = data.proj->GetProjectileRuntimeData();
            auto& pFlags = runtimeData.flags;
            if (!(pFlags & (1 << 25))) {
                pFlags |= (1 << 25);
            } else  spdlog::debug("Trident is already destroyed");
        } else {spdlog::debug("thrown Trident is nullptr!");}
    }
}
inline void Trident::TriggerExplosion(float a_force, RE::ProjectileHandle* a_pHandle)
{
    if (!data.lastHitActors.empty()) {
        if (data.lastHitBone && data.lastHitActors.front() && a_pHandle) {
            TriggerExplosionAtLocation(data.lastHitBone, a_pHandle, data.lastHitActors.front());
            spdlog::info("Trident exploded from {} of {}", data.lastHitBone->name, data.lastHitActors.front()->GetName());
        }
        data.lastHitBone = nullptr;
    } else if (a_pHandle && data.proj) {
        TriggerExplosionAtLocation(data.proj, a_pHandle);
        spdlog::info("Trident explosion done");
    } else {
        spdlog::info("all Trident explosions done");
    }
}
inline void Trident::TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle)
{
    RE::Projectile::LaunchData lData(RE::PlayerCharacter::GetSingleton(), a_proj->data.location, RE::Projectile::ProjectileRot(), SpellTridentsCallProjL);
    RE::Projectile::Launch(a_pHandle, lData);
}
inline void Trident::TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target)
{
    RE::Projectile::LaunchData lData(RE::PlayerCharacter::GetSingleton(), a_bone->world.translate, RE::Projectile::ProjectileRot(), SpellTridentsCallProjL);
    lData.desiredTarget = a_target;
    RE::Projectile::Launch(a_pHandle, lData);
}
#pragma endregion
#endif
//EVENTS
bool AnimationEventTracker::Register()
{
    const auto pc = PlayerCharacter::GetSingleton();

    bool bSinked = false;
    bool bSuccess = pc->AddAnimationGraphEventSink(AnimationEventTracker::GetSingleton());
    if (bSuccess) {
        spdlog::info("Registered {}", typeid(BSAnimationGraphEvent).name());
    } else {
        BSAnimationGraphManagerPtr graphManager;
        pc->GetAnimationGraphManager(graphManager);
        if (graphManager) {         
            for (auto& animationGraph : graphManager->graphs) {
                if (bSinked) {
                    break;
                }
                auto eventSource = animationGraph->GetEventSource<BSAnimationGraphEvent>();
                for (auto& sink : eventSource->sinks) {
                    if (sink == AnimationEventTracker::GetSingleton()) {
                        bSinked = true;
                        break;
                    }
                }
            }
        }

        if (!bSinked) {
            spdlog::info("Failed to register {}", typeid(BSAnimationGraphEvent).name());
        }        
    }
    return bSuccess || bSinked;
}
EventChecker AnimationEventTracker::ProcessEvent(const BSAnimationGraphEvent* a_event, BSTEventSource<BSAnimationGraphEvent>* a_eventSource)
{
    if (a_event) {
        std::string eventTag = a_event->tag.data();
        switch (hash(eventTag.data(), eventTag.size())) {
        // Start phase
        case "SkipNextEquipAnimation"_h:
            WeaponIdentify::skipEquipAnim = true;
            break;
//        case "BeginWeaponDraw"_h:
//            if (WeaponIdentify::isLeviathanAxe && Kratos::GetSingleton()->IsCanCallAxe()) {
//                if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() < 5U) LeviathanAxe::GetSingleton()->Call();
//            }
//            else if (WeaponIdentify::isMjolnir && Kratos::GetSingleton()->IsCanCallMjolnir()) {
//                if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() < 5U) Mjolnir::GetSingleton()->Call();
//            }
//            break;
        case "CallWeapon"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos) {
                switch (kratos->GetNextWeaponToCall())
                {
                case Kratos::Relic::kLeviathanAxe:
                    LeviathanAxe::GetSingleton()->Call();
                    break;
                case Kratos::Relic::kMjolnir:
                    Mjolnir::GetSingleton()->Call(false, false, Config::MjolnirArrivingDelay);
                    break;
#ifdef TRIDENT
                case Kratos::Relic::kTrident:
                    Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
                    break;
#endif
                default:
                    spdlog::warn("Can't found any weapon for ready to calling! Trying to call Levi");
                    LeviathanAxe::GetSingleton()->Call();
                    break;
                }
            }
/*
            if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kLeviathanAxe) {
                if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) Mjolnir::GetSingleton()->Call();
#ifdef TRIDENT
                else if ((uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
#endif
                else LeviathanAxe::GetSingleton()->Call();
            }
            else if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kMjolnir) {
                if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) LeviathanAxe::GetSingleton()->Call();
#ifdef TRIDENT
                else if ((uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() == 1U && WeaponIdentify::Trident && !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
#endif
                else Mjolnir::GetSingleton()->Call();
            }
#ifdef TRIDENT
            else if (WeaponIdentify::lastThrownRelic == Kratos::Relic::kTrident) {
                if (Trident::GetSingleton()->isTridentThrowable && WeaponIdentify::LeviathanAxe && (uint_fast8_t)LeviathanAxe::GetSingleton()->GetThrowState() > 1U) LeviathanAxe::GetSingleton()->Call();
                else if (Trident::GetSingleton()->isTridentThrowable && WeaponIdentify::Mjolnir && (uint_fast8_t)Mjolnir::GetSingleton()->GetThrowState() > 1U) Mjolnir::GetSingleton()->Call();
                else Trident::GetSingleton()->Call(10.f, 100.f, RE::PlayerCharacter::GetSingleton(), true);
            }
#endif
            else {spdlog::warn("Can't found any weapon for ready to calling! Trying to call Levi"); LeviathanAxe::GetSingleton()->Call();}
*/            break;
        case "CatchLevi"_h:
            break;
        case "LeviCallAttack"_h:     //event: attackPowerStartInPlace, attackStart, PowerAttack [IDLE:000E8456], NormalAttack [IDLE:00013215]
            if (auto Levi = LeviathanAxe::GetSingleton(); !WeaponIdentify::isLeviathanAxe && WeaponIdentify::LeviathanAxe) {
                Levi->Call(true);
                auto AnArchos = PlayerCharacter::GetSingleton();
                ObjectUtil::Actor::EquipItem(AnArchos, WeaponIdentify::LeviathanAxe, true, 1u, true, false, false, false);
                ResetEquipAnimationAfter(100, AnArchos);
            } else spdlog::info("Levi is not callable");
        case "ThrowAttackStart"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
                }
            }
            else if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton()) {
                    mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag);
                }
            }
        //    if (WeaponIdentify::isLeviathanAxe)
        //        Kratos::GetSingleton()->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false);
        //    if (WeaponIdentify::isMjolnir)
        //        Kratos::GetSingleton()->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false, Kratos::Relic::kMjolnir);
            break;
        case "ThrowWeapon"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kThrowable) {
                    Levi->Throw(false);
                } else spdlog::warn("Levi is not throwable");
            }
            if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton(); mjolnir->GetThrowState() == tStateM::kThrowable) {
                    mjolnir->Throw(false);
                } else spdlog::warn("Mjolnir is not throwable");
            }
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Throw();
            break;
        case "ThrowWeaponV"_h:
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi && Levi->GetThrowState() == tState::kThrowable) {
                Levi->Throw(true);
            }
            else spdlog::warn("Levi is not throwable");
            break;
        case "ThrowWeaponH"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kThrowable) {
                    Levi->Throw(false, false, true);
                } else spdlog::warn("Levi is not throwable");
            }
            if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton(); mjolnir->GetThrowState() == tStateM::kThrowable) {
                    mjolnir->Throw(false, false, true);
                } else spdlog::warn("Mjolnir is not throwable");
            }
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Throw();
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->Throw();
#endif
            break;
        case "LeviChargeStart"_h:
            if (auto kratos = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (kratos && AnArchos) {
                    if (auto handEffect = kratos->VFXeffect.handFrost; handEffect)
                        AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                    if (auto soundEffect = kratos->soundEffect.chargeLevi; soundEffect)
                        ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
                }
            }
            break;
        case "LeviChargeEnd"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton()))
                if (auto levi = LeviathanAxe::GetSingleton())
                    levi->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, -1);
            break;
        case "MjolnirChargeStart"_h:
            if (auto kratos = Kratos::GetSingleton(); auto AnArchos = RE::PlayerCharacter::GetSingleton()) {
                if (kratos && AnArchos) {
                    if (auto handEffect = kratos->VFXeffect.handShock; handEffect)
                        AnArchos->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone, false);
                    if (auto soundEffect = kratos->soundEffect.chargeLevi; soundEffect)
                        ObjectUtil::Sound::PlaySound(soundEffect, WeaponIdentify::RHandBone, 5.f);
                }
            }
            break;
        case "MjolnirCharge1"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 1u, -1);
            break;
        case "MjolnirCharge2"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 2u, -1);
            break;
        case "MjolnirCharge3"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
                if (auto mjolnir = Mjolnir::GetSingleton())
                    mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
            break;
    //    case "MjolnirChargeEnd"_h:
    //        if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsCanCharge(PlayerCharacter::GetSingleton(), Kratos::Relic::kMjolnir))
    //            if (auto mjolnir = Mjolnir::GetSingleton())
    //                mjolnir->Charge(Config::ChargeHitCount, Config::ChargeMagnitude, 3u, -1);
    //        break;
        case "ThrowSpear"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Throw();
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->Throw();
#endif
            break;
        case "DraupnirsCallStage1"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::SetExplosionMagnitude(1.2f);
            break;
        case "DraupnirsCallStage2"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::SetExplosionMagnitude(1.5f);
            break;
        case "DraupnirsCall"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::Call(10.f, 100.f);
#ifdef TRIDENT
            else if (WeaponIdentify::isTrident || !Trident::GetSingleton()->isTridentThrowable) Trident::GetSingleton()->Call(10, 100);
#endif
            break;
        //  rage
        case "RageFuryTriggerStart"_h:
            Kratos::GetSingleton()->StartRage(Kratos::Rage::kFury);
            break;
        case "RageFuryTriggerEnd"_h:
    //        if (auto kratos = Kratos::GetSingleton(); kratos->IsInRage())
    //            kratos->SetIsCanRage(false);
            break;
        case "RageValorStart"_h:
            Kratos::GetSingleton()->StartRage(Kratos::Rage::kValor);
            break;
        case "RageValorEnd"_h:
            Kratos::GetSingleton()->EndRage(Kratos::Rage::kValor, true);
            break;
        case "RageFinish"_h:
            Kratos::GetSingleton()->EndRage(Kratos::GetSingleton()->GetLastTriggeredRageType(), true, false);
    //        Kratos::GetSingleton()->SetIsCanRage();
            break;
        case "weaponDraw"_h:
            WeaponIdentify::WeaponCheck();
            break;
        case "weaponSwing"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsInRage())
                kratos->RestoreRage(RE::PlayerCharacter::GetSingleton(), kratos->CalcRageDamageOrBuffAmount(360.f));
            break;
    //    case "CastOKStart"_h:
        case "MCO_AttackInitiate"_h:
        case "MCO_PowerAttackInitiate"_h:
        case "MCO_SprintAttackInitiate"_h:
        case "MCO_SprintPowerAttackInitiate"_h:
        case "Bfco_AttackStartFX"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
                }
            }
            else if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton()) {
                    mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag);
                }
            }
            break;
        case "AttackWinStart"_h:
        case "MCO_WinOpen"_h:
        case "MCO_PowerWinOpen"_h:
        case "BFCO_NextWinStart"_h:
        case "BFCO_NextPowerWinStart"_h:
        case "Collision_AttackEnd"_h:
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                }
            }
            else if (WeaponIdentify::isMjolnir) {
                if (auto mjolnir = Mjolnir::GetSingleton()) {
                    mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, true);
                }
            }
            break;
        case "InsertDraupnir"_h:
            Draupnir::MeleeThrow();
            break;
        case "RainOfSpear"_h:
            if (WeaponIdentify::isDraupnirSpear) Draupnir::ArtilleryOfTheAncients(0.1f, 3.f);
#ifdef TRIDENT
            else if(WeaponIdentify::isTrident) Trident::GetSingleton()->TrishulsMight(1.f, 6.f);
#endif
            break;
        case "FlameWhiplashStart"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
            //    if (!BoC->IsScorching()) RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, 0.8f);
                BoC->SetIsScorching();
                BoC->SetScorchingSpeed(0.5f);
            }
            break;
        case "FlameWhiplashLoop"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
                if (BoC->IsQueueEnd()) {
                    RE::PlayerCharacter::GetSingleton()->NotifyAnimationGraph("chainClose");
                    RE::PlayerCharacter::GetSingleton()->NotifyAnimationGraph("IdleStop");
                }
            }
            break;
        case "FlameWhiplashEnd"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
            //    if (!BoC->IsScorching()) RE::PlayerCharacter::GetSingleton()->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kSpeedMult, -0.8f);
                BoC->SetIsScorching(false);
            }
            break;
    //    case "BFCO_DIY_recovery"_h:
    //    case "MCO_Recovery"_h:
        case "MCO_AttackStateExit"_h:
        case "tailCombatState"_h:
        case "tailCombatIdle"_h:
        case "attackStop"_h:
        case "IdleStop"_h:
        case "CastOKStop"_h:
            if (WeaponIdentify::unequipWhenAnimEnds) {
                if (auto AnArchos = PlayerCharacter::GetSingleton(); AnArchos) {
                    ObjectUtil::Actor::UnEquipItem(AnArchos, false, false, true, true, WeaponIdentify::skipEquipAnim, false);
                    ResetEquipAnimationAfter(100, AnArchos);
                } WeaponIdentify::unequipWhenAnimEnds = false;
            }
#ifdef EXPERIMENTAL_SHIELD
            //  animated shield
            ObjectUtil::Actor::SendAnimationEvent(PlayerCharacter::GetSingleton(), "shieldClose");
#endif
            break;
        case "throwAttackReady"_h:
        case "throwPowerAttackReady"_h:
            if (Config::IsAdvancedThrowingInstalled) {
                if (auto AnArchos = PlayerCharacter::GetSingleton(); AnArchos && WeaponIdentify::isRelic) {
                    bool isThrowing; AnArchos->GetGraphVariableBool("bIsThrowing", isThrowing);
                    if (isThrowing) {
                        bool isChargingThrow; AnArchos->GetGraphVariableBool("bIsPressingAttackButton", isChargingThrow);
                        if (!isChargingThrow) AnArchos->GetGraphVariableBool("bIsPressingPowerAttackButton", isChargingThrow);
                        if (auto kratos = Kratos::GetSingleton(); kratos) {
                            if (auto Levi = LeviathanAxe::GetSingleton(); WeaponIdentify::isLeviathanAxe && Levi->GetThrowState() == tState::kThrowable) {
                                Levi->data.throwingChargeDuration = 0.f;
                                if (isChargingThrow) Levi->StartChargingThrow(AnArchos);
                            } else if (auto mjolnir = Mjolnir::GetSingleton(); WeaponIdentify::isMjolnir && mjolnir->GetThrowState() == tStateM::kThrowable) {
                                mjolnir->data.throwingChargeDuration = 0.f;
                                if (isChargingThrow) mjolnir->StartChargingThrow(AnArchos);
                            } else if (WeaponIdentify::isDraupnirSpear) {
                                Draupnir::data.throwingChargeDuration = 0.f;
                                if (isChargingThrow) Draupnir::StartChargingThrow(AnArchos);
#ifdef TRIDENT
                            } else if (WeaponIdentify::isTrident) {
                                if (isChargingThrow) Trident::StartChargingThrow(AnArchos);
#endif
                            }
                        }
                    }
                }
            }
            break;
        case "throwAttackEndStart"_h:
        case "throwPowerAttackEndStart"_h:
            if (Config::IsAdvancedThrowingInstalled) {
                if (auto kratos = Kratos::GetSingleton(); kratos) kratos->SetIsChargingThrow(false);
            }
            break;
        }
    }
        return EventChecker::kContinue;
}

bool MagicEffectApplyTracker::Register()
{
    auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(MagicEffectApplyTracker::GetSingleton());
            spdlog::info("Magic effect apply event sink registered!");
            return true;
    } else  spdlog::warn("Magic effect apply event sink not registered!");
    return false;
}
EventChecker MagicEffectApplyTracker::ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource)
{
    if (a_event) {
        auto formID = a_event->magicEffect;
        auto casterRef = a_event->caster.get();
        auto targetRef = a_event->target.get();
        if (casterRef && targetRef && casterRef == targetRef) {
            auto caster = casterRef->As<RE::Actor>();
            auto casterMT = caster ? caster->AsMagicTarget() : nullptr;
            auto kratos = Kratos::GetSingleton();
            auto levi = LeviathanAxe::GetSingleton();
            auto mjolnir = Mjolnir::GetSingleton();
            if (!casterMT || !kratos || !levi || !mjolnir) {spdlog::warn("caster magic target or kratos or levi or mjolnir does not exists!"); return EventChecker::kContinue;}

            if (formID == kratos->spellID.aim) {
                spdlog::debug("aiming...");
            } else if (formID == kratos->spellID.call) {
                if (!WeaponIdentify::isRelic) {
                    if (levi->data.weap || mjolnir->data.weap || WeaponIdentify::Trident) {
                        if ((uint_fast8_t)levi->GetThrowState() <= 3U && (uint_fast8_t)levi->GetThrowState() != 0U) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)kratos->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                        } else if ((uint_fast8_t)mjolnir->GetThrowState() <= 3U && (uint_fast8_t)mjolnir->GetThrowState() != 0U) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)kratos->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
#ifdef TRIDENT
                        } else if (!Trident::GetSingleton()->isTridentThrowable) {
                            caster->SetGraphVariableInt("iNextWeaponToCall", (uint32_t)kratos->GetNextWeaponToCall());
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
#endif
                        } else spdlog::info("levi and mjolnir can't arrive!");
                    } else spdlog::info("levi and mjolnir does not exist!");
                } else if (WeaponIdentify::isLeviathanAxe) {
                    if (!levi->isAxeThrowed && kratos->IsCanCharge(caster, Kratos::Relic::kLeviathanAxe)) {
                        kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                    }
                } else if (WeaponIdentify::isBladeOfChaos) {
                    BladeOfChaos::GetSingleton()->Update(*g_engineTime);
                    BladeOfChaos::GetSingleton()->BuffScorchingSpeed();
                    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                } else if (WeaponIdentify::isDraupnirSpear || WeaponIdentify::isTrident) {
                    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                } else if (WeaponIdentify::isMjolnir) {
                    if (!mjolnir->isMjolnirThrowed && kratos->IsCanCharge(caster, Kratos::Relic::kMjolnir)) {
                        kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                    }
                }
            } else if (formID == kratos->spellID.runic) {
                if (caster->HasSpell(kratos->SpellFinisherButton)) {
                    kratos->DoKratosAction(Kratos::Action::kRage, caster);
                }
            } else if (formID == kratos->spellID.finisher) {
                if (caster->HasSpell(kratos->SpellRunicButton)) {
                    kratos->DoKratosAction(Kratos::Action::kRage, caster);
                }
            } else if (formID == kratos->spellID.leviChargeCoolDown) {
                spdlog::debug("levi charge in cooldown...");
            }
        }
    }   return EventChecker::kContinue;
}

bool InputEventTracker::Register()
{
    auto sourceHolder = RE::BSInputDeviceManager::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(InputEventTracker::GetSingleton());
            spdlog::info("input event sink registered!");
            return true;
    } else  spdlog::warn("input event sink not registered!");
    return false;
};
std::uint32_t InputEventTracker::GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key)
{
    using Key = RE::BSWin32GamepadDevice::Key;

    std::uint32_t index;
    switch (a_key) 
    {
    case Key::kUp:
        index = 0;
        break;
    case Key::kDown:
        index = 1;
        break;
    case Key::kLeft:
        index = 2;
        break;
    case Key::kRight:
        index = 3;
        break;
    case Key::kStart:
        index = 4;
        break;
    case Key::kBack:
        index = 5;
        break;
    case Key::kLeftThumb:
        index = 6;
        break;
    case Key::kRightThumb:
        index = 7;
        break;
    case Key::kLeftShoulder:
        index = 8;
        break;
    case Key::kRightShoulder:
        index = 9;
        break;
    case Key::kA:
        index = 10;
        break;
    case Key::kB:
        index = 11;
        break;
    case Key::kX:
        index = 12;
        break;
    case Key::kY:
        index = 13;
        break;
    case Key::kLeftTrigger:
        index = 14;
        break;
    case Key::kRightTrigger:
        index = 15;
        break;
    default:
        index = kInvalid;
        break;
    } return index != kInvalid ? index + kGamepadOffset : kInvalid;
}
std::uint32_t InputEventTracker::GetOffsettedKeyCode(std::uint32_t a_keyCode, RE::INPUT_DEVICE a_inputDevice) const
{
    switch (a_inputDevice) {
    case RE::INPUT_DEVICE::kKeyboard:
        break;
    case RE::INPUT_DEVICE::kMouse:
        a_keyCode += kMouseOffset;
        break;
    case RE::INPUT_DEVICE::kGamepad:
        a_keyCode = GetGamepadIndex((RE::BSWin32GamepadDevice::Key)a_keyCode);
        break;
    default:
        break;
    } return a_keyCode;
}
EventChecker InputEventTracker::ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource)
{
    if (!a_event || RE::UI::GetSingleton()->GameIsPaused()) return EventChecker::kContinue;

    for (auto event = *a_event; event; event = event->next) {
        if (!event->HasIDCode() || event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;

        auto keyCode = event->AsIDEvent()->GetIDCode();

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return EventChecker::kContinue;

        auto kratos = Kratos::GetSingleton();
        if (!kratos) return EventChecker::kContinue;

        if (auto button = static_cast<RE::ButtonEvent*>(event); button) {
            auto device = event->device.get();
            keyCode = GetOffsettedKeyCode(keyCode, device);
            if (keyCode == Config::AxeCallKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellAxeCallButton); player->SetGraphVariableBool("bPressingCallButton", true);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellAxeCallButton); player->SetGraphVariableBool("bPressingCallButton", false);}
            }
            else if (keyCode == Config::AimKey) {
                if (button->IsDown()) {kratos->Aim(true); player->AddSpell(kratos->SpellAimButton);/* player->SetGraphVariableBool("bIsAiming", true);*/}
                else if (button->IsUp()) {kratos->Aim(false); player->RemoveSpell(kratos->SpellAimButton);/* player->SetGraphVariableBool("bIsAiming", false);*/}
            }
            else if (keyCode == Config::RunicKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellRunicButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellRunicButton);}
            }
            else if (keyCode == Config::FinisherKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellFinisherButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellFinisherButton);}
            }
            else if (keyCode == Config::MediumDistanceKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellMidDistButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellMidDistButton);}
            }
            else if (keyCode == Config::LongDistanceKey) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellLongDistButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellLongDistButton);}
            }
        }
    } return EventChecker::kContinue;
}

bool HitEventTracker::Register()
{
    auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton(); 
    if (sourceHolder) {
        sourceHolder->AddEventSink(HitEventTracker::GetSingleton());
            spdlog::info("hit event sink registered!");
            return true;
    } else  spdlog::warn("hit event sink not registered!");
    return false;
};
EventChecker HitEventTracker::ProcessEvent(const RE::TESHitEvent*   a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource)
{
    if (a_event && a_eventSource && a_event->target && a_event->target->IsPlayerRef()) {

    }
        return EventChecker::kContinue;
}