#include "MainKratosCombat.h"
//#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;

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
            if (entry && entry->object && entry->object->GetFile()) {
                if (entry->object->IsWeapon()) {
#ifdef NEW_WEAPON_REGISTER_METHOD
#else
                    if (kratos->gLeviathanAxeFormID && entry->object->GetLocalFormID() == kratos->gLeviathanAxeFormID->value && entry->object->GetFile()->fileName == Config::registeredLeviathanFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::LeviathanAxeKWD);
                            LeviathanAxe = weap;
                            spdlog::info("{} is your Leviathan Axe", weap->GetName());
                        }
                    } else if (kratos->gBladeOfChaosFormID && entry->object->GetLocalFormID() == kratos->gBladeOfChaosFormID->value && entry->object->GetFile()->fileName == Config::registeredBladeOfChaosFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            BladeOfChaos = weap;
                            spdlog::info("{} is your Blade of Chaos", weap->GetName());
                        }
                    } else if (kratos->gBladeOfChaosFormID && entry->object->GetLocalFormID() == kratos->gBladeOfChaosFormID->value+1 && entry->object->GetFile()->fileName == Config::registeredLBladeOfChaosFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            BladeOfChaosL = weap;
                            spdlog::info("{} is your left Blade of Chaos", weap->GetName());
                        }
                    } else if (kratos->gDraupnirSpearFormID && entry->object->GetLocalFormID() == kratos->gDraupnirSpearFormID->value && entry->object->GetFile()->fileName == Config::registeredDraupnirFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::DraupnirSpearKWD);
                            DraupnirSpear = weap;
                            spdlog::info("{} is your Draupnir Spear", weap->GetName());
                        }
                    } else if (kratos->gBladeOfOlympusFormID && entry->object->GetLocalFormID() == kratos->gBladeOfOlympusFormID->value && entry->object->GetFile()->fileName == Config::registeredBladeOfOlympusFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfOlympusKWD);
                            BladeOfOlympus = weap;
                            spdlog::info("{} is your Blade of Olympus", weap->GetName());
                        }
                    } else if (kratos->gMjolnirFormID && entry->object->GetLocalFormID() == kratos->gMjolnirFormID->value && entry->object->GetFile()->fileName == Config::registeredMjolnirFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::MjolnirKWD);
                            Mjolnir = weap;
                            spdlog::info("{} is your Mjolnir", weap->GetName());
                        }
                    }
#ifdef TRIDENT
                    else if (kratos->gTridentFormID && entry->object->GetLocalFormID() == kratos->gTridentFormID->value && entry->object->GetFile()->fileName == Config::registeredTridentFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::TridentKWD);
                            Trident = weap;
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
                            GuardianShield = shield;
                            spdlog::info("{} is your Guardian Shield", shield->GetName());
                        }
                    }
#endif
                }
            } else spdlog::info("entry is not an ordinary object");
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
    if (a_RHandWeapon && a_RHandWeapon->GetFile()) {
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
            if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {
                spdlog::debug("Levi is throwable");
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
            if (a_LHandWeapon && (a_LHandWeapon->HasKeyword(Config::BladeOfChaosKWD) || a_LHandWeapon->GetFile() == a_RHandWeapon->GetFile())) {
                Config::registeredLBladeOfChaosFileName = a_LHandWeapon->GetFile()->fileName;
                Config::registeredLBladeOfChaosID = a_LHandWeapon->GetLocalFormID();
                BladeOfChaosL = a_LHandWeapon;
            }
            Config::registeredBladeOfChaosID = equippedWeaponID;
            kratos->gBladeOfChaosFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kBladeOfChaos;

            auto BoC = BladeOfChaos::GetSingleton();
            BoC->data.weap  = BladeOfChaos;
            BoC->data.weaponModel   = WeaponBone;
            BoC->data.weaponModelL  = ShieldBone;
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

            Draupnir::data.weap     = DraupnirSpear;
            Draupnir::data.ench     = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            Draupnir::data.poison   = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            Draupnir::data.damage   = static_cast<float>(DraupnirSpear->attackDamage);
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
        //    mjolnir->MjolnirProjBaseT->model = Mjolnir->model;
        //    mjolnir->MjolnirProjBaseA->model = Mjolnir->model;
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
    } if (a_shield && a_shield->GetFile()) {
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

    RHandBone = GetRhandBone(AnArchos);
    LHandBone = GetLhandBone(AnArchos);
    WeaponBone = GetWeaponBone(AnArchos);
    ShieldBone = GetShieldBone(AnArchos);
    AnimObjectRBone = GetAnimObjectRBone(AnArchos);

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
RE::NiAVObject* WeaponIdentify::GetRhandBone(RE::Actor* a_actor)
{
    if      (RHandBone = ObjectUtil::Actor::GetBoneByName(a_actor, "NPC R Finger20 [RF20]"); RHandBone)  {}
    else if (RHandBone = ObjectUtil::Actor::GetBoneByName(a_actor, "NPC R MagicNode [RMag]"); RHandBone) {}
    else if (RHandBone = ObjectUtil::Actor::GetBoneByName(a_actor, "NPC R Hand [RHnd]"); RHandBone)      {}
    else    spdlog::error("can't found right hand bone!");
    return RHandBone;
}
RE::NiAVObject* WeaponIdentify::GetLhandBone(RE::Actor* a_actor)
{
    if      (LHandBone = ObjectUtil::Actor::GetBoneByName(a_actor, "NPC L Finger20 [LF20]"); LHandBone)  {}
    else if (LHandBone = ObjectUtil::Actor::GetBoneByName(a_actor, "NPC L MagicNode [LMag]"); LHandBone) {}
    else if (LHandBone = ObjectUtil::Actor::GetBoneByName(a_actor, "NPC L Hand [LHnd]"); LHandBone)      {}
    else    spdlog::error("can't found right hand bone!");
    return LHandBone;
}
RE::NiAVObject* WeaponIdentify::GetWeaponBone(RE::Actor* a_actor)
{
    WeaponBone =ObjectUtil::Actor::GetBoneByName(a_actor, "Weapon");
    return WeaponBone;
}
RE::NiAVObject* WeaponIdentify::GetShieldBone(RE::Actor* a_actor)
{
    ShieldBone =ObjectUtil::Actor::GetBoneByName(a_actor, "Shield");
    return ShieldBone;
}
RE::NiAVObject* WeaponIdentify::GetAnimObjectRBone(RE::Actor* a_actor)
{
    AnimObjectRBone =ObjectUtil::Actor::GetBoneByName(a_actor, "AnimObjectR");
    return AnimObjectRBone;
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
#pragma region KRATOS
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
    soundEffect.throwLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85E, Config::KratosCombatESP);
    soundEffect.callLevi    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x86B, Config::KratosCombatESP);
    soundEffect.arrivingLeviStart   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85A, Config::KratosCombatESP);
    soundEffect.arrivingLeviLoop0   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85B, Config::KratosCombatESP);
    soundEffect.arrivingLeviLoop1   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85C, Config::KratosCombatESP);
    soundEffect.arrivingLeviLoop2   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85D, Config::KratosCombatESP);
    soundEffect.arrivingLeviNear    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x86C, Config::KratosCombatESP);
    soundEffect.catchLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x84E, Config::KratosCombatESP);
    soundEffect.throwingLeviLoop0   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x863, Config::KratosCombatESP);
    soundEffect.throwingLeviLoop1   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x864, Config::KratosCombatESP);
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
        if (auto shieldModel = a_actor->GetEquippedObject(true)->As<RE::TESObjectREFR>()->Get3D1(false); shieldModel) {
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
            if (VFXeffect.fury) a_actor->ApplyArtObject(VFXeffect.fury, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
            if (WeaponIdentify::EquippedObjR)
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
            if (WeaponIdentify::EquippedObjL && WeaponIdentify::EquippedObjL != WeaponIdentify::GuardianShield)
                ObjectUtil::Actor::UnEquipItem(a_actor, true, false, false, false, true, true);
            if (WeaponIdentify::GuardianShield && !WeaponIdentify::isGuardianShield)
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::GuardianShield, true);

            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            break;
        case Kratos::Rage::kValor:
            if (a_justAnim) {
                _gettingHittedInValor = false;
                a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kRage);
                a_actor->NotifyAnimationGraph("DoKratosAction");
                RestoreRage(a_actor, -(*values.rageDamageAmount * 5.f), true);
            }
            if (VFXeffect.valor) a_actor->ApplyArtObject(VFXeffect.valor, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
            break;
        case Kratos::Rage::kWrath:
            if (VFXeffect.wrath) a_actor->ApplyArtObject(VFXeffect.wrath, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
            return;
        case Kratos::Rage::kLegacy:
            if (WeaponIdentify::BladeOfOlympus) {
            //    if (auto mCaster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant); mCaster && SpellSpartanRage) {
            //        mCaster->CastSpellImmediate(SpellSpartanRage, false, a_actor, 1.f, false, 2.f, a_actor);
            //    }
                if (VFXeffect.legacy) a_actor->ApplyArtObject(VFXeffect.legacy, 1.f, nullptr, false, false, WeaponIdentify::GetRhandBone(a_actor));
                ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::BladeOfOlympus);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
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
                ObjectUtil::Actor::ResetEquipAnimationAfter(0, a_actor);
                RestoreRage(a_actor, -(*values.rageDamageAmount * 3.f), true);
                if (_LastEquippedObjectR)
                    ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectR, true);
                if (_LastEquippedObjectL)
                    ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectL, true);
                ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
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
            ObjectUtil::Actor::ResetEquipAnimationAfter(0, a_actor);
            if (_LastEquippedObjectR)
                ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectR, true);
            else if (WeaponIdentify::EquippedObjR) 
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
            if (_LastEquippedObjectL)
                ObjectUtil::Actor::EquipItem(a_actor, _LastEquippedObjectL, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
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
#pragma region LEVIATHAN_AXE
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
        if (data.model && data.proj && data.proj->Get3D() && data.weaponModelCopy && data.model.get() == data.proj->Get3D()) {
            const RE::BSFixedString rotatingBoneName = "Cylinder02";
            auto animatedBone = data.model->GetObjectByName(rotatingBoneName);
            auto animatedNode = animatedBone ? animatedBone->AsNode() : nullptr;

            auto cloneModel = data.weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            data.replacedProjectileModel.reset(cloneNode);

            if (animatedNode) {
                animatedNode->AttachChild(data.replacedProjectileModel.get(), false);
            //    auto oldWorld = data.transformW;
            //    oldWorld.translate *= 70.f;
            //    oldWorld.scale = data.replacedProjectileModel.get()->world.scale;
            //    data.replacedProjectileModel.get()->local = ObjectUtil::Node::GetLocalTransform(data.replacedProjectileModel.get(), oldWorld);
                projectileUpdate.Done();
                trailData.projTrail.reset();
                trailUpdate.RegisterForUpdate(a_delta * 2.f, false);
                spdlog::debug("levi model changed!");
            } else spdlog::warn("animated node or levinode null");
        } else spdlog::warn("proj or proj->Get3D2() null");
    }
    if (soundData.arrivingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopArrivingLoopSounds();}
    if (soundData.throwingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopThrowingLoopSounds();}
    if (Config::DrawTrails) {
        AddProjectileTrail(a_delta);
        FadeProjectileTrail(a_delta);
    }
//    if (data.replacedProjectileModel) {
//        auto projNiTransform = data.replacedProjectileModel->world;
//        auto projBHKTransform = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(data.replacedProjectileModel.get());
//        auto projHKPTransform = ObjectUtil::Node::GetHavokHKPRigidBodyWorldTransform(data.replacedProjectileModel.get());
//        spdlog::debug(
//            "NI: ({}, {}, {})  BHK: ({}, {}, {})",
//            projNiTransform.translate.x,
//            projNiTransform.translate.y,
//            projNiTransform.translate.z,
//            projBHKTransform.translate.x * 70.f,
//            projBHKTransform.translate.y * 70.f,
//            projBHKTransform.translate.z * 70.f
//        );
    //    spdlog::debug(
    //        "NI - BHK: ({}, {}, {})",
    //        projNiTransform.translate.x - projBHKTransform.translate.x * 70.f,
    //        projNiTransform.translate.y - projBHKTransform.translate.y * 70.f,
    //        projNiTransform.translate.z - projBHKTransform.translate.z * 70.f
    //    );
//    }
}
tState LeviathanAxe::GetThrowState()     const          {return throwState;}
void LeviathanAxe::SetThrowState(ThrowState a_state)    {throwState = a_state;}
void LeviathanAxe::GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor)
{
    if (!a_actor) return;
    if (data.model) {
        data.transformPW = data.model->world;
        data.transformPL = data.model->local;
    }
    if (LastLeviProjectile) {
        LastLeviProjectile->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
    }
    if (data.replacedProjectileModel) {
        data.transformW = GetWorldTransform();
        a_point = data.transformW.translate * 70.f;
    //    a_point = data.replacedProjectileModel->worldBound.center;
        spdlog::debug("leviathan coming from weapon model location.");
    } else spdlog::debug("we can't get leviathan's weapon model!");

    if (data.stuckedBone) {
        a_point = data.replacedProjectileModel ? data.transformW.translate : data.stuckedBone->world.translate;
        data.stuckedBone.reset();

        if (data.stuckedActor) {
#ifdef EXPERIMENTAL_EXTRAARROW
            ObjectUtil::Projectile::DeleteAnExtraArrow(data.stuckedActor, data.model);
#else
            data.stuckedActor->RemoveExtraArrows3D();
#endif
            spdlog::debug("levi stucked actor's extra arrows removed");
            data.stuckedActor.reset();
        } else spdlog::debug("levi not stucked anybody");
    } else spdlog::debug("levi not stucked any bone");

    if (GetThrowState() == tState::kThrowable) {
        if (auto backWeaponSheathe = a_actor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            const auto& backSheatheTransform = backWeaponSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            data.lastVelocity = (0.69f * rightDir + 0.31f * backDir) * 2400.f;
            spdlog::debug("levi is coming from your back sheathe");
        }
    } else {
        auto pcPos = a_actor->GetPosition();
        float dist = pcPos.GetDistance(a_point);
        if (dist > 36000.f) {   // ~42000 is limit
            spdlog::info("levi is too far away from you! ({} m)", (int)dist / 100);
            auto dir = a_point - pcPos;
            dir.Unitize();
            a_point = pcPos + dir * 36000.f;
        }
    }
}
void LeviathanAxe::Throw(const bool a_isVertical, const bool justContinue, const bool isHoming, RE::Actor* a_actor)
{
    if (!a_actor) {spdlog::error("LeviathanAxe::Throw - a_actor is null"); return;}
    WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
    if (!WeaponIdentify::RHandBone) {spdlog::error("LeviathanAxe::Throw - RHandBone is null"); return;}

    trailRemoveUpdate.Done();

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
        const auto leviDamage = data.damage;
        mag = leviDamage * WeaponIdentify::DamageMult * Config::ThrowingDamageMult;
        bool isPowerThrow; a_actor->GetGraphVariableBool("IsPowerThrowing", isPowerThrow);
        if (isVertical || isPowerThrow) {mag *= 1.5f; data.yAngle = 1.57f;}
        else data.yAngle = 0.35f;
        float throwChargeDamageMult = std::sqrtf(data.throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        mag *= throwChargeDamageMult;

        if (const auto leviProjBaseEff = leviProjEff->baseEffect; leviProjBaseEff && leviProjBaseEff->data.projectileBase) {
        //  //  leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
        //  //  leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
            auto& pbData = leviProjBaseEff->data.projectileBase->data;
            pbData.speed = !justContinue ? Config::ThrowSpeed * std::clamp(throwChargeDamageMult / 2.f, 1.f, 1.25f) : pbData.speed * 0.7f;
            pbData.force = mag;
            pbData.gravity = 3.21f;
        } else spdlog::warn("not found Levi throwing effect!");

        if (!justContinue) {
            soundData.PlayThrowingSounds(WeaponIdentify::RHandBone);

            data.gravity = 3.21f;
            data.gravity /= (std::powf(data.throwingChargeDuration + 1.f, 3.f));
            data.gravity = std::max(data.gravity, 0.5f);
        }

        auto kratos = Kratos::GetSingleton();
        //  set the launch data
        auto origin = justContinue ? data.position : WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {a_actor->GetAimAngle(), a_actor->GetAimHeading()};
        if (data.model && (justContinue/* || isHoming*/)) a_actor->Unk_A0(data.model.get(), pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, leviThrowSpell);

    //    lData.weaponSource = data.weap; // somehow causing very high damage
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(a_actor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor);
        else
            lData.enchantItem = nullptr;
        _isLastThrowCharged = lData.enchantItem != nullptr;
        //  throw the projectile
        RE::Projectile::Launch(&pHandle, lData);
        data.proj.reset(pHandle.get().get());

        if (WeaponIdentify::isLeviathanAxe) {
            const auto root = a_actor->Get3D1(false);
            auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
            auto copyWeaponModel = weapon3D ? weapon3D->Clone() : nullptr;
            if (copyWeaponModel) {
                copyWeaponModel->RemoveExtraData("BSX");
                copyWeaponModel->RemoveExtraData("BSXFlags");
                if (copyWeaponModel->GetCollisionObject())
                    copyWeaponModel->GetCollisionObject()->flags.reset(RE::bhkCollisionObject::Flag::kActive);
                copyWeaponModel->collisionObject.reset();
                auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
                data.weaponModelCopy.reset(copyWeaponModelNode);
                if (data.weaponModelCopy) {
                    data.weaponModelCopy->local = RE::NiTransform();
                    data.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
                }
            }
        }

        projectileUpdate.RegisterForUpdate(0.0f, false);

        if (isHoming) {
            if (justContinue) {
                homingLevi.proj = homingLevi.isBoomerang || !homingLevi.targets.empty() ? data.proj : nullptr;
            } else {
                std::vector<RE::Actor *> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(a_actor, Config::HProjectileTargetRange, true);
                homingLevi = HomingLeviathan(data.proj.get(), nearCombatTargets, a_actor, Config::HProjectileLoopCount, Config::ReturnHProjectileAfterLoops);
            }
        }
        if (justContinue) return;

        if (Config::IsAdvancedThrowingInstalled && (isThrowAttack || isPowerThrowAttack)) {
            ResetCharge(data.enchMag, data.defaultEnchMag, true);
            WeaponIdentify::skipEquipAnim = true;
            ObjectUtil::Actor::UnEquipItem(a_actor, false, false, true, true, WeaponIdentify::skipEquipAnim, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            spdlog::debug("Leviathan unequipped after throwing");
        } else {
            WeaponIdentify::isLeviathanAxe = false;
            WeaponIdentify::isRelic = false;
        //    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        //    a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
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
        data.throwingChargeDuration = 0.f;
        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        data.lastHitActors.clear();
        data.lastHitForms.clear();
        if (a_actor->HasSpell(SpellCatchLevi)) a_actor->RemoveSpell(SpellCatchLevi);
    } else spdlog::info("Leviathan Axe is not equipped for throwing!");
}
void LeviathanAxe::Call(const bool a_justDestroy, const bool a_justContinue, RE::Actor* a_actor)
{
    homingLevi.proj.reset();
    if (a_actor && data.weap) {
        spdlog::debug("Levi is calling...");
        projectileUpdate.Done();
        trailData.projTrail.reset();

        trailUpdate.Done();
        trailRemoveUpdate.Done();

        if (data.model) {
            data.transformPW = data.model->world;
            data.transformPL = data.model->local;
        }
        data.model.reset();
    //    if (data.replacedProjectileModel) {
    //        if (data.projState == ProjectileState::kHavok && data.replacedProjectileModel->collisionObject && data.replacedProjectileModel->collisionObject->AsBhkRigidBody()) {
    //            RE::hkTransform rbTransform;
    //            data.replacedProjectileModel->collisionObject->AsBhkRigidBody()->GetTransform(rbTransform);
    //            RE::NiTransform niTransform;
    //            niTransform.translate = MathUtil::Algebra::HkVectorToNiPoint(rbTransform.translation);
    //            niTransform.rotate = MathUtil::Algebra::HKMatrixToNiMatrix(rbTransform.rotation);
    //            data.transformW = niTransform;
    //            data.transformL = niTransform;
    //        } else {
    //            data.transformW = data.replacedProjectileModel->world;
    //            data.transformL = data.replacedProjectileModel->local;
    //        }
    //    }

        soundData.FadeThrowingLoopSounds(369);

        auto stuckedLevi =  LastLeviProjectile ? LastLeviProjectile : nullptr;
        if (!stuckedLevi)   stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
        if (stuckedLevi) {
            if (!a_justContinue) data.position = stuckedLevi->data.location;
            auto& runtimeData = stuckedLevi->GetProjectileRuntimeData();
            auto& pFlags = runtimeData.flags;
            if (!(pFlags & pFlag::kDestroyed)) {
                pFlags |= pFlag::kDestroyed;
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

            if (!Config::DontDamageWhileArrive) {
                const auto leviProjEff = SpellLeviProjA->effects[0];
                auto& leviProjEffSetting = leviProjEff->effectItem;
                auto& mag = leviProjEffSetting.magnitude;
                const auto leviDamage = data.damage;
                mag = leviDamage * WeaponIdentify::DamageMult * Config::ThrowingDamageMult;
                mag *= 0.5f;
                if (const auto leviProjBaseEff = leviProjEff->baseEffect; leviProjBaseEff && leviProjBaseEff->data.projectileBase) {
                //    leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
                //    leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
                    auto& pbData = leviProjBaseEff->data.projectileBase->data;
                    pbData.force = mag * 2.f;
                } else spdlog::warn("not found Levi arriving effect!");
                spdlog::debug("damage mult: {} throwing dm {} levi damage {} total damage {}", WeaponIdentify::DamageMult, Config::ThrowingDamageMult, leviDamage, mag);
            }

            RE::NiPoint3 startPoint = data.position;
            WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
            RE::NiPoint3  targetPoint = WeaponIdentify::RHandBone ? WeaponIdentify::RHandBone->world.translate : AnArchos->GetPosition();
            if (!a_justContinue) {
                soundData.PlayCallingHandSounds(WeaponIdentify::RHandBone);
                GetPosition(startPoint, AnArchos);
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
            data.proj.reset(pHandle.get().get());
            auto kratos = Kratos::GetSingleton();
            if (GetThrowState() == tState::kThrowable) kratos->SetIsCanCallAxe(a_actor, false);

            projectileUpdate.RegisterForUpdate(0.0f, false);

            if (a_justContinue) {
                arrivingLevi = ArrivingWeapon(arrivingLevi, pHandle.get().get(), startPoint);
            } else {
                arrivingLevi = ArrivingWeapon(this, pHandle.get().get(), a_actor, &WeaponIdentify::RHandBone, startPoint);
                spdlog::debug("Levi call is started");
            }
            SetThrowState(ThrowState::kArriving);
            spdlog::info("Levi is arriving...");
        } else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
}
void LeviathanAxe::Catch(const bool a_justDestroy, RE::Actor* a_actor)
{
    if (LeviathanAxeProjectileA) {
    //    if (APIs::precision || APIs::Request()) {
    //        APIs::precision->RemoveProjectileCollision(a_actor->GetHandle(), collisionDefinition);
    //    }

        auto& runtimeData = LeviathanAxeProjectileA->GetProjectileRuntimeData();
        runtimeData.flags |= pFlag::kDestroyed;
        if (a_justDestroy) return;
    }
    if (a_actor && !WeaponIdentify::isLeviathanAxe) {
        a_actor->SetGraphVariableBool("bLeviInCatchRange", true);
        if (EffCatchLevi && SpellCatchLevi && !a_actor->AsMagicTarget()->HasMagicEffect(EffCatchLevi)) {
            a_actor->AddSpell(SpellCatchLevi);
        }

        SetThrowState(ThrowState::kArrived);

        auto kratos = Kratos::GetSingleton();
        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
        if (auto handEffect = kratos->VFXeffect.handFrost; handEffect) 
            a_actor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);

        soundData.FadeArrivingNearSounds(469);
    //    soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTimeRealTime * 1200.f);
        soundData.PlayCatchingSounds(WeaponIdentify::RHandBone);

        if (WeaponIdentify::LeviathanAxe) {
            WeaponIdentify::unequipWhenAnimEnds = false;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
            a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            kratos->DoKratosAction(Kratos::Action::kWeaponCharge);
            ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::LeviathanAxe, WeaponIdentify::skipEquipAnim);//, 1U, true, false, false, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            RE::ShakeCamera(0.3f, data.position, 0.5f);
            if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
        } else spdlog::warn("you not have the leviathan axe");

        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        data.lastHitActors.clear();
        data.lastHitForms.clear();

        if (WeaponIdentify::AnimObjectRBone && WeaponIdentify::AnimObjectRBone->AsNode() && data.replacedProjectileModel && data.replacedProjectileModel->parent) {
            WeaponIdentify::AnimObjectRBone->AsNode()->AttachChild(data.replacedProjectileModel->parent);
            data.replacedProjectileModel->parent->local.translate = RE::NiPoint3();
            data.replacedProjectileModel->parent->local.rotate = data.replacedProjectileModel->parent->local.rotate * RE::NiMatrix3(PI2, 0.f, PI2);
        }
        trailUpdate.Done();
        data.model.reset();
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);

        isAxeCalled = false;
        std::jthread delayedCast([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            if (GetThrowState() == tState::kThrowable) kratos->SetIsCanCallAxe(a_actor, false);
        });
        delayedCast.detach();
    }
}
void LeviathanAxe::AddProjectileTrail(const float a_delta)
{
    if (trailUpdate.IsTimeToUpdate()) {
        trailRemoveUpdate.Done();
        DeleteProjectileTrail();
        auto bone = data.replacedProjectileModel;
        if (bone) {
            const bool isCharged = IsCharged(true);
            const float intensity = isCharged ? 3.f : 2.f;
            const auto meshOverride = isCharged ? Config::TrailModelPathFrost : Config::TrailModelPathDef;
            float length = bone->worldBound.radius;
            ObjectUtil::Capsule capsule;
            ObjectUtil::Node::GetCapsuleParams(bone->AsNode(), capsule);
            float capsuleLength = capsule.a.GetDistance(capsule.b);
            length = length > capsuleLength ? length : capsuleLength;
            float scale = fmax(length, capsule.radius) * 0.01f;
            float tipOffset = length;
            trailData = TrailData(meshOverride, intensity);

            if (Config::UsePrecisionTrails && (Config::IsPrecisionInstalled || APIs::precision || APIs::Request())) {
                trailUpdate.Done();
                trailData.transformOverride.additionalRotation = RE::NiMatrix3(0.f, 0.f, -NI_HALF_PI);
                trailData.transformOverride.scale = bone->worldBound.radius * 0.01f;
                auto node = RE::NiNode::Create(0);
                node->name = "trailParentNode";
                bone->AttachChild(node, false);
                APIs::precision->AddTrailEffect(
                    node, 
                    RE::PlayerCharacter::GetSingleton()->GetHandle(), 
                    RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                    trailData.trailOverride, 
                    trailData.transformOverride);
                if (isCharged) {
                    trailData.trailOverride.meshOverride = Config::TrailModelPathDef;
                    APIs::precision->AddTrailEffect(
                        node, 
                        RE::PlayerCharacter::GetSingleton()->GetHandle(), 
                        RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                }
            //    APIs::precision->AddAttackCollision(RE::PlayerCharacter::GetSingleton()->GetHandle(), collisionDefinition, LastLeviProjectile);
            } else {
                auto weaponForward = frontVec3;
                trailData.trailTransform = bone->world;
                trailData.trailTransform.translate +=
                    trailData.trailTransform.rotate * weaponForward * tipOffset;

            //    auto dir = data.trailTransform.translate - bone->world.translate;
                auto dir = data.lastOrientation;
            //    dir.Unitize();
                MathUtil::Algebra::SetRotationMatrix(trailData.trailTransform.rotate, -dir.x, dir.y, dir.z);
                RE::NiMatrix3 weaponRotation(0.f, NI_HALF_PI, -NI_HALF_PI);
                trailData.trailTransform.rotate = trailData.trailTransform.rotate * weaponRotation;
                RE::NiMatrix3 flip90(-NI_HALF_PI, 0.f, 0.f);
                trailData.trailTransform.rotate = trailData.trailTransform.rotate * flip90;

                if (!trailData.projTrail) {
                    trailData.projTrail = RE::NiPointer<RE::BSTempEffectParticle>(
                        RE::BSTempEffectParticle::Spawn(
                            RE::PlayerCharacter::GetSingleton()->GetParentCell(),
                            10.f,
                            trailData.trailOverride.meshOverride.value().data(),
                            trailData.trailTransform.rotate,
                            trailData.trailTransform.translate,
                            scale,
                            7,
                            nullptr));

                    trailData.trailRootNode.reset();
                    trailData.trailTimeAccumulator = 0.f;
                    trailData.segmentTimestamps.clear();
                    spdlog::warn("Created trail particle.");
                } else if (!trailData.trailRootNode) {
                    auto particleObject = trailData.projTrail ? trailData.projTrail->particleObject : nullptr;
                    auto fadeNode = particleObject ? particleObject->AsFadeNode() : nullptr;
                    auto trailRoot = fadeNode ? fadeNode->GetObjectByName("TrailRoot"sv) : nullptr;
                    trailData.trailRootNode.reset(trailRoot ? trailRoot->AsNode() : nullptr);
                    trailData.trailSegmentCount = trailData.trailRootNode ? trailData.trailRootNode->GetChildren().size() : 0u;

                    trailData.trailTransformHistory.clear();
                //    for (uint32_t i = 0; i < trailData.trailSegmentCount; i++)
                //        trailData.trailTransformHistory.push_front(trailData.trailTransform);

                    if (trailData.trailRootNode)
                        if (trailData.trailRootNode->GetChildren().empty())
                            spdlog::error("Trail root node has no children! Check the trail model path!");
                        else
                            spdlog::info("Found projectile trail root node!");
                    else 
                        spdlog::warn("Cannot find projectile trail root node...");
                } else {
                    if (auto fadeNode = trailData.projTrail->particleObject ? trailData.projTrail->particleObject->AsFadeNode() : nullptr; fadeNode) {
                        fadeNode->currentFade = 1.f;

                        trailData.trailRootNode->world = trailData.trailTransform;
                        ObjectUtil::Node::UpdateNodeTransformLocal(trailData.trailRootNode.get(), trailData.trailTransform);
                        trailData.trailTransformHistory.emplace_back(trailData.trailTransform);
                    }
                }
            }
        }
    } else if (trailData.projTrail) {
        trailData.projTrail->age += trailData.projTrail->lifetime;
        if (trailData.projTrail->particleObject && trailData.projTrail->particleObject->AsGeometry()) {
            auto effect = trailData.projTrail->particleObject->AsGeometry()->properties[RE::BSGeometry::States::kEffect];
            auto effectShader = netimmerse_cast<RE::BSEffectShaderProperty*>(effect.get());
            if (effectShader) {
                auto effectShaderMaterial = skyrim_cast<RE::BSEffectShaderMaterial*>(effectShader->material);
                if (effectShaderMaterial) {
                    if (auto newMaterial = static_cast<RE::BSEffectShaderMaterial*>(effectShaderMaterial->Create())) {
                        newMaterial->CopyMembers(effectShaderMaterial);
                        effectShader->SetMaterial(newMaterial, false);
                        newMaterial->~BSEffectShaderMaterial();
                        RE::free(newMaterial);

                        effectShaderMaterial = skyrim_cast<RE::BSEffectShaderMaterial*>(effectShader->material);
                        if (effectShaderMaterial->baseColor.alpha > 0.f)
                            effectShaderMaterial->baseColor.alpha -= a_delta / 1.5f;
                        else {
                            effectShaderMaterial->baseColor.alpha = 0.f;
                            trailData.projTrail.reset();
                            trailData.trailRootNode.reset();
                        }
                    }
                }
            }
        } else if (auto fadeNode = trailData.projTrail->particleObject ? trailData.projTrail->particleObject->AsFadeNode() : nullptr; fadeNode) {
            if (fadeNode->currentFade > 0.f) {
                float fadeDuration = 0.5f;
                fadeNode->currentFade -= a_delta / fadeDuration;
            } else {
                fadeNode->currentFade = 0.f;
                trailData.projTrail.reset();
                trailData.trailRootNode.reset();
            }
        } else {
            trailData.projTrail.reset();
            trailData.trailRootNode.reset();
        }
    }
}
void LeviathanAxe::FadeProjectileTrail(const float a_delta)
{
    if (trailRemoveUpdate.IsTimeToUpdate()) {
        if (data.replacedProjectileModel) {
            if (data.proj && data.projState == ProjectileState::kHavok) {
        //        auto& rtData = data.proj->GetProjectileRuntimeData();
                auto velocity = (data.replacedProjectileModel->world.translate - data.replacedProjectileModel->previousWorld.translate) / *g_deltaTime;
                auto speed = velocity.Length();//rtData.linearVelocity.Length();
                spdlog::debug("projectile trail fading... current speed: {}", speed);
                if (speed != 0.f && speed < 669.f) {
                    DeleteProjectileTrail();
                    trailRemoveUpdate.Done();
                }
            } else {
                DeleteProjectileTrail();
                data.replacedProjectileModel.reset();
                trailRemoveUpdate.Done();
            }
        }
    }
}
void LeviathanAxe::DeleteProjectileTrail()
{
    if (data.replacedProjectileModel) {
        auto trailParentBone = data.replacedProjectileModel->GetObjectByName("trailParentNode");
        data.replacedProjectileModel->DetachChild(trailParentBone);
    //    if (data.replacedProjectileModel->parent)
    //        data.replacedProjectileModel->parent->DetachChild(data.replacedProjectileModel.get());
        if (WeaponIdentify::AnimObjectRBone) {
            WeaponIdentify::AnimObjectRBone->AsNode()->DetachChild(data.replacedProjectileModel->parent);
        }
        spdlog::debug("projectile trail deleted");
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
    WeaponIdentify::WeaponBone = WeaponIdentify::GetWeaponBone(AnArchos);

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
bool LeviathanAxe::IsArriving(RE::Projectile* a_proj) const {return a_proj == arrivingLevi.proj.get();}
bool LeviathanAxe::IsHoming(RE::Projectile* a_proj) const {return a_proj == homingLevi.proj.get();}
void LeviathanAxe::StartChargingThrow(RE::Actor* a_actor)
{
    if (auto kratos = Kratos::GetSingleton(); a_actor && kratos) {
        kratos->_soundHandle.Stop();
        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
        ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviLoop, kratos->_soundHandle, WeaponIdentify::RHandBone, 5.f);
        a_actor->ApplyArtObject(kratos->VFXeffect.handFrostBright, 5.f, nullptr, false, false, WeaponIdentify::RHandBone);
        kratos->SetIsChargingThrow(true);
    }
}
RE::NiTransform LeviathanAxe::GetWorldTransform()
{
    if (data.replacedProjectileModel) {
        data.transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(data.replacedProjectileModel.get());
        return data.transformW;
    } else return data.transformPW;
    return {};
}
RE::NiTransform LeviathanAxe::GetLocalTransform()
{
    RE::NiTransform ret;
    if (data.replacedProjectileModel) {
        data.transformL = data.replacedProjectileModel->local;
        ret = data.transformL;
    } else ret = data.transformPL;
    return ret;
}
#pragma region Trails
RE::NiColorA LeviathanAxe::TrailData::GetColorByIndex(const uint32_t a_index)
{
    switch ((TrailColor)a_index) {
    case TrailColor::kWhite:
        return WHITE;
    case TrailColor::kIceBlue:
        return ICEBLUE;
    case TrailColor::kSkyBlue:
        return SKYBLUE;
    case TrailColor::kBlue:
        return BLUE;
    case TrailColor::kYellow:
        return YELLOW;
    case TrailColor::kGold:
        return GOLD;
    case TrailColor::kSilver:
        return SILVER;
    default:
        return WHITE;
    }
}
#pragma endregion
#pragma region Arriving
void LeviathanAxe::ArrivingWeapon::UpdateRotation()
{
    if (parent->data.replacedProjectileModel && parent->data.replacedProjectileModel->parent && WeaponIdentify::GetWeaponBone()) {
        auto& replacedPMParent = parent->data.replacedProjectileModel->parent;
        auto& localRotation = replacedPMParent->local.rotate;
        RE::NiMatrix3 targetLocalRotation;
    //    float targetAngleZ;
        if (replacedPMParent->parent) {
            targetLocalRotation = replacedPMParent->parent->world.rotate.Transpose() * WeaponIdentify::WeaponBone->world.rotate;
        //    targetAngleZ = std::atan2(
        //        targetLocalRotation.entry[0][1],
        //        targetLocalRotation.entry[0][0]
        //    );
        } else {
            targetLocalRotation = WeaponIdentify::WeaponBone->world.rotate;
        //    targetAngleZ = std::atan2(
        //        targetLocalRotation.entry[0][1],
        //        targetLocalRotation.entry[0][0]
        //    );
        }
        const float angleZ = livingTime * rotationSpeed;
        MathUtil::Angle::NormalAbsoluteAngle(angleZ);
        arrivalSpin = (int)(std::floor(1 + (livingTime + remainingTimeToArrive) * rotationSpeed / TWO_PI)) * TWO_PI;
        const float blendZ = std::lerp(angleZ, arrivalSpin, tReal);
        const auto& startRotationC = startRotation;
        localRotation = MathUtil::Algebra::InterpolateRotation(startRotationC, targetLocalRotation, tReal);
        localRotation = localRotation * RE::NiMatrix3(0.f, 0.f, blendZ);
    //    spdlog::debug("target angle: {} calculated target angle: {} blended angle: {} tau: {}", angleZ, arrivalSpin, blendZ, tReal);
    }
//    leviAngle.x = asin(desiredDir.z);
//    leviAngle.z = atan2(desiredDir.x, desiredDir.y);
//    if (leviAngle.z < 0.0) {
//        leviAngle.z += PI;
//    }
//    if (desiredDir.x < 0.0) {
//        leviAngle.z += PI;
//    }
}
void LeviathanAxe::ArrivingWeapon::UpdateAI(RE::NiPoint3& a_outVel)
{
    float height = position.z - caller->GetPosition().z;
    if (!Config::DontDamageWhileArrive && tReal < 0.69f) {
        if (auto aTarget = GetNextTarget(position); aTarget) {
            auto targetPos = aTarget->GetPosition() + (aTarget->GetBoundMax() + aTarget->GetBoundMin()) * 0.75f;
            auto targetDir = (targetPos - position);
            targetDir.Unitize();
            height = position.z - aTarget->GetPosition().z;
            a_outVel = targetDir * speed;
        }
    }
}
void LeviathanAxe::ArrivingWeapon::UpdateArrivingDirection(const bool a_initial)
{
    if (RE::PlayerCamera::GetSingleton()->IsInFirstPerson()) {

    } else if (caller && parent && callerBreastBone) {
        if (parent->GetThrowState() == ThrowState::kThrowable || isCatchable) {

        } else if (a_initial || linearDistance > 100.f) {
            RE::NiPoint3  spineForwardDir = callerBreastBone->world.rotate * RE::NiPoint3(frontVec3);
            spineForwardDir.z = 0.f;  //  ignore vertical direction
            spineForwardDir.Unitize();

            RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
            linearDir2D.Unitize();

            float dot = spineForwardDir.Dot(linearDir2D);
            float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;

            arrivingRelativeAngleZ = atan2(det, dot);  //  angle between spine forward direction and arriving weapon direction
            arrivingRelativeAngleZ = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngleZ);      //  normalize angle to [0, 2PI]

            float snapStrength = arrivingRelativeAngleSnapStrength;
            if (a_initial) snapStrength = 1.f;
            else if ((arrivingRelativeAngleZ < PI4 || arrivingRelativeAngleZ > ONEANDHALF_PI) && snapStrength < 0.69f) snapStrength += 0.69f;
            snapStrength = std::min(snapStrength, 1.f);
            if (snapStrength > 0.f) {
                arrivingRelativeAngleZ = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngleZ, arrivingDirections, snapStrength);    //  for helping to the blender generator
            }

            if (!a_initial) {
                float previousAngle; caller->GetGraphVariableFloat("fArrivingWeaponDirection", previousAngle); previousAngle *= TWO_PI;
                const float delta = MathUtil::Angle::NormalizeSignedAngle(arrivingRelativeAngleZ - previousAngle);
                constexpr float smoothTime = 0.369f;
                const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
                float smoothedArrivingRelativeAngle = previousAngle;
                smoothedArrivingRelativeAngle += delta * alpha;
                arrivingRelativeAngleZ = smoothedArrivingRelativeAngle;
                
            }
            arrivingRelativeAngleZ /= TWO_PI;
        }
        caller->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngleZ);
    }
}
void LeviathanAxe::ArrivingWeapon::UpdateArrivingRoute()
{
    RE::NiMatrix3 handRot   = callerHandBone->world.rotate;
    const float alphaHandRot = 1.f - std::exp(-*g_deltaTimeRealTime / 0.169f);
    RE::NiMatrix3 smoothedHandRot = 
        MathUtil::Algebra::QuaternionToMatrix(MathUtil::Algebra::Slerp(
            MathUtil::Algebra::MatrixToQuaternion(callerHandBone->previousWorld.rotate),
            MathUtil::Algebra::MatrixToQuaternion(callerHandBone->world.rotate), alphaHandRot));
    RE::NiPoint3 palmDir    = smoothedHandRot * RE::NiPoint3(backVec3);
    RE::NiPoint3 handForward= smoothedHandRot * RE::NiPoint3(upVec3);
    palmDir.Unitize();
    handForward.Unitize();

    const float handSideOffsetMult = MathUtil::Algebra::ParabolicClamp(arrivingRelativeAngleZ, 0.f, 0.25f);

    RE::NiPoint3 handVelocity = (callerHandBone->world.translate - callerHandBone->previousWorld.translate) / *g_deltaTimeRealTime;
    const float predictionTime = std::clamp(*g_deltaTimeRealTime, 0.0f, 0.1f);
    RE::NiPoint3 predictedHandPos = handPosition + handVelocity * predictionTime;
    bezierControlPoints[3] = predictedHandPos;
    bezierControlPoints[2] = predictedHandPos + (float)IsInCallingAnimation() * (palmDir * (linearDistanceFromLastCallPos * 0.33f + 20.f) + handForward * (linearDistanceFromLastCallPos * handSideOffsetMult + 10.f));

    RE::NiPoint3& p0 = bezierControlPoints[0];
    RE::NiPoint3& p1 = bezierControlPoints[1];
    RE::NiPoint3& p3 = bezierControlPoints[3];
    RE::NiPoint3& p2 = bezierControlPoints[2];

    arrivingRoute = MathUtil::Algebra::CalculateAndMeasureBezier(
        p0, p1, p2, p3,
        routeResolution);
    float bestDist2 = FLT_MAX;

    for (int i = 0; i < arrivingRoute.samples.size() - 1; i++) {
        const float d2 = (arrivingRoute.samples[i].point - position).SqrLength();
        if (d2 < bestDist2)
        {
            bestDist2 = d2;
            arrivingRouteClosestIndex = i;
        }
    }

    closestSample = arrivingRoute.samples[arrivingRouteClosestIndex];
    const float t = closestSample.t;

    const float lookAheadDistance = std::clamp(std::abs(speed) * 0.15f, 50.f, 200.f);
    const float targetDistance = closestSample.distanceFromStart + lookAheadDistance;

    int targetIndex = arrivingRouteClosestIndex;
    while (targetIndex + 1 <
        static_cast<int>(arrivingRoute.samples.size()) &&
        arrivingRoute.samples[targetIndex].distanceFromStart < targetDistance)
    {
        targetIndex++;
    }

    bezierDir = arrivingRoute.samples[targetIndex].point - position;
    bezierDir.Unitize();
}
void LeviathanAxe::ArrivingWeapon::Update(const float a_delta)
{
    if (!parent) return;

    model = parent->data.model;
    if (!model) return;

    callerHandBone = GetCallerHandBone();
    if (!callerHandBone) return;
    handPosition = callerHandBone->world.translate;

    if (!proj) return;
    auto& rtData = proj->GetProjectileRuntimeData();
    auto& vel = rtData.linearVelocity;

    currentDir = vel;
    currentDir.Unitize();

    position = proj->data.location;

    linearDistance = handPosition.GetDistance(position);
    linearArrivingDir = handPosition - position;
    linearArrivingDir.Unitize();

    isCatchable = (linearDistance <= Config::CatchingTreshold) || (linearDistance <= (*g_deltaTime * vel.Length()));

    if (parent->LeviathanAxeProjectileA != proj.get()) {  //  first frame of the arriving projectile
        parent->LeviathanAxeProjectileA = proj.get();

        if (!isCatchable) {
            parent->soundData.PlayArrivingStartSounds(model.get());
            parent->soundData.PlayArrivingLoopSounds(model.get());
        }
    }
    if (isCatchable) {
        if (parent->GetThrowState() == tState::kArriving) parent->SetThrowState(tState::kArrived);
        parent->Catch();
        spdlog::debug("Levi proj catched");
    }
    if (false && startRotation == RE::NiMatrix3()) {
        if (parent->data.replacedProjectileModel && parent->data.replacedProjectileModel->parent) {
            model.get()->world = parent->data.transformPW;
            model.get()->local = parent->data.transformPL;
            auto& replacedPMParent = parent->data.replacedProjectileModel->parent;
            auto parentWorldInverse = replacedPMParent->world.Invert();
            auto previousWorld = parent->data.transformW;
            auto& localRotation = replacedPMParent->local.rotate;
            auto& localPosition = replacedPMParent->local.translate;
            if (replacedPMParent->parent) {
                localRotation = parentWorldInverse.rotate * previousWorld.rotate;
            //    localPosition = parentWorldInverse.rotate * (previousWorld.translate - localPosition);
            } else {
                localRotation = previousWorld.rotate;
            //    localPosition = previousWorld.translate;
            }
            startRotation = localRotation;
            spdlog::debug("start rotation initiated");
        }
    }

    livingTime = GetLivingTime();
    UpdateArrivingRoute();

    const float minArrivalTime = *g_deltaTime * 2.f;
    float remainingRouteLength = arrivingRoute.arcLength - closestSample.distanceFromStart;
    remainingRouteLength = std::max(remainingRouteLength, linearDistance);
    remainingTimeToArrive = std::max(timeToArrive - livingTime, minArrivalTime);
    const float requiredAverageSpeed = remainingRouteLength / remainingTimeToArrive;

    if (requiredAverageSpeed < Config::MinArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MinArrivalSpeed, minArrivalTime);
    else if (requiredAverageSpeed > Config::MaxArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MaxArrivalSpeed, minArrivalTime);

    const float desiredAcceleration = 2.f * (remainingRouteLength - speed * remainingTimeToArrive) / (remainingTimeToArrive * remainingTimeToArrive);
    speed += desiredAcceleration * *g_deltaTimeRealTime;
    speed = std::max(speed, Config::MinArrivalSpeed);

    remainingTimeToArrive = remainingRouteLength / speed;
    constexpr float nearArriveTimeThreshold = 0.369f;
    isNear = remainingTimeToArrive < (nearArriveTimeThreshold < timeToArrive ? nearArriveTimeThreshold : timeToArrive * 0.8f);
    constexpr float almostArrivedTimeThreshold = 0.169f;
    isAlmostArrived = remainingTimeToArrive < (almostArrivedTimeThreshold < timeToArrive ? almostArrivedTimeThreshold : timeToArrive * 0.5f);
    if (isAlmostArrived || isCatchable) {
        uint16_t fadeDuration = (uint16_t)(almostArrivedTimeThreshold * 1000.f) + 200u;
        parent->soundData.FadeArrivingStartSounds(fadeDuration);
        parent->soundData.FadeArrivingLoopSounds(fadeDuration);
        parent->soundData.FadeCallingHandSounds(fadeDuration);
    } else if (isNear) {
        parent->soundData.PlayArrivingNearSounds(model.get());
    }
//    spdlog::debug(
//        "AFTER ACCEL speed={:.1f}, accel={:.1f}, L={:.1f}, T={:.3f}",
//        speed,
//        desiredAcceleration,
//        remainingRouteLength,
//        remainingTimeToArrive);
//    spdlog::debug(
//        "closest={} target={} closestT={:.3f} targetT={:.3f} "
//        "closestL={:.1f} targetL={:.1f} routeL={:.1f}",
//        arrivingRouteClosestIndex,
//        targetIndex,
//        closestSample.t,
//        arrivingRoute.samples[targetIndex].t,
//        closestSample.distanceFromStart,
//        arrivingRoute.samples[targetIndex].distanceFromStart,
//        arrivingRoute.arcLength
//    );

//    tReal = 1.f - remainingTimeToArrive / almostArrivedTimeThreshold;//timeToArrive;
    tReal = timeToArrive / (timeToArrive + 2.f * (remainingTimeToArrive - 0.05f));
    tReal = std::pow(tReal, 2.f);
    tReal = std::clamp(tReal, 0.f, 1.f);
//    tReal = tReal * tReal * (3.f - 2.f * tReal);

    const float smoothTime = std::clamp(1.f - tReal, 0.01f, 0.069f);
    const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
    smoothedDesiredVelocity += (bezierDir * speed - smoothedDesiredVelocity) * alpha;
    desiredDir = smoothedDesiredVelocity;
    desiredDir.Unitize();
    const bool doBlend = 
        parent->data.projState == ProjectileState::kNone ||
        parent->data.projState == ProjectileState::kLaunched;
    vel = MathUtil::Angle::BlendVectors(doBlend ? parent->data.lastVelocity : (linearArrivingDir * speed), desiredDir * speed, livingTime / 0.2f);
    if (vel.z < 0.f) {
        constexpr float minHeight = -40.f;
        constexpr float dampingRange = 69.f;
        const float height = position.z - handPosition.z;
        const float dampFactor = std::clamp((height - minHeight) / dampingRange, 0.f, 1.f);
        vel.z *= dampFactor;
    }

    UpdateRotation();
    UpdateAI(vel);
    UpdateArrivingDirection();
}
#pragma endregion
#pragma region Sounds
void LeviathanAxe::SoundData::StopAllSounds()
{
    if (CallStartSH.IsPlaying()) CallStartSH.Stop();
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.Stop();
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.Stop();
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.Stop();
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.Stop();
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.Stop();
    if (ThrowingStartSH.IsPlaying()) ThrowingStartSH.Stop();
    if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.Stop();
    if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.Stop();
    if (CatchSH.IsPlaying()) CatchSH.Stop();
}
void LeviathanAxe::SoundData::PauseAllSounds()
{
//    if (CallStartSH.IsPlaying() && GetState(SoundName::kCallStart) == State::kTriggered) {CallStartSH.Pause(); soundState[SoundName::kCallStart] = State::kPaused;}
//    if (ArrivingStartSH.IsPlaying() && GetState(SoundName::kArrivingStart) == State::kTriggered) {ArrivingStartSH.Pause(); soundState[SoundName::kArrivingStart] = State::kPaused;}
    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingNearSH.IsPlaying() && GetState(SoundName::kArrivingNear) == State::kTriggered) {ArrivingNearSH.Pause(); soundState[SoundName::kArrivingNear] = State::kPaused;}
//    if (ThrowingStartSH.IsPlaying() && GetState(SoundName::kThrowingStart) == State::kTriggered) {ThrowingStartSH.Pause(); soundState[SoundName::kThrowingStart] = State::kPaused;}
    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    else if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (CatchSH.IsPlaying() && GetState(SoundName::kCatch) == State::kTriggered) {CatchSH.Pause(); soundState[SoundName::kCatch] = State::kPaused;}

//    if (CallStartSH.IsPlaying() && GetState(SoundName::kCallStart) == State::kTriggered) {CallStartSH.Pause(); soundState[SoundName::kCallStart] = State::kPaused;}
//    if (ArrivingStartSH.IsPlaying() && GetState(SoundName::kArrivingStart) == State::kTriggered) {ArrivingStartSH.Pause(); soundState[SoundName::kArrivingStart] = State::kPaused;}
//    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop0SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop1SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop2SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingNearSH.IsPlaying() && GetState(SoundName::kArrivingNear) == State::kTriggered) {ArrivingNearSH.Pause(); soundState[SoundName::kArrivingNear] = State::kPaused;}
//    if (ThrowingStartSH.IsPlaying() && GetState(SoundName::kThrowingStart) == State::kTriggered) {ThrowingStartSH.Pause(); soundState[SoundName::kThrowingStart] = State::kPaused;}
//    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {ThrowingLoop0SH.Pause(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {ThrowingLoop1SH.Pause(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (CatchSH.IsPlaying() && GetState(SoundName::kCatch) == State::kTriggered) {CatchSH.Pause(); soundState[SoundName::kCatch] = State::kPaused;}
}
void LeviathanAxe::SoundData::ContinueAllSounds()
{
    if (GetState(SoundName::kCallStart) == State::kPaused) {CallStartSH.Play(); soundState[SoundName::kCallStart] = State::kPlaying;}
    if (GetState(SoundName::kArrivingStart) == State::kPaused) {ArrivingStartSH.Play(); soundState[SoundName::kArrivingStart] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop0SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop1SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop2SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingNear) == State::kPaused) {ArrivingNearSH.Play(); soundState[SoundName::kArrivingNear] = State::kPlaying;}
    if (GetState(SoundName::kThrowingStart) == State::kPaused) {ThrowingStartSH.Play(); soundState[SoundName::kThrowingStart] = State::kPlaying;}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {ThrowingLoop0SH.Play(); soundState[SoundName::kThrowingLoop] = State::kPlaying;}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {ThrowingLoop1SH.Play(); soundState[SoundName::kThrowingLoop] = State::kPlaying;}
    if (GetState(SoundName::kCatch) == State::kPaused) {CatchSH.Play(); soundState[SoundName::kCatch] = State::kPlaying;}
}
void LeviathanAxe::SoundData::PauseAllLoopingSounds()
{
    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    else if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
}
void LeviathanAxe::SoundData::ContinueAllLoopingSounds()
{
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {PlayArrivingLoopSounds(weaponData->model.get());}// soundState[SoundName::kArrivingLoop] = State::kPlaying; spdlog::debug("ArrivingLoop sounds continuing...");}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {PlayThrowingLoopSounds(weaponData->model.get());}// soundState[SoundName::kThrowingLoop] = State::kPlaying; spdlog::debug("ThrowingLoop sounds continuing...");}
}
void LeviathanAxe::SoundData::FadeAllSounds(const uint16_t a_durationMS)
{
    if (CallStartSH.IsPlaying()) CallStartSH.FadeOutAndRelease(a_durationMS);
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.FadeOutAndRelease(a_durationMS);
    if (CatchSH.IsPlaying()) CatchSH.FadeOutAndRelease(a_durationMS);
}
void LeviathanAxe::SoundData::PlayCallingHandSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.callLevi; soundEffect)
        ObjectUtil::Sound::PlaySound(soundEffect, CallStartSH, a_source, 5.f);
}
void LeviathanAxe::SoundData::PlayArrivingStartSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.arrivingLeviStart; soundEffect && IsSoundValid(SoundName::kArrivingStart)) {
        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingStartSH, a_source, 5.f);
        soundState[SoundName::kArrivingStart] = State::kTriggered;
    } else {
        if (ArrivingStartSH.IsPlaying())
            ArrivingStartSH.SetObjectToFollow(a_source);
    }
}
void LeviathanAxe::SoundData::PlayArrivingLoopSounds(RE::NiAVObject* a_source)
{
    if (IsSoundValid(SoundName::kArrivingLoop)) {
        if (auto soundEffect = kratos->soundEffect.arrivingLeviLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kArrivingLoop] = State::kTriggered;
        }
        if (auto soundEffect = kratos->soundEffect.arrivingLeviLoop1; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop1SH, a_source, 0.5f);
            soundState[SoundName::kArrivingLoop] = State::kTriggered;
        }
    //    if (auto soundEffect = kratos->soundEffect.arrivingLeviLoop2; soundEffect) {
    //        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop2SH, a_source, 0.5f);
    //        soundState[SoundName::kArrivingLoop] = State::kTriggered;
    //    }
    } else {
        spdlog::debug("updating the following node of the arriving loop sounds.");
        if (ArrivingLoop0SH.IsPlaying()) {
            ArrivingLoop0SH.SetObjectToFollow(a_source);
            ArrivingLoop0SH.Play();
        } if (ArrivingLoop1SH.IsPlaying()) {
            ArrivingLoop1SH.SetObjectToFollow(a_source);
            ArrivingLoop1SH.Play();
        }// if (ArrivingLoop2SH.IsPlaying()) {
    //        ArrivingLoop2SH.SetObjectToFollow(a_source);
    //        ArrivingLoop2SH.Play();
    //    }
    }
}
void LeviathanAxe::SoundData::PlayArrivingNearSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.arrivingLeviNear; soundEffect && IsSoundValid(SoundName::kArrivingNear) ) {
        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingNearSH, a_source, 5.f);
        soundState[SoundName::kArrivingNear] = State::kTriggered;
    } else {
        if (ArrivingNearSH.IsPlaying())
            ArrivingNearSH.SetObjectToFollow(a_source);
    }
}
void LeviathanAxe::SoundData::PlayCatchingSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.catchLevi; soundEffect)
        ObjectUtil::Sound::PlaySound(soundEffect, CatchSH, a_source, 5.f);
}
void LeviathanAxe::SoundData::PlayThrowingSounds(RE::NiAVObject* a_source)
{
//    if (auto soundEffect = kratos->soundEffect.throwingLeviNear; soundEffect)
//        ThrowingStartSH = ObjectUtil::Sound::PlaySound(soundEffect, a_source, 5.f);
}
void LeviathanAxe::SoundData::PlayThrowingLoopSounds(RE::NiAVObject* a_source)
{
    if (IsSoundValid(SoundName::kThrowingLoop)) {
        if (auto soundEffect = kratos->soundEffect.throwingLeviLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ThrowingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kThrowingLoop] = State::kTriggered;
        }
    //    if (auto soundEffect = kratos->soundEffect.throwingLeviLoop1; soundEffect) {
    //        ObjectUtil::Sound::PlaySound(soundEffect, ThrowingLoop1SH, a_source, 0.5f);
    //        soundState[SoundName::kThrowingLoop] = State::kTriggered;
    //    }
    } else {
        spdlog::debug("updating the following node of the throwing loop sounds.");
        if (ThrowingLoop0SH.IsPlaying()) {
            ThrowingLoop0SH.SetObjectToFollow(a_source);
            ThrowingLoop0SH.Play();
        }// if (ThrowingLoop1SH.IsPlaying()) {
    //        ThrowingLoop1SH.SetObjectToFollow(a_source);
    //        ThrowingLoop1SH.Play();
    //    }
    }
}
void LeviathanAxe::SoundData::FadeCallingHandSounds(const uint16_t a_durationMS)
{
    if (CallStartSH.IsPlaying()) CallStartSH.FadeOutAndRelease(a_durationMS);
}
void LeviathanAxe::SoundData::FadeArrivingStartSounds(const uint16_t a_durationMS)
{
    
    soundState[SoundName::kArrivingStart] = State::kFading;
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.FadeOutAndRelease(a_durationMS);
    else soundState[SoundName::kArrivingStart] = State::kStopped;
}
void LeviathanAxe::SoundData::FadeArrivingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kArrivingLoop] = State::kFading;
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}

    StopArrivingLoopSounds(a_durationMS);
}
void LeviathanAxe::SoundData::FadeArrivingNearSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kArrivingNear] = State::kFading;
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.FadeOutAndRelease(a_durationMS);
    else soundState[SoundName::kArrivingNear] = State::kStopped;
}
void LeviathanAxe::SoundData::FadeThrowingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kThrowingLoop] = State::kFading;
    if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kStopped;}
    if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kStopped;}

    StopThrowingLoopSounds(a_durationMS);
}
void LeviathanAxe::SoundData::StopArrivingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ArrivingLoop0SH.IsPlaying()) {ArrivingLoop0SH.Stop(); spdlog::debug("arriving loop sound 1 stopped.");}
        else {spdlog::debug("arriving loop sound 1 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        if (ArrivingLoop1SH.IsPlaying()) {ArrivingLoop1SH.Stop(); spdlog::debug("arriving loop sound 2 stopped.");}
        else {spdlog::debug("arriving loop sound 2 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        if (ArrivingLoop2SH.IsPlaying()) {ArrivingLoop2SH.Stop(); spdlog::debug("arriving loop sound 3 stopped.");}
        else {spdlog::debug("arriving loop sound 3 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        soundState[SoundName::kArrivingLoop] = State::kStopped;
    } else {
        arrivingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
void LeviathanAxe::SoundData::StopThrowingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.Stop();
        else {spdlog::debug("throwing loop sound is already stopped."); soundState[SoundName::kThrowingLoop] = State::kStopped;}
        if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.Stop();
        else {spdlog::debug("throwing loop sound is already stopped."); soundState[SoundName::kThrowingLoop] = State::kStopped;}
        soundState[SoundName::kThrowingLoop] = State::kStopped;
    } else {
        throwingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
#pragma endregion
#pragma endregion
#pragma region BLADES_OF_CHAOS
bool BladeOfChaos::Initialize()
{
    bool found = true;
    auto dataHandler    = RE::TESDataHandler::GetSingleton();
    Config::RetroMovesetSpell = dataHandler->LookupForm<RE::SpellItem>   (0x811, Config::BladesOfChaosModESP);
    if (Config::RetroMovesetSpell)
            spdlog::debug("RetroMovesetSpell found");
    else     {spdlog::error("RetroMovesetSpell can't found!"); found = false;}
    HideChains();
    return found;
}
wStateB BladeOfChaos::GetWeaponState()     const          {return weaponState;}
void BladeOfChaos::SetWeaponState(WeaponState a_state)    {weaponState = a_state;}
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
}
void BladeOfChaos::BuffScorchingSpeed(const float a_buff, const bool a_forced)
{
    _fScorchingSpeed += a_buff;
    if (!a_forced && _fScorchingSpeed > 1.f) _fScorchingSpeed = 1.f;

    RE::PlayerCharacter::GetSingleton()->SetGraphVariableFloat("ScorchingSpeed", _fScorchingSpeed);
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
void BladeOfChaos::HideChains(const bool a_hide)
{return;
    auto AnArchos = RE::PlayerCharacter::GetSingleton();
    if (AnArchos && AnArchos->Get3D1(false)) {
        data.weaponModel  = AnArchos->Get3D1(false)->GetObjectByName("WEAPON");
        data.weaponModelL = AnArchos->Get3D1(false)->GetObjectByName("SHIELD");
    }
    if (_isChainHidden == a_hide) {
        spdlog::debug("chains are already {}hidden", a_hide ? "" : "no more ");
    } else if (WeaponIdentify::isBladeOfChaos && data.weaponModel && data.weaponModelL) {
        if (auto chainBoneR = data.weaponModel->GetObjectByName("Chain"); chainBoneR) {
            if (a_hide) chainBoneR->GetFlags() |= RE::NiAVObject::Flag::kHidden;
            else chainBoneR->GetFlags().reset(RE::NiAVObject::Flag::kHidden);
            _isChainHidden = a_hide;
            spdlog::debug("right blade of chaos' chains are {}hidden", a_hide ? "" : "no more ");
        } else {
            spdlog::warn("can't find the chain node of the right-hand blade of chaos!");
        }
        if (auto chainBoneL = data.weaponModelL->GetObjectByName("Chain"); chainBoneL) {
            if (a_hide) chainBoneL->GetFlags() |= RE::NiAVObject::Flag::kHidden;
            else chainBoneL->GetFlags().reset(RE::NiAVObject::Flag::kHidden);
            _isChainHidden = a_hide;
            spdlog::debug("left blade of chaos' chains are {}hidden", a_hide ? "" : "no more ");
        } else {
            spdlog::warn("can't find the chain node of the left-hand blade of chaos!");
        }
    }
}
#pragma endregion
#pragma region DRAUPNIR_SPEAR
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
        if (projectileUpdate.IsTimeToUpdate()) {
        if (data.model && data.proj && data.proj->Get3D() && data.weaponModelCopy && data.model.get() == data.proj->Get3D()) {
        //    const RE::BSFixedString stuckedModelNodeName = "DraupnirProjectile";
        //    auto stuckedModel = data.model->GetObjectByName(stuckedModelNodeName);
        //    stuckedModel->GetFlags() |= RE::NiAVObject::Flag::kHidden;

        //    const RE::BSFixedString stuckedModelLightFadeNodeName = "LightSpellProjectile";
        //    auto stuckedLight = data.model->GetObjectByName(stuckedModelLightFadeNodeName);
        //    stuckedLight->GetFlags() |= RE::NiAVObject::Flag::kHidden;

            auto node = data.model->AsNode();

            auto cloneModel = data.weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            data.replacedProjectileModel.reset(cloneNode);

            if (node) {
                node->AttachChild(data.replacedProjectileModel.get(), false);
                projectileUpdate.Done();
                trailUpdate.RegisterForUpdate(a_delta * 2.f, false);
                spdlog::debug("draupnir model changed!");
            } else spdlog::warn("draupnir node null");
        } else spdlog::warn("proj or proj->Get3D2() null");
    }
    if (Config::DrawTrails) {
        if (trailUpdate.IsTimeToUpdate()) {
            auto bone = data.replacedProjectileModel;
            if (bone) {
                const float intensity = 2.f;
                const auto meshOverride = Config::TrailModelPathDef;
                float length = bone->worldBound.radius;
                ObjectUtil::Capsule capsule;
                ObjectUtil::Node::GetCapsuleParams(bone->AsNode(), capsule);
                float capsuleLength = capsule.a.GetDistance(capsule.b);
                length = length > capsuleLength ? length : capsuleLength;
                float scale = fmax(length, capsule.radius) * 0.01f;
                float tipOffset = length;
                trailData = TrailData(meshOverride, intensity);

                if (Config::UsePrecisionTrails && (Config::IsPrecisionInstalled || APIs::precision || APIs::Request())) {
                    trailUpdate.Done();
                    trailData.transformOverride.additionalRotation = RE::NiMatrix3(0.f, 0.f, -NI_HALF_PI);
                    trailData.transformOverride.scale = bone->worldBound.radius * 0.01f;
                    auto node = RE::NiNode::Create(0);
                    bone->AttachChild(node, false);
                    APIs::precision->AddTrailEffect(
                        node, 
                        RE::PlayerCharacter::GetSingleton()->GetHandle(), 
                        RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                }
            }
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
        const auto draupnirDamage = data.damage;
        mag = draupnirDamage * WeaponIdentify::DamageMult * Config::ThrowingDamageMult;
        float throwChargeDamageMult = std::sqrtf(data.throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        mag *= throwChargeDamageMult;

        if (const auto draupnirBaseEffect = effDraupnir->baseEffect; draupnirBaseEffect && draupnirBaseEffect->data.projectileBase) {
            auto& pbData = draupnirBaseEffect->data.projectileBase->data;
            pbData.speed = Config::ThrowSpeed * 1.1f * std::clamp(throwChargeDamageMult / 2.f, 1.f, 1.25f);
            pbData.force = mag * 0.75f;
        }

        data.gravity = 2.69f;
        data.gravity /= (std::powf(data.throwingChargeDuration + 1.f, 3.f));
        data.gravity = std::max(data.gravity, 0.5f);

        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(AnArchos);
        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {AnArchos->GetAimAngle(), AnArchos->GetAimHeading()};
        RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);

    //    lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(AnArchos, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

        RE::Projectile::Launch(&pHandle, lData);
        data.proj.reset(pHandle.get().get());
        data.throwingChargeDuration = 0.f;

        if (WeaponIdentify::isDraupnirSpear) {
            const auto root = AnArchos->Get3D1(false);
            auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
            auto copyWeaponModel = weapon3D ? weapon3D->Clone() : nullptr;
            copyWeaponModel->RemoveExtraData("BSXFlags");
            copyWeaponModel->GetCollisionObject()->flags &= RE::bhkCollisionObject::Flag::kActive;
            copyWeaponModel->collisionObject.reset();
            auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
            data.weaponModelCopy.reset(copyWeaponModelNode);
            if (data.weaponModelCopy) {
                data.weaponModelCopy->local = RE::NiTransform();
                data.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
            }

            projectileUpdate.RegisterForUpdate(0.0f, false);
        }
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

        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(AnArchos);
        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot{};
        WeaponIdentify::WeaponBone = WeaponIdentify::GetWeaponBone(AnArchos);
        origin = WeaponIdentify::WeaponBone->world.translate;
        AnArchos->Unk_A0(WeaponIdentify::WeaponBone, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(AnArchos, origin, pRot, SpellDraupnirProjL);
    //    lData.weaponSource = WeaponIdentify::DraupnirSpear;
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

        RE::Projectile::Launch(&pHandle, lData);
        if (pHandle.get().get()) {
            MeleeHitProjectileIDs.emplace_back(pHandle.get().get()->formID);
            spdlog::debug("melee thrown draupnir form id: {}", pHandle.get().get()->formID);
        }
        data.proj.reset(pHandle.get().get());

        if (WeaponIdentify::isDraupnirSpear) {
            const auto root = AnArchos->Get3D1(false);
            auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
            auto copyWeaponModel = weapon3D ? weapon3D->Clone() : nullptr;
            copyWeaponModel->RemoveExtraData("BSXFlags");
            copyWeaponModel->GetCollisionObject()->flags &= RE::bhkCollisionObject::Flag::kActive;
            copyWeaponModel->collisionObject.reset();
            auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
            data.weaponModelCopy.reset(copyWeaponModelNode);
            if (data.weaponModelCopy) {
                data.weaponModelCopy->local = RE::NiTransform();
                data.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
            }

            projectileUpdate.RegisterForUpdate(0.0f, false);
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
//    lData.weaponSource = WeaponIdentify::DraupnirSpear;
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

    RE::Projectile::Launch(&pHandle, lData);
    data.proj.reset(pHandle.get().get());

    if (WeaponIdentify::isDraupnirSpear) {
        const auto root = AnArchos->Get3D1(false);
        auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
        auto copyWeaponModel = weapon3D ? weapon3D->Clone() : nullptr;
        copyWeaponModel->RemoveExtraData("BSXFlags");
        copyWeaponModel->GetCollisionObject()->flags &= RE::bhkCollisionObject::Flag::kActive;
        copyWeaponModel->collisionObject.reset();
        auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
        data.weaponModelCopy.reset(copyWeaponModelNode);
        if (data.weaponModelCopy) {
            data.weaponModelCopy->local = RE::NiTransform();
            data.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
        }

        projectileUpdate.RegisterForUpdate(0.0f, false);
    }

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
            if (oldProjectile) oldProjectile->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
            spearHits.erase(spearHits.begin());
        }   spearHits.emplace_back(bone, actor, nullptr);
    }
}
void Draupnir::AddSpearHit(RE::Projectile* a_proj)
{
    if (a_proj) {
        if (spearHits.size() > Config::DraupnirSpearCount) {
            auto& [oldBone, oldTarget, oldProjectile] = spearHits.front();
            if (oldProjectile) oldProjectile->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
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
            TriggerExplosionAtLocation(bone.get(), a_pHandle, target.get());
            target->RemoveExtraArrows3D();
            spdlog::info("{}. draupnir explosion done", currentHitIndex);
        } else if (proj && a_pHandle) {
            TriggerExplosionAtLocation(proj.get(), a_pHandle);
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
    a_proj->GetProjectileRuntimeData().flags |= pFlag::kDestroyed;
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
        kratos->_soundHandle.Stop();
        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
    //    ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeLeviLoop, WeaponIdentify::RHandBone, 5.f, &kratos->_soundHandle);
        a_actor->ApplyArtObject(kratos->VFXeffect.handFlame, 5.f, nullptr, false, false, WeaponIdentify::RHandBone);
        kratos->SetIsChargingThrow(true);
    }
}
void Draupnir::ReplaceStickedProjectileModel(RE::Projectile* a_proj)
{
    if (a_proj && a_proj->Get3D() && data.model) {
        auto projModel = a_proj->Get3D();
        auto projNode = projModel ? projModel->AsFadeNode() : nullptr;
        auto copyModel = data.model->Clone();
        if (projNode && copyModel) {
            const RE::BSFixedString stuckedModelNodeName = "DraupnirSpearBlade";
            auto stuckedModel = copyModel->GetObjectByName(stuckedModelNodeName);
            if (stuckedModel) {
                stuckedModel->flags &= RE::NiAVObject::Flag::kHidden;
                projNode->AttachChild(stuckedModel, false);
                spdlog::debug("draupnir model replaced with stucked model");
            }

            const RE::BSFixedString stuckedModelLightFadeNodeName = "LightSpellProjectile";
            auto stuckedLight = copyModel->GetObjectByName(stuckedModelLightFadeNodeName);
            if (stuckedLight) {
                stuckedLight->AsFadeNode()->flags &= RE::NiAVObject::Flag::kHidden;
                projNode->AttachChild(stuckedLight, false);
                spdlog::debug("added the light model to stucked draupnir model");
            }

            if (data.replacedProjectileModel && data.replacedProjectileModel->parent) {
                data.replacedProjectileModel->parent->DetachChild(data.replacedProjectileModel.get());
                data.replacedProjectileModel.reset();
            }
        } else spdlog::warn("projectile node is null");
    } else spdlog::warn("projectile or projectile model is null");
}
#pragma region Trails
RE::NiColorA Draupnir::TrailData::GetColorByIndex(const uint32_t a_index)
{
    switch ((TrailColor)a_index) {
    case TrailColor::kWhite:
        return WHITE;
    case TrailColor::kIceBlue:
        return ICEBLUE;
    case TrailColor::kSkyBlue:
        return SKYBLUE;
    case TrailColor::kBlue:
        return BLUE;
    case TrailColor::kYellow:
        return YELLOW;
    case TrailColor::kGold:
        return GOLD;
    case TrailColor::kSilver:
        return SILVER;
    default:
        return WHITE;
    }
}
#pragma endregion
#pragma endregion
#pragma region MJOLNIR
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
        callUpdate.Done();
    }
    if (projectileUpdate.IsTimeToUpdate()) {
        if (data.model && data.proj && data.proj->Get3D() && data.weaponModelCopy && data.model.get() == data.proj->Get3D()) {
            const RE::BSFixedString rotatingBoneName = "Cylinder02";
            auto animatedBone = data.model->GetObjectByName(rotatingBoneName);
            auto animatedNode = animatedBone ? animatedBone->AsNode() : nullptr;

            auto cloneModel = data.weaponModelCopy.get()->Clone();
            auto cloneNode = cloneModel ? cloneModel->AsNode() : nullptr;
            data.replacedProjectileModel.reset(cloneNode);

            if (animatedNode) {
                animatedNode->AttachChild(data.replacedProjectileModel.get(), false);
                projectileUpdate.Done();
                trailUpdate.RegisterForUpdate(a_delta * 2.f, false);
                spdlog::debug("mjolnir model changed!");
            } else spdlog::warn("animated node or mjolnirnode null");
        } else spdlog::warn("proj or proj->Get3D2() null");
    }
    if (soundData.arrivingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopArrivingLoopSounds();}
    if (soundData.throwingLoopStopUpdate.IsTimeToUpdate()) {soundData.StopThrowingLoopSounds();}
    if (Config::DrawTrails) {
        AddProjectileTrail(a_delta);
        FadeProjectileTrail(a_delta);
    }
}
tStateM Mjolnir::GetThrowState()     const      {return throwState;}
void Mjolnir::SetThrowState(ThrowState a_state) {throwState = a_state;}
bool Mjolnir::GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor)
{
    bool result = false;
    if (!a_actor) return result;
    if (data.model) {
        data.transformPW = data.model->world;
        data.transformPL = data.model->local;
    }
    if (data.replacedProjectileModel) {
        data.transformW = GetWorldTransform();
        a_point = data.transformW.translate * 70.f;
    //    a_point = data.replacedProjectileModel->worldBound.center;
        result = true;
        spdlog::debug("mjolnir coming from weapon model location.");
    } else spdlog::debug("we can't get leviathan's weapon model!");

    if (GetThrowState() == tStateM::kThrowable) {
        if (auto maceSheathe = a_actor->GetNodeByName("WeaponMace"); maceSheathe) {
            const auto& backSheatheTransform = maceSheathe->world;
            a_point = backSheatheTransform.translate;
            const auto& rightDir = backSheatheTransform.rotate * rightVec3;
            const auto& downDir = backSheatheTransform.rotate * downVec3;
            const auto& backDir = backSheatheTransform.rotate * backVec3;
            data.lastVelocity = (0.5f * rightDir + 0.2f * backDir + 0.3f * downDir) * 2400.f;
            spdlog::debug("mjolnir is coming from your back sheathe");
        }
    } else {
        auto pcPos = a_actor->GetPosition();
        float dist = pcPos.GetDistance(a_point);
        if (dist > 36000.f) {   // ~42000 is limit
            spdlog::info("mjolnir is too far from you! ({} m)", (int)dist / 100);
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
    WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
    if (!WeaponIdentify::RHandBone) {spdlog::error("Mjolnir::Throw - RHandBone is null"); return;}

    trailRemoveUpdate.Done();

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
    {
        const auto MjolnirProjEff = MjolnirThrowSpell->effects[0];
        auto& MjolnirProjEffSetting = MjolnirProjEff->effectItem;
        auto& mag = MjolnirProjEffSetting.magnitude;
        const auto MjolnirDamage = data.damage;
        mag = MjolnirDamage * WeaponIdentify::DamageMult * Config::ThrowingDamageMult;
        data.rotationSpeed = Config::ThrowRotationSpeed;
        bool isPowerThrow; a_actor->GetGraphVariableBool("IsPowerThrowing", isPowerThrow);
        if (isVertical || isPowerThrow) {mag *= 1.5f; data.yAngle = 1.57f; data.rotationSpeed = Config::ThrowRotationSpeed * -1.f;}
        else data.yAngle = 0.f;
        float throwChargeDamageMult = std::sqrtf(data.throwingChargeDuration + 1.f);
        if (throwChargeDamageMult > 2.f) throwChargeDamageMult = 2.f;
        mag *= throwChargeDamageMult;

        if (const auto MjolnirProjBaseEff = MjolnirProjEff->baseEffect; MjolnirProjBaseEff) {
            auto& pbData = MjolnirProjBaseEff->data.projectileBase->data;
            pbData.speed = !justContinue ? Config::ThrowSpeed * 0.8f * std::clamp(throwChargeDamageMult / 2.f, 1.f, 1.25f) : pbData.speed * 0.7f;
            pbData.force = mag * 1.5f;
            pbData.gravity = 3.69f;
        } else spdlog::warn("not found mjolnir throwing effect!");

        if (!justContinue) {
            soundData.PlayThrowingSounds(WeaponIdentify::RHandBone);

            data.gravity = 3.69f;
            data.gravity /= (std::powf(data.throwingChargeDuration + 1.f, 3.f));
            data.gravity = std::max(data.gravity, 0.5f);
        }

        auto kratos = Kratos::GetSingleton();
        //  set the launch data
        auto origin = justContinue ? data.position : WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = RE::Projectile::ProjectileRot(a_actor->GetAimAngle(), a_actor->GetAimHeading());
        if (data.model && (justContinue/* || isHoming*/)) pRot = MathUtil::Algebra::VectorToPitchYaw(data.lastOrientation);//a_actor->Unk_A0(data.model, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, MjolnirThrowSpell);

    //    lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(a_actor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor);
        else
            lData.enchantItem = nullptr;
        _isLastThrowCharged = lData.enchantItem != nullptr;
        //  throw the projectile
        RE::Projectile::Launch(&pHandle, lData);
        data.proj.reset(pHandle.get().get());

        if (WeaponIdentify::isMjolnir) {
            const auto root = a_actor->Get3D1(false);
            auto weapon3D = root ? root->GetObjectByName("WEAPON") : nullptr;
            auto copyWeaponModel = weapon3D ? weapon3D->Clone() : nullptr;
            if (copyWeaponModel) {
                copyWeaponModel->RemoveExtraData("BSX");
                copyWeaponModel->RemoveExtraData("BSXFlags");
                if (copyWeaponModel->GetCollisionObject())
                    copyWeaponModel->GetCollisionObject()->flags.reset(RE::bhkCollisionObject::Flag::kActive);
                copyWeaponModel->collisionObject.reset();
                auto copyWeaponModelNode = copyWeaponModel ? copyWeaponModel->AsNode() : nullptr;
                data.weaponModelCopy.reset(copyWeaponModelNode);
                if (data.weaponModelCopy) {
                    data.weaponModelCopy->local = RE::NiTransform();
                    data.weaponModelCopy->GetFlags() |= RE::NiAVObject::Flag::kAlwaysDraw;
                }
            }
        }

        projectileUpdate.RegisterForUpdate(0.0f, false);

        if (isHoming) {
            if (justContinue) {
                homingMjolnir.proj = homingMjolnir.isBoomerang || !homingMjolnir.targets.empty() ? data.proj : nullptr;
            } else {
                std::vector<RE::Actor *> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(a_actor, Config::HProjectileTargetRange, true);
                homingMjolnir = HomingMjolnir(data.proj.get(), nearCombatTargets, a_actor, Config::HProjectileLoopCount, Config::ReturnHProjectileAfterLoops);
            }
        }
        if (justContinue) return;
        if (Config::IsAdvancedThrowingInstalled && (isThrowAttack || isPowerThrowAttack)) {
            ResetCharge(data.enchMag, data.defaultEnchMag, true);
            WeaponIdentify::skipEquipAnim = true;
            ObjectUtil::Actor::UnEquipItem(a_actor, false, false, true, true, WeaponIdentify::skipEquipAnim, false);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            spdlog::debug("Mjolnir unequipped after throwing");
        } else {
            WeaponIdentify::isMjolnir = false;
            WeaponIdentify::isRelic = false;
        //    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        //    a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
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

        data.throwingChargeDuration = 0.f;
        data.lastHitActors.clear();
        data.lastHitForms.clear();
        if (a_actor->HasSpell(SpellCatchMjolnir)) a_actor->RemoveSpell(SpellCatchMjolnir);
    } else spdlog::info("Mjolnir is not equipped for throwing!");
}
void Mjolnir::Call(const bool a_justDestroy, const bool a_justContinue, std::optional<float> a_delay, RE::Actor* a_actor)
{
    homingMjolnir.proj.reset();
    RE::NiPoint3 startPoint = data.position;
    GetPosition(startPoint, a_actor);
    WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
    if (!isMjolnirCalled)
        arrivingMjolnir = ArrivingWeapon(this, a_actor, &WeaponIdentify::RHandBone, startPoint);
    auto kratos = Kratos::GetSingleton();
    const bool isDelayed = a_delay.has_value() && a_delay != 0.f;
    const bool isDelayedConfig = Config::MjolnirArrivingDelay.has_value() && Config::MjolnirArrivingDelay != 0.f;
    if (!a_justContinue && (isDelayed || (!isDelayed && !isDelayedConfig))) {
        isMjolnirCalled = true;
        isMjolnirArriving = false;
        soundData.PlayCallingHandSounds(WeaponIdentify::RHandBone);
    //    arrivingMjolnir = ArrivingMjolnir(a_actor, WeaponIdentify::RHandBone, pos);

        spdlog::debug("Mjolnir is calling...");
        if (isDelayed) callUpdate.RegisterForUpdate(*a_delay, true);
        else isMjolnirArriving = true;
    }
    if (a_delay.has_value() && a_delay != 0.f && (isDelayed || isDelayedConfig)) {
        spdlog::debug("waiting the delay {} seconds...", a_delay.has_value() ? *a_delay : 0.f);
    } else if (a_actor && data.weap) {
        projectileUpdate.Done();

        trailUpdate.Done();
        trailRemoveUpdate.Done();

        if (data.model) {
            data.transformPW = data.model->world;
            data.transformPL = data.model->local;
        }
        data.model.reset();

        soundData.FadeThrowingLoopSounds(369);

        auto thrownMjolnir =  LastMjolnirProjectile;
        if (!thrownMjolnir)   thrownMjolnir = MjolnirProjectileT;
        if (thrownMjolnir) {
            data.position = thrownMjolnir->data.location;
            auto& runtimeData = thrownMjolnir->GetProjectileRuntimeData();
            auto& pFlags = runtimeData.flags;
            if (!(pFlags & pFlag::kDestroyed)) {
                pFlags |= pFlag::kDestroyed;
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
            RE::Projectile::Launch(&pHandle, lData);
            data.proj.reset(pHandle.get().get());
            projectileUpdate.RegisterForUpdate(0.0f, false);

            if (a_justContinue) {
                arrivingMjolnir.Continue(pHandle.get().get(), startPoint);
        //        arrivingMjolnir.proj.reset(pHandle.get().get());
        //        arrivingMjolnir.linearDistanceFromLastCallPos = startPoint.GetDistance(targetPoint);
            //    arrivingMjolnir.timeToArrive = arrivingLevi.linearDistanceFromLastCallPos / arrivingLevi.speed;
            } else {
                arrivingMjolnir = ArrivingWeapon(arrivingMjolnir, pHandle.get().get(), startPoint);
            //    arrivingMjolnir = ArrivingMjolnir(this, pHandle.get().get(), a_actor, WeaponIdentify::RHandBone, startPoint);
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
        auto& runtimeData = MjolnirProjectileA->GetProjectileRuntimeData();
        runtimeData.flags |= pFlag::kDestroyed;
        if (a_justDestroy) return;
    }
    if (a_actor && !WeaponIdentify::isMjolnir) {
        a_actor->SetGraphVariableBool("bLeviInCatchRange", true);
        if (EffCatchMjolnir && SpellCatchMjolnir && !a_actor->AsMagicTarget()->HasMagicEffect(EffCatchMjolnir)) {
            a_actor->AddSpell(SpellCatchMjolnir);
        }

        SetThrowState(ThrowState::kArrived);

        auto kratos = Kratos::GetSingleton();
        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);

        if (auto handEffect = kratos->VFXeffect.handShock; handEffect) 
            a_actor->ApplyArtObject(handEffect, 1.f, nullptr, false, false, WeaponIdentify::RHandBone);

        soundData.FadeArrivingNearSounds(469);
    //    soundData.FadeArrivingLoopSounds(469);
        soundData.StopArrivingLoopSounds(*g_deltaTimeRealTime * 1200.f);
        soundData.PlayCatchingSounds(WeaponIdentify::RHandBone);

        if (WeaponIdentify::Mjolnir) {
            WeaponIdentify::unequipWhenAnimEnds = false;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
            a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
            kratos->DoKratosAction(Kratos::Action::kWeaponCharge);
            ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::Mjolnir, WeaponIdentify::skipEquipAnim);//, 1U, true, false, false, true);
            ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
            RE::ShakeCamera(0.42f, data.position, 0.5f);
            if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
        } else spdlog::warn("you not have the Mjolnir");

        data.lastHitActors.clear();
        data.lastHitForms.clear();

        if (WeaponIdentify::AnimObjectRBone && WeaponIdentify::AnimObjectRBone->AsNode() && data.replacedProjectileModel && data.replacedProjectileModel->parent) {
            WeaponIdentify::AnimObjectRBone->AsNode()->AttachChild(data.replacedProjectileModel->parent);
            data.replacedProjectileModel->parent->local.translate = RE::NiPoint3();
            data.replacedProjectileModel->parent->local.rotate = data.replacedProjectileModel->parent->local.rotate * RE::NiMatrix3(PI2, 0.f, PI2);
        }
        trailUpdate.Done();
        data.model.reset();
        trailRemoveUpdate.RegisterForUpdate(*g_deltaTime * 2.f, false);

        isMjolnirCalled = false;
        std::jthread delayedCast([=]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            if (GetThrowState() == tStateM::kThrowable) kratos->SetIsCanCallMjolnir(a_actor, false);
        });
        delayedCast.detach();
    }
}
void Mjolnir::AddProjectileTrail(const float a_delta)
{
    if (trailUpdate.IsTimeToUpdate()) {
        auto bone = data.model ? data.model->AsNode() : nullptr;
        if (bone) {
            trailRemoveUpdate.Done();
            DeleteProjectileTrail();
            const bool isCharged = IsCharged(true);
            const float intensity = isCharged ? 3.f : 2.f;
            const auto meshOverride = isCharged ? Config::TrailModelPathShock : Config::TrailModelPathDef;
            float length = bone->worldBound.radius;
            ObjectUtil::Capsule capsule;
            ObjectUtil::Node::GetCapsuleParams(bone->AsNode(), capsule);
            float capsuleLength = capsule.a.GetDistance(capsule.b);
            length = length > capsuleLength ? length : capsuleLength;
            float scale = fmax(length, capsule.radius) * 0.01f;
            trailData = TrailData(meshOverride, intensity);

            if (Config::UsePrecisionTrails && (Config::IsPrecisionInstalled || APIs::precision || APIs::Request())) {
                trailUpdate.Done();
                trailData.transformOverride.additionalRotation = RE::NiMatrix3(0.f, 0.f, -NI_HALF_PI);
                trailData.transformOverride.scale = scale;
                auto node = RE::NiNode::Create(0);
                node->name = "trailParentNode";
                bone->AttachChild(node, false);
                APIs::precision->AddTrailEffect(
                    node, 
                    RE::PlayerCharacter::GetSingleton()->GetHandle(), 
                    RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                    trailData.trailOverride, 
                    trailData.transformOverride);
                if (isCharged) {
                    trailData.trailOverride.meshOverride = Config::TrailModelPathDef;
                    APIs::precision->AddTrailEffect(
                        node, 
                        RE::PlayerCharacter::GetSingleton()->GetHandle(), 
                        RE::PlayerCharacter::GetSingleton()->GetParentCell(), 
                        trailData.trailOverride, 
                        trailData.transformOverride);
                }
            }
        }
    }
}
void Mjolnir::FadeProjectileTrail(const float a_delta)
{
    if (trailRemoveUpdate.IsTimeToUpdate()) {
        if (data.replacedProjectileModel) {
            if (data.proj && data.projState == ProjectileState::kHavok) {
        //        auto& rtData = data.proj->GetProjectileRuntimeData();
                auto velocity = (data.replacedProjectileModel->world.translate - data.replacedProjectileModel->previousWorld.translate) / *g_deltaTime;
                auto speed = velocity.Length();//rtData.linearVelocity.Length();
                spdlog::debug("projectile trail fading... current speed: {}", speed);
                if (speed != 0.f && speed < 669.f) {
                    DeleteProjectileTrail();
                    trailRemoveUpdate.Done();
                }
            } else {
                DeleteProjectileTrail();
                data.replacedProjectileModel.reset();
                trailRemoveUpdate.Done();
            }
        }
    }
}
void Mjolnir::DeleteProjectileTrail()
{
    if (data.replacedProjectileModel) {
        auto trailParentBone = data.replacedProjectileModel->GetObjectByName("trailParentNode");
        data.replacedProjectileModel->DetachChild(trailParentBone);
    //    if (data.replacedProjectileModel->parent)
    //        data.replacedProjectileModel->parent->DetachChild(data.replacedProjectileModel.get());
        if (WeaponIdentify::AnimObjectRBone) {
            WeaponIdentify::AnimObjectRBone->AsNode()->DetachChild(data.replacedProjectileModel->parent);
        }
        spdlog::debug("projectile trail deleted");
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
        WeaponIdentify::WeaponBone = WeaponIdentify::GetWeaponBone(AnArchos);
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
bool Mjolnir::IsArriving(RE::Projectile* a_proj) const {return a_proj == arrivingMjolnir.proj.get();}
bool Mjolnir::IsHoming(RE::Projectile* a_proj) const {return a_proj == homingMjolnir.proj.get();}
void Mjolnir::StartChargingThrow(RE::Actor* a_actor)
{
    if (auto kratos = Kratos::GetSingleton(); a_actor && kratos) {
        kratos->_soundHandle.Stop();
        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
        ObjectUtil::Sound::PlaySound(kratos->soundEffect.chargeMjolnir, kratos->_soundHandle, WeaponIdentify::RHandBone, 5.f);
        a_actor->ApplyArtObject(kratos->VFXeffect.handShock, 5.f, nullptr, false, false, WeaponIdentify::RHandBone);
        kratos->SetIsChargingThrow(true);
    }
}
RE::NiTransform Mjolnir::GetWorldTransform()
{
    if (data.replacedProjectileModel) {
        data.transformW = ObjectUtil::Node::GetHavokBHKRigidBodyWorldTransform(data.replacedProjectileModel.get());
        return data.transformW;
    } else return data.transformPW;
    return {};
}
RE::NiTransform Mjolnir::GetLocalTransform()
{
    RE::NiTransform ret;
    if (data.replacedProjectileModel) {
        data.transformL = data.replacedProjectileModel->local;
        ret = data.transformL;
    } else ret = data.transformPL;
    return ret;
}
#pragma region Trails
RE::NiColorA Mjolnir::TrailData::GetColorByIndex(const uint32_t a_index)
{
    switch ((TrailColor)a_index) {
    case TrailColor::kWhite:
        return WHITE;
    case TrailColor::kIceBlue:
        return ICEBLUE;
    case TrailColor::kSkyBlue:
        return SKYBLUE;
    case TrailColor::kBlue:
        return BLUE;
    case TrailColor::kYellow:
        return YELLOW;
    case TrailColor::kGold:
        return GOLD;
    case TrailColor::kSilver:
        return SILVER;
    default:
        return WHITE;
    }
}
#pragma endregion
#pragma region Arriving
void Mjolnir::ArrivingWeapon::UpdateRotation()
{
    if (parent->data.replacedProjectileModel && parent->data.replacedProjectileModel->parent && WeaponIdentify::GetWeaponBone()) {
        auto& replacedPMParent = parent->data.replacedProjectileModel->parent;
        auto& localRotation = replacedPMParent->local.rotate;
        RE::NiMatrix3 targetLocalRotation;
        if (replacedPMParent->parent) {
            targetLocalRotation = replacedPMParent->parent->world.rotate.Transpose() * WeaponIdentify::WeaponBone->world.rotate;
        } else {
            targetLocalRotation = WeaponIdentify::WeaponBone->world.rotate;
        }
        const float angleZ = livingTime * rotationSpeed;
        MathUtil::Angle::NormalAbsoluteAngle(angleZ);
        arrivalSpin = (int)(std::floor(1 + (livingTime + remainingTimeToArrive) * rotationSpeed / TWO_PI)) * TWO_PI;
        const float blendZ = std::lerp(angleZ, arrivalSpin, tReal);
        const auto& startRotationC = startRotation;
        auto desiredLocalRotation = MathUtil::Algebra::InterpolateRotation(startRotationC, targetLocalRotation, tReal);
        desiredLocalRotation = desiredLocalRotation * RE::NiMatrix3(0.f, blendZ, 0.f);
        constexpr float smoothTime = 0.0069f;
        const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
        MathUtil::Algebra::InterpolateRotation(localRotation, desiredLocalRotation, alpha);
    //    spdlog::debug("target angle: {} calculated target angle: {} blended angle: {} tau: {}", angleZ, arrivalSpin, blendZ, tReal);
    }
//    mjolnirAngle = mjolnir->data.lastEulerAngles;
//    mjolnirAngle.z = atan2(desiredDir.x, desiredDir.y);
//    mjolnir->data.lastEulerAngles = mjolnirAngle;
}
void Mjolnir::ArrivingWeapon::UpdateAI(RE::NiPoint3& a_outVel)
{
    float height = position.z - caller->GetPosition().z;
    if (!Config::DontDamageWhileArrive && tReal < 0.80f) {
        if (auto aTarget = GetNextTarget(position); aTarget) {
            auto targetPos = aTarget->GetPosition() + (aTarget->GetBoundMax() + aTarget->GetBoundMin()) * 0.75f;
            auto targetDir = (targetPos - position);
            targetDir.Unitize();
            height = position.z - aTarget->GetPosition().z;
            a_outVel = targetDir * speed;

            if (parent->data.replacedProjectileModel) {
                auto& replacedPMParent = parent->data.replacedProjectileModel->parent;
                auto& localRotation = replacedPMParent->local.rotate;
                RE::NiMatrix3 targetLocalRotation;
                MathUtil::Algebra::SetRotationMatrix(targetLocalRotation, -targetDir.x, targetDir.y, targetDir.z);
                constexpr float smoothTime = 0.0069f;
                const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
                MathUtil::Algebra::InterpolateRotation(localRotation, targetLocalRotation, alpha);
                auto& mjolnirAngle = parent->data.lastEulerAngles;
                mjolnirAngle.x = asin(desiredDir.z);
                mjolnirAngle.z = atan2(desiredDir.x, desiredDir.y);
                if (mjolnirAngle.z < 0.0) {
                    mjolnirAngle.z += PI;
                }
                if (desiredDir.x < 0.0) {
                    mjolnirAngle.z += PI;
                }
            }
        }
    }
}
void Mjolnir::ArrivingWeapon::UpdateArrivingDirection(const bool a_initial)
{
    if (RE::PlayerCamera::GetSingleton()->IsInFirstPerson()) {

    } else if (caller && parent && callerBreastBone) {
        if (parent->GetThrowState() == ThrowState::kThrowable || isCatchable) {

        } else if (a_initial || linearDistance > 100.f) {
            RE::NiPoint3  spineForwardDir = callerBreastBone->world.rotate * RE::NiPoint3(frontVec3);
            spineForwardDir.z = 0.f;  //  ignore vertical direction
            spineForwardDir.Unitize();

            RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
            linearDir2D.Unitize();

            float dot = spineForwardDir.Dot(linearDir2D);
            float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;

            arrivingRelativeAngleZ = atan2(det, dot);  //  angle between spine forward direction and arriving weapon direction
            arrivingRelativeAngleZ = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngleZ);      //  normalize angle to [0, 2PI]

            float snapStrength = arrivingRelativeAngleSnapStrength;
            if (a_initial) snapStrength = 1.f;
            else if ((arrivingRelativeAngleZ < PI4 || arrivingRelativeAngleZ > ONEANDHALF_PI) && snapStrength < 0.69f) snapStrength += 0.69f;
            snapStrength = std::min(snapStrength, 1.f);
            if (snapStrength > 0.f) {
                arrivingRelativeAngleZ = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngleZ, arrivingDirections, snapStrength);    //  for helping to the blender generator
            }

            if (!a_initial) {
                float previousAngle; caller->GetGraphVariableFloat("fArrivingWeaponDirection", previousAngle); previousAngle *= TWO_PI;
                const float delta = MathUtil::Angle::NormalizeSignedAngle(arrivingRelativeAngleZ - previousAngle);
                constexpr float smoothTime = 0.369f;
                const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
                float smoothedArrivingRelativeAngle = previousAngle;
                smoothedArrivingRelativeAngle += delta * alpha;
                arrivingRelativeAngleZ = smoothedArrivingRelativeAngle;
                
            }
            arrivingRelativeAngleZ /= TWO_PI;
        }
        caller->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngleZ);
    }
}
void Mjolnir::ArrivingWeapon::UpdateArrivingRoute()
{
    RE::NiMatrix3 handRot   = callerHandBone->world.rotate;
    const float alphaHandRot = 1.f - std::exp(-*g_deltaTimeRealTime / 0.169f);
    RE::NiMatrix3 smoothedHandRot = 
        MathUtil::Algebra::QuaternionToMatrix(MathUtil::Algebra::Slerp(
            MathUtil::Algebra::MatrixToQuaternion(callerHandBone->previousWorld.rotate),
            MathUtil::Algebra::MatrixToQuaternion(callerHandBone->world.rotate), alphaHandRot));
    RE::NiPoint3 palmDir    = smoothedHandRot * RE::NiPoint3(backVec3);
    RE::NiPoint3 handForward= smoothedHandRot * RE::NiPoint3(upVec3);
    palmDir.Unitize();
    handForward.Unitize();

    const float handSideOffsetMult = MathUtil::Algebra::ParabolicClamp(arrivingRelativeAngleZ, 0.f, 0.25f);

    RE::NiPoint3 handVelocity = (callerHandBone->world.translate - callerHandBone->previousWorld.translate) / *g_deltaTimeRealTime;
    const float predictionTime = std::clamp(*g_deltaTimeRealTime, 0.0f, 0.1f);
    RE::NiPoint3 predictedHandPos = handPosition + handVelocity * predictionTime;
    bezierControlPoints[3] = predictedHandPos;
    bezierControlPoints[2] = predictedHandPos + (float)IsInCallingAnimation() * (palmDir * (linearDistanceFromLastCallPos * 0.33f + 20.f) + handForward * (linearDistanceFromLastCallPos * handSideOffsetMult + 10.f));

    RE::NiPoint3& p0 = bezierControlPoints[0];
    RE::NiPoint3& p1 = bezierControlPoints[1];
    RE::NiPoint3& p3 = bezierControlPoints[3];
    RE::NiPoint3& p2 = bezierControlPoints[2];

    arrivingRoute = MathUtil::Algebra::CalculateAndMeasureBezier(
        p0, p1, p2, p3,
        routeResolution);
    float bestDist2 = FLT_MAX;

    for (int i = 0; i < arrivingRoute.samples.size() - 1; i++) {
        const float d2 = (arrivingRoute.samples[i].point - position).SqrLength();
        if (d2 < bestDist2)
        {
            bestDist2 = d2;
            arrivingRouteClosestIndex = i;
        }
    }

    closestSample = arrivingRoute.samples[arrivingRouteClosestIndex];
    const float t = closestSample.t;

    const float lookAheadDistance = std::clamp(std::abs(speed) * 0.15f, 50.f, 200.f);
    const float targetDistance = closestSample.distanceFromStart + lookAheadDistance;

    int targetIndex = arrivingRouteClosestIndex;
    while (targetIndex + 1 <
        static_cast<int>(arrivingRoute.samples.size()) &&
        arrivingRoute.samples[targetIndex].distanceFromStart < targetDistance)
    {
        targetIndex++;
    }

    bezierDir = arrivingRoute.samples[targetIndex].point - position;
    bezierDir.Unitize();
}
void Mjolnir::ArrivingWeapon::Update(const float a_delta)
{
    if (!parent) return;

    model = parent->data.model;
    if (!model) return;
    
    callerHandBone = GetCallerHandBone();
    if (!callerHandBone) return;
    handPosition = callerHandBone->world.translate;

    if (!proj) return;
    auto& rtData = proj->GetProjectileRuntimeData();
    auto& vel = rtData.linearVelocity;

    currentDir = vel;
    currentDir.Unitize();

    position = proj->data.location;

    linearDistance = handPosition.GetDistance(position);
    linearArrivingDir = handPosition - position;
    linearArrivingDir.Unitize();

    isCatchable = (linearDistance <= Config::CatchingTreshold) || (linearDistance <= (*g_deltaTime * vel.Length()));

    if (parent->MjolnirProjectileA != proj.get()) {  //  first frame of the arriving projectile
        parent->MjolnirProjectileA = proj.get();

        if (!isCatchable) {
            parent->soundData.PlayArrivingStartSounds(model.get());
            parent->soundData.PlayArrivingLoopSounds(model.get());
        }
    }
    if (isCatchable) {
        if (parent->GetThrowState() == ThrowState::kArriving) parent->SetThrowState(ThrowState::kArrived);
        parent->Catch();
        spdlog::debug("mjolnir proj catched");
    }
    if (false && startRotation == RE::NiMatrix3()) {
        if (parent->data.replacedProjectileModel && parent->data.replacedProjectileModel->parent) {
            model.get()->world = parent->data.transformPW;
            model.get()->local = parent->data.transformPL;
            auto& replacedPMParent = parent->data.replacedProjectileModel->parent;
            auto parentWorldInverse = replacedPMParent->world.Invert();
            auto previousWorld = parent->data.transformW;
            auto& localRotation = replacedPMParent->local.rotate;
            auto& localPosition = replacedPMParent->local.translate;
            if (replacedPMParent->parent) {
                localRotation = parentWorldInverse.rotate * previousWorld.rotate;
            //    localPosition = parentWorldInverse.rotate * (previousWorld.translate - localPosition);
            } else {
                localRotation = previousWorld.rotate;
            //    localPosition = previousWorld.translate;
            }
            startRotation = localRotation;
            spdlog::debug("start rotation initiated");
        }
    }

    livingTime = GetLivingTime();
    UpdateArrivingRoute();

    const float minArrivalTime = *g_deltaTime * 2.f;
    float remainingRouteLength = arrivingRoute.arcLength - closestSample.distanceFromStart;
    remainingRouteLength = std::max(remainingRouteLength, linearDistance);
    remainingTimeToArrive = std::max(timeToArrive - livingTime, minArrivalTime);
    const float requiredAverageSpeed = remainingRouteLength / remainingTimeToArrive;

    if (requiredAverageSpeed < Config::MinArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MinArrivalSpeed, minArrivalTime);
    else if (requiredAverageSpeed > Config::MaxArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MaxArrivalSpeed, minArrivalTime);

    const float desiredAcceleration = 2.f * (remainingRouteLength - speed * remainingTimeToArrive) / (remainingTimeToArrive * remainingTimeToArrive);
    speed += desiredAcceleration * *g_deltaTimeRealTime;
    speed = std::max(speed, Config::MinArrivalSpeed);

    remainingTimeToArrive = remainingRouteLength / speed;
    constexpr float nearArriveTimeThreshold = 0.369f;
    isNear = remainingTimeToArrive < (nearArriveTimeThreshold < timeToArrive ? nearArriveTimeThreshold : timeToArrive * 0.8f);
    constexpr float almostArrivedTimeThreshold = 0.169f;
    isAlmostArrived = remainingTimeToArrive < (almostArrivedTimeThreshold < timeToArrive ? almostArrivedTimeThreshold : timeToArrive * 0.5f);
    if (isAlmostArrived || isCatchable) {
        uint16_t fadeDuration = (uint16_t)(almostArrivedTimeThreshold * 1000.f) + 200u;
        parent->soundData.FadeArrivingStartSounds(fadeDuration);
        parent->soundData.FadeArrivingLoopSounds(fadeDuration);
        parent->soundData.FadeCallingHandSounds(fadeDuration);
    } else if (isNear) {
        parent->soundData.PlayArrivingNearSounds(model.get());
    }
//    spdlog::debug(
//        "AFTER ACCEL speed={:.1f}, accel={:.1f}, L={:.1f}, T={:.3f}",
//        speed,
//        desiredAcceleration,
//        remainingRouteLength,
//        remainingTimeToArrive);
//    spdlog::debug(
//        "closest={} target={} closestT={:.3f} targetT={:.3f} "
//        "closestL={:.1f} targetL={:.1f} routeL={:.1f}",
//        arrivingRouteClosestIndex,
//        targetIndex,
//        closestSample.t,
//        arrivingRoute.samples[targetIndex].t,
//        closestSample.distanceFromStart,
//        arrivingRoute.samples[targetIndex].distanceFromStart,
//        arrivingRoute.arcLength
//    );

//    tReal = 1.f - remainingTimeToArrive / almostArrivedTimeThreshold;//timeToArrive;
    tReal = timeToArrive / (timeToArrive + 2.f * (remainingTimeToArrive - 0.05f));
    tReal = std::pow(tReal, 2.f);
    tReal = std::clamp(tReal, 0.f, 1.f);
//    tReal = tReal * tReal * (3.f - 2.f * tReal);

    const float smoothTime = std::clamp(1.f - tReal, 0.01f, 0.169f);
    const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
    smoothedDesiredVelocity += (bezierDir * speed - smoothedDesiredVelocity) * alpha;
    desiredDir = smoothedDesiredVelocity;
    desiredDir.Unitize();
    const bool doBlend = 
        parent->data.projState == ProjectileState::kNone ||
        parent->data.projState == ProjectileState::kLaunched;
    vel = MathUtil::Angle::BlendVectors(doBlend ? parent->data.lastVelocity : (linearArrivingDir * speed), desiredDir * speed, livingTime / 0.2f);
    if (vel.z < 0.f) {
        constexpr float minHeight = -40.f;
        constexpr float dampingRange = 69.f;
        const float height = position.z - handPosition.z;
        const float dampFactor = std::clamp((height - minHeight) / dampingRange, 0.f, 1.f);
        vel.z *= dampFactor;
    }

    UpdateRotation();
    UpdateAI(vel);
    UpdateArrivingDirection();
}
#pragma endregion
#pragma region Sounds
void Mjolnir::SoundData::StopAllSounds()
{
    if (CallStartSH.IsPlaying()) CallStartSH.Stop();
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.Stop();
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.Stop();
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.Stop();
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.Stop();
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.Stop();
    if (ThrowingStartSH.IsPlaying()) ThrowingStartSH.Stop();
    if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.Stop();
    if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.Stop();
    if (CatchSH.IsPlaying()) CatchSH.Stop();
}
void Mjolnir::SoundData::PauseAllSounds()
{
//    if (CallStartSH.IsPlaying() && GetState(SoundName::kCallStart) == State::kTriggered) {CallStartSH.Pause(); soundState[SoundName::kCallStart] = State::kPaused;}
//    if (ArrivingStartSH.IsPlaying() && GetState(SoundName::kArrivingStart) == State::kTriggered) {ArrivingStartSH.Pause(); soundState[SoundName::kArrivingStart] = State::kPaused;}
    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingNearSH.IsPlaying() && GetState(SoundName::kArrivingNear) == State::kTriggered) {ArrivingNearSH.Pause(); soundState[SoundName::kArrivingNear] = State::kPaused;}
//    if (ThrowingStartSH.IsPlaying() && GetState(SoundName::kThrowingStart) == State::kTriggered) {ThrowingStartSH.Pause(); soundState[SoundName::kThrowingStart] = State::kPaused;}
    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    else if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (CatchSH.IsPlaying() && GetState(SoundName::kCatch) == State::kTriggered) {CatchSH.Pause(); soundState[SoundName::kCatch] = State::kPaused;}

//    if (CallStartSH.IsPlaying() && GetState(SoundName::kCallStart) == State::kTriggered) {CallStartSH.Pause(); soundState[SoundName::kCallStart] = State::kPaused;}
//    if (ArrivingStartSH.IsPlaying() && GetState(SoundName::kArrivingStart) == State::kTriggered) {ArrivingStartSH.Pause(); soundState[SoundName::kArrivingStart] = State::kPaused;}
//    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop0SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop1SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {ArrivingLoop2SH.Pause(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
//    if (ArrivingNearSH.IsPlaying() && GetState(SoundName::kArrivingNear) == State::kTriggered) {ArrivingNearSH.Pause(); soundState[SoundName::kArrivingNear] = State::kPaused;}
//    if (ThrowingStartSH.IsPlaying() && GetState(SoundName::kThrowingStart) == State::kTriggered) {ThrowingStartSH.Pause(); soundState[SoundName::kThrowingStart] = State::kPaused;}
//    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {ThrowingLoop0SH.Pause(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {ThrowingLoop1SH.Pause(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
//    if (CatchSH.IsPlaying() && GetState(SoundName::kCatch) == State::kTriggered) {CatchSH.Pause(); soundState[SoundName::kCatch] = State::kPaused;}
}
void Mjolnir::SoundData::ContinueAllSounds()
{
    if (GetState(SoundName::kCallStart) == State::kPaused) {CallStartSH.Play(); soundState[SoundName::kCallStart] = State::kPlaying;}
    if (GetState(SoundName::kArrivingStart) == State::kPaused) {ArrivingStartSH.Play(); soundState[SoundName::kArrivingStart] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop0SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop1SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {ArrivingLoop2SH.Play(); soundState[SoundName::kArrivingLoop] = State::kPlaying;}
    if (GetState(SoundName::kArrivingNear) == State::kPaused) {ArrivingNearSH.Play(); soundState[SoundName::kArrivingNear] = State::kPlaying;}
    if (GetState(SoundName::kThrowingStart) == State::kPaused) {ThrowingStartSH.Play(); soundState[SoundName::kThrowingStart] = State::kPlaying;}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {ThrowingLoop0SH.Play(); soundState[SoundName::kThrowingLoop] = State::kPlaying;}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {ThrowingLoop1SH.Play(); soundState[SoundName::kThrowingLoop] = State::kPlaying;}
    if (GetState(SoundName::kCatch) == State::kPaused) {CatchSH.Play(); soundState[SoundName::kCatch] = State::kPlaying;}
}
void Mjolnir::SoundData::PauseAllLoopingSounds()
{
    if (ArrivingLoop0SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop1SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    else if (ArrivingLoop2SH.IsPlaying() && GetState(SoundName::kArrivingLoop) == State::kTriggered) {StopArrivingLoopSounds(); soundState[SoundName::kArrivingLoop] = State::kPaused;}
    if (ThrowingLoop0SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
    else if (ThrowingLoop1SH.IsPlaying() && GetState(SoundName::kThrowingLoop) == State::kTriggered) {StopThrowingLoopSounds(); soundState[SoundName::kThrowingLoop] = State::kPaused;}
}
void Mjolnir::SoundData::ContinueAllLoopingSounds()
{
    if (GetState(SoundName::kArrivingLoop) == State::kPaused) {PlayArrivingLoopSounds(weaponData->model.get());}// soundState[SoundName::kArrivingLoop] = State::kPlaying; spdlog::debug("ArrivingLoop sounds continuing...");}
    if (GetState(SoundName::kThrowingLoop) == State::kPaused) {PlayThrowingLoopSounds(weaponData->model.get());}// soundState[SoundName::kThrowingLoop] = State::kPlaying; spdlog::debug("ThrowingLoop sounds continuing...");}
}
void Mjolnir::SoundData::FadeAllSounds(const uint16_t a_durationMS)
{
    if (CallStartSH.IsPlaying()) CallStartSH.FadeOutAndRelease(a_durationMS);
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.FadeOutAndRelease(a_durationMS);
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.FadeOutAndRelease(a_durationMS);
    if (CatchSH.IsPlaying()) CatchSH.FadeOutAndRelease(a_durationMS);
}
void Mjolnir::SoundData::PlayCallingHandSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.fingerSnap; soundEffect)
        ObjectUtil::Sound::PlaySound(soundEffect, CallStartSH, a_source, 5.f);
}
void Mjolnir::SoundData::PlayArrivingStartSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.arrivingLeviStart; soundEffect && IsSoundValid(SoundName::kArrivingStart)) {
        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingStartSH, a_source, 5.f);
        soundState[SoundName::kArrivingStart] = State::kTriggered;
    } else {
        if (ArrivingStartSH.IsPlaying())
            ArrivingStartSH.SetObjectToFollow(a_source);
    }
}
void Mjolnir::SoundData::PlayArrivingLoopSounds(RE::NiAVObject* a_source)
{
    if (IsSoundValid(SoundName::kArrivingLoop)) {
        if (auto soundEffect = kratos->soundEffect.arrivingLeviLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kArrivingLoop] = State::kTriggered;
        }
        if (auto soundEffect = kratos->soundEffect.arrivingLeviLoop1; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop1SH, a_source, 0.5f);
            soundState[SoundName::kArrivingLoop] = State::kTriggered;
        }
    //    if (auto soundEffect = kratos->soundEffect.arrivingLeviLoop2; soundEffect) {
    //        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingLoop2SH, a_source, 0.5f);
    //        soundState[SoundName::kArrivingLoop] = State::kTriggered;
    //    }
    } else {
        spdlog::debug("updating the following node of the arriving loop sounds.");
        if (ArrivingLoop0SH.IsPlaying()) {
            ArrivingLoop0SH.SetObjectToFollow(a_source);
            ArrivingLoop0SH.Play();
        } if (ArrivingLoop1SH.IsPlaying()) {
            ArrivingLoop1SH.SetObjectToFollow(a_source);
            ArrivingLoop1SH.Play();
        }// if (ArrivingLoop2SH.IsPlaying()) {
    //        ArrivingLoop2SH.SetObjectToFollow(a_source);
    //        ArrivingLoop2SH.Play();
    //    }
    }
}
void Mjolnir::SoundData::PlayArrivingNearSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.arrivingLeviNear; soundEffect && IsSoundValid(SoundName::kArrivingNear) ) {
        ObjectUtil::Sound::PlaySound(soundEffect, ArrivingNearSH, a_source, 5.f);
        soundState[SoundName::kArrivingNear] = State::kTriggered;
    } else {
        if (ArrivingNearSH.IsPlaying())
            ArrivingNearSH.SetObjectToFollow(a_source);
    }
}
void Mjolnir::SoundData::PlayCatchingSounds(RE::NiAVObject* a_source)
{
    if (auto soundEffect = kratos->soundEffect.catchLevi; soundEffect)
        ObjectUtil::Sound::PlaySound(soundEffect, CatchSH, a_source, 5.f);
}
void Mjolnir::SoundData::PlayThrowingSounds(RE::NiAVObject* a_source)
{
//    if (auto soundEffect = kratos->soundEffect.throwingLeviNear; soundEffect)
//        ThrowingStartSH = ObjectUtil::Sound::PlaySound(soundEffect, a_source, 5.f);
}
void Mjolnir::SoundData::PlayThrowingLoopSounds(RE::NiAVObject* a_source)
{
    if (IsSoundValid(SoundName::kThrowingLoop)) {
        if (auto soundEffect = kratos->soundEffect.throwingLeviLoop0; soundEffect) {
            ObjectUtil::Sound::PlaySound(soundEffect, ThrowingLoop0SH, a_source, 0.5f);
            soundState[SoundName::kThrowingLoop] = State::kTriggered;
        }
    //    if (auto soundEffect = kratos->soundEffect.throwingLeviLoop1; soundEffect) {
    //        ObjectUtil::Sound::PlaySound(soundEffect, ThrowingLoop1SH, a_source, 0.5f);
    //        soundState[SoundName::kThrowingLoop] = State::kTriggered;
    //    }
    } else {
        spdlog::debug("updating the following node of the throwing loop sounds.");
        if (ThrowingLoop0SH.IsPlaying()) {
            ThrowingLoop0SH.SetObjectToFollow(a_source);
            ThrowingLoop0SH.Play();
        }// if (ThrowingLoop1SH.IsPlaying()) {
    //        ThrowingLoop1SH.SetObjectToFollow(a_source);
    //        ThrowingLoop1SH.Play();
    //    }
    }
}
void Mjolnir::SoundData::FadeCallingHandSounds(const uint16_t a_durationMS)
{
    if (CallStartSH.IsPlaying()) CallStartSH.FadeOutAndRelease(a_durationMS);
}
void Mjolnir::SoundData::FadeArrivingStartSounds(const uint16_t a_durationMS)
{
    
    soundState[SoundName::kArrivingStart] = State::kFading;
    if (ArrivingStartSH.IsPlaying()) ArrivingStartSH.FadeOutAndRelease(a_durationMS);
    else soundState[SoundName::kArrivingStart] = State::kStopped;
}
void Mjolnir::SoundData::FadeArrivingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kArrivingLoop] = State::kFading;
    if (ArrivingLoop0SH.IsPlaying()) ArrivingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}
    if (ArrivingLoop1SH.IsPlaying()) ArrivingLoop1SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}
    if (ArrivingLoop2SH.IsPlaying()) ArrivingLoop2SH.FadeOutAndRelease(a_durationMS);
    else {StopArrivingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kArrivingLoop] = State::kStopped;}

    StopArrivingLoopSounds(a_durationMS);
}
void Mjolnir::SoundData::FadeArrivingNearSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kArrivingNear] = State::kFading;
    if (ArrivingNearSH.IsPlaying()) ArrivingNearSH.FadeOutAndRelease(a_durationMS);
    else soundState[SoundName::kArrivingNear] = State::kStopped;
}
void Mjolnir::SoundData::FadeThrowingLoopSounds(const uint16_t a_durationMS)
{
    soundState[SoundName::kThrowingLoop] = State::kFading;
    if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kStopped;}
    if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.FadeOutAndRelease(a_durationMS);
    else {StopThrowingLoopSounds(*g_deltaTimeRealTime * 2000.f); soundState[SoundName::kThrowingLoop] = State::kStopped;}

    StopThrowingLoopSounds(a_durationMS);
}
void Mjolnir::SoundData::StopArrivingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ArrivingLoop0SH.IsPlaying()) {ArrivingLoop0SH.Stop(); spdlog::debug("arriving loop sound 1 stopped.");}
        else {spdlog::debug("arriving loop sound 1 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        if (ArrivingLoop1SH.IsPlaying()) {ArrivingLoop1SH.Stop(); spdlog::debug("arriving loop sound 2 stopped.");}
        else {spdlog::debug("arriving loop sound 2 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        if (ArrivingLoop2SH.IsPlaying()) {ArrivingLoop2SH.Stop(); spdlog::debug("arriving loop sound 3 stopped.");}
        else {spdlog::debug("arriving loop sound 3 is already stopped."); soundState[SoundName::kArrivingLoop] = State::kStopped;}
        soundState[SoundName::kArrivingLoop] = State::kStopped;
    } else {
        arrivingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
void Mjolnir::SoundData::StopThrowingLoopSounds(const uint16_t a_delayMS)
{
    if (a_delayMS == 0) {
        if (ThrowingLoop0SH.IsPlaying()) ThrowingLoop0SH.Stop();
        else {spdlog::debug("throwing loop sound is already stopped."); soundState[SoundName::kThrowingLoop] = State::kStopped;}
        if (ThrowingLoop1SH.IsPlaying()) ThrowingLoop1SH.Stop();
        else {spdlog::debug("throwing loop sound is already stopped."); soundState[SoundName::kThrowingLoop] = State::kStopped;}
        soundState[SoundName::kThrowingLoop] = State::kStopped;
    } else {
        throwingLoopStopUpdate.RegisterForUpdate(((float)a_delayMS) / 1000.f);
    }
}
#pragma endregion
#pragma endregion
#ifdef TRIDENT
#pragma region TRIDENT
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
        const float mag = TridentDamage * WeaponIdentify::DamageMult * Config::ThrowingDamageMult;
        const auto effTrident = SpellTridentProjL->effects[0];
        auto& leviProjEffSetting = effTrident->effectItem;
        leviProjEffSetting.magnitude = mag / 2.f;
        data.damage = mag;
        if (data.explosionThrow) {
            data.explosionThrow->data.damage = data.damage;
        }

        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
        auto origin = justContinue ? data.position : WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = RE::Projectile::ProjectileRot(a_actor->GetAimAngle(), a_actor->GetAimHeading());
        if (justContinue && data.model) a_actor->Unk_A0(data.model, pRot.x, pRot.z, origin);
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, SpellTridentProjL);
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
    //    lData.weaponSource = WeaponIdentify::Trident;
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

        data.throwingChargeDuration = 0.f;
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
//    lData.weaponSource = WeaponIdentify::DraupnirSpear;
    if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(AnArchos) > 0.f)
        lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(AnArchos);

    RE::Projectile::Launch(&pHandle, lData);

    nextLaunchTime = AsyncUtil::GameTime::GetEngineTime() + nextLaunchDelay;
}
void Trident::Call(const float a_damage, const float a_force, RE::Actor* a_actor, const bool a_justEquip)
{
    if (WeaponIdentify::Trident && !WeaponIdentify::isTrident) {
        ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::Trident, true, 1U, true, false, false, false, nullptr, a_justEquip);
        ObjectUtil::Actor::ResetEquipAnimationAfter(100, a_actor);
        if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
    }

    auto kratos = Kratos::GetSingleton();
    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, a_actor);

    if (!a_justEquip) {
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "weaponDraw");           //  loads the 3d model of the weapon
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "WeapEquip_Out");
        ObjectUtil::Actor::SendAnimationEvent(a_actor, "WeapEquip_OutMoving");
        WeaponIdentify::RHandBone = WeaponIdentify::GetRhandBone(a_actor);
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
            if (!(pFlags & pFlag::kDestroyed)) {
                pFlags |= pFlag::kDestroyed;
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