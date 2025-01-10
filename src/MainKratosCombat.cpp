#include "MainKratosCombat.h"
//#include "API/TrueDirectionalMovementAPI.h"

using namespace Util;
using tState = LeviathanAxe::ThrowState;

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
//WEAPON IDENTIFICATION
void WeaponIdentify::Initialize(RE::Actor* a_actor)
{
    RE::EnchantmentItem* formEnch = nullptr;
    RE::EnchantmentItem* ench = nullptr;
    if (a_actor) {
        auto kratos = Kratos::GetSingleton();
        auto invChanges = a_actor->GetInventoryChanges();
        auto entries = invChanges->entryList;
        spdlog::info("Kratos's special weapons initializing...");
        for (auto entry : *entries) {
            if (entry && entry->object) {
                if (entry->object->IsWeapon()) {
                    if (entry->object->formID == kratos->gLeviathanAxeFormID->value) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::LeviathanAxeKWD);
                            spdlog::info("{} is your Leviathan Axe", weap->GetName());
                        }
                    } else if (entry->object->formID == kratos->gBladeOfChaosFormID->value) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            spdlog::info("{} is your Blade of Chaos", weap->GetName());
                        }
                    } else if (entry->object->formID == kratos->gDraupnirSpearFormID->value) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::DraupnirSpearKWD);
                            spdlog::info("{} is your Draupnir Spear", weap->GetName());
                        }
                    } else if (entry->object->formID == kratos->gBladeOfOlympusFormID->value) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfOlympusKWD);
                            spdlog::info("{} is your Blade of Olympus", weap->GetName());
                        }
                    }
                } else if (entry->object->IsArmor()) {
                    if (entry->object->formID == kratos->gGuardianShieldFormID->value) {
                        if (auto shield = entry->object->As<RE::TESObjectARMO>(); shield) {
                            shield->RemoveKeywords(Config::SpecialKWDs);
                            shield->AddKeyword(Config::GuardianShieldKWD);
                            spdlog::info("{} is your Guardian Shield", shield->GetName());
                        }
                    }
                }
            }
        } spdlog::info("Kratos's special weapons initialized.");
    }
}
void WeaponIdentify::WeaponIdentifier(RE::Actor* a_actor, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectARMO* a_shield)
{
    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
    Config::SpecialShield->value = (uint8_t)Kratos::Shield::kNone;
    auto RelicName = "not a Relic";
//  auto address = reinterpret_cast<std::uintptr_t>(a_RHandWeapon);
    auto kratos = Kratos::GetSingleton();
    if (a_RHandWeapon) {
        if (kratos->IsInRage(a_actor) && !kratos->IsWantFinishRage())
            kratos->EndRage(kratos->GetLastTriggeredRageType(), false, false, false, a_actor);
        if ((Config::LeviathanAxeKWD && a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) || a_RHandWeapon->formID == kratos->gLeviathanAxeFormID->value) {
            isLeviathanAxe = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Leviathan Axe";
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kLeviathanAxe;
            LeviathanAxe = a_RHandWeapon;
            kratos->gLeviathanAxeFormID->value = a_RHandWeapon->formID;

            auto Levi = LeviathanAxe::GetSingleton();
            Levi->data.weap     = LeviathanAxe;
            Levi->data.ench     = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            Levi->data.poison   = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            Levi->data.damage   = static_cast<float>(LeviathanAxe->attackDamage);
        //  Levi->LeviProjBaseL->model = LeviathanAxe->model;
        //  Levi->LeviProjBaseH->model = LeviathanAxe->model;
        //  Levi->LeviProjBaseA->model = LeviathanAxe->model;
            if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {
                spdlog::debug("Levi is throwable fID {:08x}, rID {:08x}, lID {:08x}", LeviathanAxe->formID, LeviathanAxe->GetRawFormID(), LeviathanAxe->GetLocalFormID());
                Levi->SetThrowState(tState::kThrowable);
                Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
            } else spdlog::warn("Levi is not equipped for real");

            if (Levi->SpellCatchLevi && a_actor->AsMagicTarget()->HasMagicEffect(Levi->EffCatchLevi)) {a_actor->RemoveSpell(Levi->SpellCatchLevi);}

            if (Levi->LeviathanAxeProjectileA 
            && (Levi->GetThrowState() == tState::kArriving
             || Levi->GetThrowState() == tState::kArrived)) 
                Levi->Catch(true);
        }
        else if ((Config::BladeOfChaosKWD && a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) || a_RHandWeapon->formID == kratos->gBladeOfChaosFormID->value) {
            isBladeOfChaos = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Chaos";
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kBladeOfChaos;
            BladeOfChaos = a_RHandWeapon;
            kratos->gBladeOfChaosFormID->value = a_RHandWeapon->formID;
        }
        else if ((Config::DraupnirSpearKWD && a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) || a_RHandWeapon->formID == kratos->gDraupnirSpearFormID->value) {
            isDraupnirSpear = true;
            isRelic = true;
            isKratos = true;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kDraupnirSpear;
            RelicName = "the Draupnir Spear";
            DraupnirSpear = a_RHandWeapon;
            kratos->gDraupnirSpearFormID->value = a_RHandWeapon->formID;
        }
        else if ((Config::BladeOfOlympusKWD && a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) || a_RHandWeapon->formID == kratos->gBladeOfOlympusFormID->value) {
            isBladeOfOlympus = true;
            isRelic = true;
            isKratos = true;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kBladeOfOlympus;
            RelicName = "the Blade of Olympus";
            BladeOfOlympus = a_RHandWeapon;
            kratos->gBladeOfOlympusFormID->value = a_RHandWeapon->formID;
        }
        a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
        spdlog::info("{} is {}", a_RHandWeapon->GetName(), RelicName);
    } if (a_shield) {
        RelicName = "not a Relic";
        if ((Config::GuardianShieldKWD && a_shield->HasKeyword(Config::GuardianShieldKWD)) || a_shield->formID == kratos->gGuardianShieldFormID->value) {
            isGuardianShield = true;
            isKratos = true;
            Config::SpecialShield->value = (uint8_t)Kratos::Shield::kGuardianShield;
            RelicName = "the Guardian Shield";
            GuardianShield = a_shield;
            kratos->gGuardianShieldFormID->value = a_shield->formID;
        }
        spdlog::info("{} is {}", a_shield->GetName(), RelicName);
    }
    a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
}
void WeaponIdentify::SpecialityCheck(RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectARMO* a_shield, const Kratos::Relic a_relic, const Kratos::Shield a_specialShield)
{
    if (Config::SpecialKWDs.size() != 5) {spdlog::error("check the special equipment keyword list!"); return;}
    if (a_RHandWeapon) {
        switch (a_relic)
        {
        case Kratos::Relic::kNone:
            if (a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false))
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
            break;
        case Kratos::Relic::kLeviathanAxe:
            if (!isLeviathanAxe || !a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::LeviathanAxeKWD);
            } break;
        case Kratos::Relic::kBladeOfChaos:
            if (!isBladeOfChaos || !a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::BladeOfChaosKWD);
            } break;
        case Kratos::Relic::kDraupnirSpear:
            if (!isDraupnirSpear || !a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::DraupnirSpearKWD);
            } break;
        case Kratos::Relic::kBladeOfOlympus:
            if (!isBladeOfOlympus || !a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::BladeOfOlympusKWD);
            } break;

        default:
            break;
        }
    }
    if (a_shield) {
        switch (a_specialShield)
        {
        case Kratos::Shield::kNone:
            if (isGuardianShield || !a_shield->HasKeywordInArray(Config::SpecialKWDs, false)) {
                a_shield->RemoveKeywords(Config::SpecialKWDs);
            } break;
        case Kratos::Shield::kGuardianShield:
            if (!isGuardianShield || !a_shield->HasKeywordInArray(Config::SpecialKWDs, false)) {
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
    isRelic = false;
    isKratos = false;

    auto AnArchos = PlayerCharacter::GetSingleton();
    if      (!AnArchos || !AnArchos->AsActorValueOwner()) return;
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
            SpecialityCheck(RHandWeapon, shield, static_cast<Kratos::Relic>((uint8_t)Config::SpecialWeapon->value), static_cast<Kratos::Shield>((uint8_t)Config::SpecialShield->value));
        }
        WeaponIdentifier(AnArchos, RHandWeapon, shield);
    } else {
        Config::SpecialWeapon->value = (float)Kratos::Relic::kNone;
        Config::SpecialShield->value = (float)Kratos::Shield::kNone;
        AnArchos->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
    }
}
bool WeaponIdentify::IsRelic(RE::BGSProjectile *a_baseProj, bool a_onlyLevi, bool a_onlyDraupnir)
{
    if (!a_baseProj) return false;
    auto Levi = LeviathanAxe::GetSingleton();
    auto leviA = Levi->LeviProjBaseA;
    auto leviL = Levi->LeviProjBaseL;
    auto leviH = Levi->LeviProjBaseH;
    auto draup = Draupnir::DraupnirSpearProjBaseL;
    auto dcall = Draupnir::DraupnirsCallProjBaseL;
    if (a_onlyLevi) return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH);
    else if (a_onlyDraupnir) return (a_baseProj == draup || a_baseProj == dcall);

    return (a_baseProj == leviA || a_baseProj == leviL || a_baseProj == leviH || a_baseProj == draup || a_baseProj == dcall);
}
//KRATOS
void Kratos::Update(RE::Actor* a_actor)
{
    if (IsInRage()) RestoreRage(a_actor, -(*values.rageDamageAmount * (*g_deltaTimeRealTime)));
    Draupnir::Update();
}
bool Kratos::Initialize()
{
    RE::PlayerCharacter::GetSingleton()->GetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);
    RE::PlayerCharacter::GetSingleton()->GetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);
    RE::PlayerCharacter::GetSingleton()->GetGraphVariableBool("Skip3DLoading", _skipLoad3D);

    auto dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {spdlog::error("can't get data handler!!"); return false;}
    bool found = true;
    SpellAxeThrownState     = dataHandler->LookupForm<RE::SpellItem>(0x81B, Config::KratosCombatESP);
    SpellSpartanRage        = dataHandler->LookupForm<RE::SpellItem>(0x80F, Config::KratosCombatESP);
    SpellStrenghtBuff       = dataHandler->LookupForm<RE::SpellItem>(0x829, Config::KratosCombatESP);
    SpellLeviChargeCD       = dataHandler->LookupForm<RE::SpellItem>(0x827, Config::KratosCombatESP);
    SpellAxeCallButton      = dataHandler->LookupForm<RE::SpellItem>(0x803, Config::KratosCombatESP);
    SpellAimButton          = dataHandler->LookupForm<RE::SpellItem>(0x805, Config::KratosCombatESP);
    SpellRunicButton        = dataHandler->LookupForm<RE::SpellItem>(0x809, Config::KratosCombatESP);
    SpellFinisherButton     = dataHandler->LookupForm<RE::SpellItem>(0x807, Config::KratosCombatESP);
    SpellMidDistButton      = dataHandler->LookupForm<RE::SpellItem>(0x80B, Config::KratosCombatESP);
    SpellLongDistButton     = dataHandler->LookupForm<RE::SpellItem>(0x80D, Config::KratosCombatESP);
    soundEffect.catchLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x2398A, "Skyrim.esm");
    soundEffect.callLevi    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x7D013, "Skyrim.esm");
    soundEffect.chargeLevi  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EDD5, "Skyrim.esm");
    soundEffect.chargeLeviEnd   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EAC6, "Skyrim.esm");
    VFXeffect.handFrost         = dataHandler->LookupForm<RE::BGSArtObject>(0x42854, "Skyrim.esm");
    VFXeffect.handFrostBright   = dataHandler->LookupForm<RE::BGSArtObject>(0x334B9, "Skyrim.esm");
    VFXeffect.handFlame     = dataHandler->LookupForm<RE::BGSArtObject>(0x1B211, "Skyrim.esm");
    VFXeffect.iceCloak      = dataHandler->LookupForm<RE::BGSArtObject>(0x4253F, "Skyrim.esm");
    VFXeffect.fireCloak     = dataHandler->LookupForm<RE::BGSArtObject>(0x2ACD7, "Skyrim.esm");
    action.normalAttack     = dataHandler->LookupForm<RE::BGSAction>(0x13005, "Skyrim.esm");
    action.powerAttack      = dataHandler->LookupForm<RE::BGSAction>(0x13383, "Skyrim.esm");
    action.dualPowerAttack  = dataHandler->LookupForm<RE::BGSAction>(0x2E2F7, "Skyrim.esm");

    gLeviathanAxeFormID     = dataHandler->LookupForm<RE::TESGlobal>(0x839, Config::KratosCombatESP);
    gBladeOfChaosFormID     = dataHandler->LookupForm<RE::TESGlobal>(0x83A, Config::KratosCombatESP);
    gDraupnirSpearFormID    = dataHandler->LookupForm<RE::TESGlobal>(0x83B, Config::KratosCombatESP);
    gBladeOfOlympusFormID   = dataHandler->LookupForm<RE::TESGlobal>(0x83C, Config::KratosCombatESP);
    gGuardianShieldFormID   = dataHandler->LookupForm<RE::TESGlobal>(0x83D, Config::KratosCombatESP);

    if (!SpellAxeThrownState || !SpellSpartanRage || !SpellStrenghtBuff || !SpellLeviChargeCD || !VFXeffect.handFrost)
        {spdlog::error("Check Kratos's spell addresses"); found = false;}

    values.rage = &Config::RageAmount->value;
    values.rageLimit = &Config::RageLimit->value;
    values.rageBuffAmount = &Config::RageBuffRate->value;
    values.rageDamageAmount = &Config::RageDamageRate->value;

    EffectAxeThrownState= SpellAxeThrownState ? SpellAxeThrownState->effects[0]->baseEffect : nullptr;
    EffectSpartanRage   = SpellSpartanRage ? SpellSpartanRage->effects[0]->baseEffect : nullptr;
    EffectStrenghtBuff  = SpellStrenghtBuff ? SpellStrenghtBuff->effects[0]->baseEffect : nullptr;
    EffectLeviChargeCD  = SpellLeviChargeCD ? SpellLeviChargeCD->effects[0]->baseEffect : nullptr;
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
    default:
        break;
    } return false;
}
bool Kratos::IsCanCallAxe(RE::Actor* a_actor) const {if (a_actor && SpellAxeThrownState) return a_actor->HasSpell(SpellAxeThrownState); return false;}
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
    auto magicTarget = a_actor->GetMagicTarget();
    auto cdLeviEffect = SpellLeviChargeCD ? SpellLeviChargeCD->effects[0] : nullptr;
    auto cdLeviBaseEffect = cdLeviEffect ? cdLeviEffect->baseEffect : nullptr;
    if (magicTarget) {
        switch  (a_relic) {
        case Relic::kLeviathanAxe:
            if (a_actor && cdLeviBaseEffect) isCan = !magicTarget->HasMagicEffect(cdLeviBaseEffect);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
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
    return Relic::kNone;
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
            if (!IsInRage()) StartRage((Rage)Config::RageType, true, a_actor);
            else EndRage(GetLastTriggeredRageType());
            break;
        case Kratos::Action::kWeaponCharge:
            a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kWeaponCharge);
            a_actor->NotifyAnimationGraph("DoKratosAction");
            break;
        case Kratos::Action::kSpecialIdle:
            a_actor->SetGraphVariableInt("iKratosActionType", (uint8_t)Kratos::Action::kSpecialIdle);
            a_actor->NotifyAnimationGraph("DoKratosAction");
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
        default:
            break;
        }
    }
    else if (!a_isCan && IsCanCharge(a_actor, a_relic) && magicTarget) {
        switch (a_relic) {
        case Relic::kLeviathanAxe:
            a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(SpellLeviChargeCD, false, nullptr, 1.f, false, 1.f, a_actor);
            break;
        case Relic::kBladeOfChaos:
            break;
        case Relic::kDraupnirSpear:
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
            if (WeaponIdentify::EquippedObjL)
                ObjectUtil::Actor::UnEquipItem(a_actor, false, false, false, false, true, true);
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
        a_actor->SetGraphVariableInt("IsInRage", true);
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
        a_actor->SetGraphVariableInt("IsInRage", false);
        _isWantFinishRage = false;
        _LastEquippedObjectR = nullptr;
        _LastEquippedObjectL = nullptr;
    }
}
//LEVIATHAN AXE
LeviathanAxe* LeviathanAxe::GetSingleton()              {static LeviathanAxe singleton; return &singleton;}
bool LeviathanAxe::Initialize()
{
    bool found = true;
    auto dataHandler = RE::TESDataHandler::GetSingleton();
    LeviProjBaseL       = dataHandler->LookupForm<RE::BGSProjectile>     (0x815, Config::KratosCombatESP);
    LeviProjBaseH       = dataHandler->LookupForm<RE::BGSProjectile>     (0x816, Config::KratosCombatESP);
    LeviProjBaseA       = dataHandler->LookupForm<RE::BGSProjectile>     (0x822, Config::KratosCombatESP);
    SpellLeviProjL   = dataHandler->LookupForm<RE::SpellItem>           (0x811, Config::KratosCombatESP);
    SpellLeviProjH   = dataHandler->LookupForm<RE::SpellItem>           (0x813, Config::KratosCombatESP);
    SpellCatchLevi   = dataHandler->LookupForm<RE::SpellItem>           (0x81D, Config::KratosCombatESP);
    SpellLeviProjA   = dataHandler->LookupForm<RE::SpellItem>           (0x823, Config::KratosCombatESP);
    EnchCharge          = dataHandler->LookupForm<RE::EnchantmentItem>  (0x45D58, "Skyrim.esm");     // EnchWeaponFrostDamage06, +30 frost damage, ff, touch
    if (LeviProjBaseL && LeviProjBaseH && LeviProjBaseA)
            spdlog::debug("Leviathan Axe projectiles are {}, {} and {}", LeviProjBaseL->GetName(), LeviProjBaseH->GetName(), LeviProjBaseA->GetName());
    else     {spdlog::error("Can't find Leviathan Axe projectiles"); found = false;}
    if (SpellLeviProjL && SpellLeviProjH)   
            spdlog::debug("Leviathan Axe projectile spells are {} and {}", SpellLeviProjL->GetName(), SpellLeviProjH->GetName());
    else     {spdlog::error("Can't find Leviathan Axe projectile spells"); found = false;}
    if (SpellCatchLevi) {
            EffCatchLevi = SpellCatchLevi->effects[0]->baseEffect;
            spdlog::debug("Leviathan Axe catching spell is {}", SpellCatchLevi->GetName()); EffCatchLevi = SpellCatchLevi->effects[0]->baseEffect;}
    else     {spdlog::error("Can't find Leviathan Axe catching spell"); found = false;}
    if (SpellLeviProjA)
            spdlog::debug("Leviathan Axe calling spell is {}", SpellLeviProjA->GetName());
    else     {spdlog::error("Can't find Leviathan Axe calling spell"); found = false;}
    if (EnchCharge) {           
            spdlog::debug("Default Leviathan Axe charging enchantment is {}", EnchCharge->GetName());}
    else     {spdlog::error("Can't find default Leviathan Axe enchantment spell"); found = false;}

    return found;
}
tState LeviathanAxe::GetThrowState()     const          {return throwState;}
void LeviathanAxe::SetThrowState(ThrowState a_state)    {throwState = a_state;}
void LeviathanAxe::SetStartPos(RE::NiPoint3& a_point, RE::Actor* a_actor)
{
    if (!a_actor) return;
    auto stuckedLevi =  LastLeviProjectile;
    if (!stuckedLevi) stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : LeviathanAxeProjectileH);
    if (stuckedLevi) {
        a_point = stuckedLevi->data.location; 
    } else spdlog::debug("we cant get leviathan's stucked proj!");

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
    if (GetThrowState() == tState::kThrowable) {
        if (auto backWeaponSheathe = a_actor->GetNodeByName("WeaponBack"); backWeaponSheathe) {
            a_point = backWeaponSheathe->world.translate;
        }
    }
}
void LeviathanAxe::Throw(const bool isVertical, RE::Actor* a_actor)
{
    const auto leviThrowSpell = (isVertical ? SpellLeviProjH : SpellLeviProjL);
//  auto leviBaseProj = (isVertical ? LeviProjBaseH : LeviProjBaseL);
    if (a_actor && leviThrowSpell && WeaponIdentify::isLeviathanAxe) 
    {   //  calculate damage
        const auto leviDamage = static_cast<float>(WeaponIdentify::LeviathanAxe->attackDamage);
        float mag = leviDamage * WeaponIdentify::DamageMult;
        if (isVertical) {mag *= 1.5f; data.yAngle = 1.57f;}
        else data.yAngle = 0.35f;
        const auto leviProjEff = leviThrowSpell->effects[0];
        auto& leviProjEffSetting = leviProjEff->effectItem;
        leviProjEffSetting.magnitude = mag;

    //  if (const auto leviProjBaseEff = leviProjEff->baseEffect; leviProjBaseEff) {
    //      leviProjBaseEff->data.projectileBase->SetModel(WeaponIdentify::LeviathanAxe->GetModel());
    //  //  leviProjBaseEff->data.projectileBase->data.defaultWeaponSource = WeaponIdentify::LeviathanAxe;
    //  //  leviProjBaseEff->data.associatedForm = WeaponIdentify::LeviathanAxe;
    //  } else spdlog::warn("not found throwing effect!");
//
//      auto extraData = AnArchos->extraList;
//      extraData.SetInventoryChanges(RE::InventoryChanges)
    //  RE::ActorEquipManager::GetSingleton()->UnequipObject(AnArchos, WeaponIdentify::LeviathanAxe);//, nullptr, 1U, nullptr, false, false, false, true, nullptr);
    //  AnArchos->RemoveWeapon(RE::BIPED_OBJECT::kOneHandAxe);
    //  AnArchos->NotifyAnimationGraph("UnequipNoAnim");
//
    //  AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->CastSpellImmediate(leviThrowSpell, false, nullptr, 1.f, false, mag, AnArchos);
    //  auto origin = AnArchos->GetMagicCaster(RE::MagicSystem::CastingSource::kRightHand)->GetMagicNode()->world.translate;

        auto kratos = Kratos::GetSingleton();
        //  set the launch data
        auto origin = WeaponIdentify::RHandBone->world.translate;
        RE::ProjectileHandle pHandle;
        RE::Projectile::ProjectileRot pRot = {a_actor->GetAimAngle(), a_actor->GetAimHeading()};
        RE::Projectile::LaunchData lData(a_actor, origin, pRot, leviThrowSpell);

        lData.weaponSource = data.weap;
#ifdef EXPERIMENTAL_THROWPOISON
        lData.poison = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
        if (ObjectUtil::Enchantment::GetEquippedWeaponCharge(a_actor) > 0.f)
            lData.enchantItem = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor);
        //  throw the projectile
        RE::Projectile::Launch(&pHandle, lData);
        a_actor->SetGraphVariableBool("bLeviInCatchRange", false);

        kratos->SetIsCanCallAxe(a_actor);
        isAxeCalled = false;
        isAxeThrowed = true;
        SetThrowState(ThrowState::kThrown);
        WeaponIdentify::EquippedObjR = nullptr;
            spdlog::info("Leviathan Axe throwed, raw damage is: {}", mag);
        //  reset the last throw's traces
        if (data.stuckedBone)   data.stuckedBone     = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        if (data.lastHitActor)  data.lastHitActor   = nullptr;
        if (a_actor->HasSpell(SpellCatchLevi)) a_actor->RemoveSpell(SpellCatchLevi);
    } else spdlog::info("Leviathan Axe is not equipped for throwing!");
}
void LeviathanAxe::Call(const bool a_justDestroy, RE::Actor* a_actor)
{
    if (a_actor && data.weap) {
        spdlog::debug("Levi is calling...");
        auto stuckedLevi =  LastLeviProjectile ? LastLeviProjectile : nullptr;
        if (!stuckedLevi)   stuckedLevi = (LeviathanAxeProjectileL ? LeviathanAxeProjectileL : (LeviathanAxeProjectileH ? LeviathanAxeProjectileH : nullptr));
        if (stuckedLevi) {
            data.position = stuckedLevi->data.location;
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
            SetStartPos(startPoint, AnArchos);
            RE::ProjectileHandle pHandle;
            RE::Projectile::ProjectileRot pRot = {-AnArchos->GetAimAngle(), -AnArchos->GetAimHeading()};
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
        } else {spdlog::warn("WEIRD SpellLeviProjA is nullptr!");}
    } else {spdlog::warn("WEIRD you don't have the axe for calling!!");}
    spdlog::info("Levi is arriving...");
}
void LeviathanAxe::Catch(const bool a_justDestroy, RE::Actor* a_actor)
{
    if (a_actor) {
        if (LeviathanAxeProjectileA) {
            auto& runtimeData = LeviathanAxeProjectileA->GetProjectileRuntimeData();
            runtimeData.flags |= (1 << 25);                                     //  set as destroyed, RE::Projectile::Flags::kDestroyed
            if (a_justDestroy) return;
        }
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
            ObjectUtil::Actor::EquipItem(a_actor, WeaponIdentify::LeviathanAxe, WeaponIdentify::skipEquipAnim);//, 1U, true, false, false, true);
            ResetEquipAnimationAfter(100, a_actor);
            WeaponIdentify::WeaponCheck();
            SetThrowState(ThrowState::kThrowable);
            if (WeaponIdentify::skipEquipAnim) WeaponIdentify::skipEquipAnim = false;
        } else spdlog::warn("you not have the leviathan axe");

        if (data.stuckedBone)   data.stuckedBone    = nullptr;
        if (data.stuckedActor)  data.stuckedActor   = nullptr;
        if (data.lastHitActor)  data.lastHitActor   = nullptr;

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
    auto leviWeap   = data.weap;
    auto ench        = leviWeap ? ObjectUtil::Enchantment::GetInventoryItemEnchantment(AnArchos, leviWeap->formID) : nullptr;
    auto enchEffect = ench ? ench->effects[0] : nullptr;
    auto enchBase   = enchEffect ? enchEffect->baseEffect : nullptr;
    const auto leviDam  = data.damage;

    if (leviWeap && chargeHitCount <= 0) {
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
                kratos->chargeData.lastBuffedLevi = leviWeap->formID;
                kratos->SetIsCanCharge(AnArchos, false);
                spdlog::debug("magnitude buffing from {} to: {}", mag / a_magnitude, mag);
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
                    kratos->chargeData.lastChargedLevi = leviWeap->formID;

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
        if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi) == EnchCharge)
            ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi);
    } else if (kratos->IsEquipped(Kratos::Relic::kLeviathanAxe)) {
        if (chargeHitCount <= 0) {
            if (a_magnitude) *a_magnitude = a_defMagnitude;
            else {
                if (ObjectUtil::Enchantment::GetInventoryItemEnchantment(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi) == EnchCharge)
                    ObjectUtil::Enchantment::DisEnchantInventoryWeapon(RE::PlayerCharacter::GetSingleton(), kratos->chargeData.lastChargedLevi);
            }
            kratos->SetIsCanCharge(RE::PlayerCharacter::GetSingleton());
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
//BLADES OF CHAOS
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
    if (AsyncUtil::GameTime::GetEngineTime() > (_lastChargeTime + 0.5f)) {
        BuffScorchingSpeed(-0.06f);
    }
}
bool BladeOfChaos::IsQueueEnd()
{
    if (GetScorchingSpeed() < 0.4f) {
        return true;
    }
    DeBuffScorchingSpeed();
    return false;
}
//DRAUPNIR SPEAR
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
void Draupnir::Update()
{
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
        const auto draupnirDamage = (float)(WeaponIdentify::DraupnirSpear->attackDamage);
        const float mag = draupnirDamage * WeaponIdentify::DamageMult;
        const auto effDraupnir = SpellDraupnirProjL->effects[0];
        auto& leviProjEffSetting = effDraupnir->effectItem;
        leviProjEffSetting.magnitude = mag;

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
    const RE::BSTArray<RE::Actor *> nearCombatTargets = ObjectUtil::Actor::GetNearCombatTargets(AnArchos, 666.f);
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
        DraupnirExplosion->data.force = a_force;
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
            spdlog::info("draupnir explosion chain finished");
        } spdlog::info("all draupnir explosions done");
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
    if (auto targetAVO = a_target->AsActorValueOwner(); targetAVO)
        targetAVO->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, RE::ActorValue::kHealth, -damage);
}
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
        case "CallWeapon"_h:
            LeviathanAxe::GetSingleton()->Call();
            break;
        case "CatchLevi"_h:
            break;
        case "LeviCallAttack"_h:     //event: attackPowerStartInPlace, attackStart, PowerAttack [IDLE:000E8456], NormalAttack [IDLE:00013215]
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi->GetThrowState() == tState::kCanArrive && WeaponIdentify::LeviathanAxe) {
                Levi->Call(true);
                auto AnArchos = PlayerCharacter::GetSingleton();
                ObjectUtil::Actor::EquipItem(AnArchos, WeaponIdentify::LeviathanAxe, true, 1u, true, false, false, false);
                ResetEquipAnimationAfter(100, AnArchos);
            } else spdlog::info("Levi is not callable");
        case "ThrowAttackStart"_h:
            if (WeaponIdentify::isLeviathanAxe)
                Kratos::GetSingleton()->SetIsCanCharge(RE::PlayerCharacter::GetSingleton(), false);
            break;
        case "ThrowWeapon"_h:
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi && Levi->GetThrowState() == tState::kThrowable) {
                Levi->Throw(false);
            }
            else spdlog::warn("Levi is not throwable");
            break;
        case "ThrowWeaponV"_h:
            if (auto Levi = LeviathanAxe::GetSingleton(); Levi && Levi->GetThrowState() == tState::kThrowable) {
                Levi->Throw(true);
            }
            else spdlog::warn("Levi is not throwable");
            break;
        case "ThrowAttackEnd"_h:
            if (auto AnArchos = PlayerCharacter::GetSingleton(); auto kratos = Kratos::GetSingleton()) {
                ObjectUtil::Actor::UnEquipItem(AnArchos, false, false, true, true, true, true);
                ResetEquipAnimationAfter(100, AnArchos);
            }
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
        case "ThrowSpear"_h:
            Draupnir::Throw();
            break;
        case "DraupnirsCall"_h:
            if (WeaponIdentify::DraupnirSpear) {
                Draupnir::Call(10.f, 100.f);
            //    const float draupnirDamage = WeaponIdentify::DraupnirSpear->attackDamage;
            //    const float damage = draupnirDamage * WeaponIdentify::DamageMult;
            //    Draupnir::Call(damage, 100.f);
            }// else Draupnir::Call(10.f, 100.f);
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
#ifdef EXPERIMENTAL_SHIELD
        //  animated shield
        case "shieldOpen"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos) kratos->OpenShield(PlayerCharacter::GetSingleton());
            break;
        case "shieldClose"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos) kratos->CloseShield(PlayerCharacter::GetSingleton());
            break;
        case "BFCO_DIY_recovery"_h:
        case "MCO_Recovery"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsShieldOpened()) kratos->CloseShield(PlayerCharacter::GetSingleton());
            break;
        case "tailCombatState"_h:
            ObjectUtil::Actor::SendAnimationEvent(PlayerCharacter::GetSingleton(), "shieldClose");
            break;
#endif
        case "weaponDraw"_h:
            WeaponIdentify::WeaponCheck();
            if (WeaponIdentify::isLeviathanAxe) {
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->SetThrowState(tState::kThrowable);
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                }
            }
            break;
        case "weaponSwing"_h:
            if (auto kratos = Kratos::GetSingleton(); kratos && kratos->IsInRage())
                kratos->RestoreRage(RE::PlayerCharacter::GetSingleton(), kratos->CalcRageDamageOrBuffAmount(360.f));
            break;
        case "CastOKStart"_h:
            if (WeaponIdentify::isLeviathanAxe)
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag);
                }
            break;
        case "AttackWinStart"_h:
        case "MCO_WinOpen"_h:
        case "BFCO_WinOpen"_h:
        case "Collision_AttackEnd"_h:
            if (WeaponIdentify::isLeviathanAxe)
                if (auto Levi = LeviathanAxe::GetSingleton()) {
                    Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                }
            break;
        case "InsertDraupnir"_h:
                Draupnir::MeleeThrow();
            break;
        case "RainOfSpear"_h:
            Draupnir::ArtilleryOfTheAncients(0.1f, 3.f);
            break;
        case "FlameWhiplashStart"_h:
            if (auto BoC = BladeOfChaos::GetSingleton()) {
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
                BoC->SetIsScorching(false);
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
            auto levi = LeviathanAxe::GetSingleton();
            auto kratos = Kratos::GetSingleton();
            if (!casterMT || !levi || !kratos) return EventChecker::kContinue;

            if (formID == kratos->spellID.aim) {
                spdlog::debug("aiming...");
            } else if (formID == kratos->spellID.call) {
                if (!WeaponIdentify::isRelic) {
                    if (levi->data.weap) {
                        if ((uint_fast8_t)levi->GetThrowState() <= 3U) {
                            caster->SetGraphVariableBool("bLeviInCatchRange", false);
                            kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                        } else spdlog::debug("levi can't arrive!");
                    } else spdlog::debug("levi does not exist!");
                } else if (WeaponIdentify::isLeviathanAxe) {
                    if (!levi->isAxeThrowed && kratos->IsCanCharge(caster, Kratos::Relic::kLeviathanAxe)) {
                        kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                    }
                } else if (WeaponIdentify::isBladeOfChaos) {
                    BladeOfChaos::GetSingleton()->Update();
                    BladeOfChaos::GetSingleton()->BuffScorchingSpeed();
                    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
                } else if (WeaponIdentify::isDraupnirSpear) {
                    kratos->DoKratosAction(Kratos::Action::kWeaponCharge, caster);
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
EventChecker InputEventTracker::ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource)
{
    if (!a_event || RE::UI::GetSingleton()->GameIsPaused()) return EventChecker::kContinue;

    for (auto event = *a_event; event; event = event->next) {
        if (!event->HasIDCode() || event->GetEventType() != RE::INPUT_EVENT_TYPE::kButton) continue;

        auto keyCode = event->AsIDEvent()->GetIDCode();
        if (keyCode <= 0u) return EventChecker::kContinue;

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return EventChecker::kContinue;

        auto kratos = Kratos::GetSingleton();
        if (!kratos) return EventChecker::kContinue;

        if (auto button = static_cast<RE::ButtonEvent*>(event); button) {
            auto device = event->device.get();
            if (keyCode == GetOffsettedKeyCode(Config::AxeCallKey, device)) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellAxeCallButton); player->SetGraphVariableBool("bPressingCallButton", true);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellAxeCallButton); player->SetGraphVariableBool("bPressingCallButton", false);}
            }
            else if (keyCode == GetOffsettedKeyCode(Config::AimKey, device)) {
                if (button->IsDown()) {kratos->Aim(true); player->AddSpell(kratos->SpellAimButton); player->SetGraphVariableBool("bPressingAimButton", true);}
                else if (button->IsUp()) {kratos->Aim(false); player->RemoveSpell(kratos->SpellAimButton); player->SetGraphVariableBool("bPressingAimButton", true);}
            }
            else if (keyCode == GetOffsettedKeyCode(Config::RunicKey, device)) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellRunicButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellRunicButton);}
            }
            else if (keyCode == GetOffsettedKeyCode(Config::FinisherKey, device)) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellFinisherButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellFinisherButton);}
            }
            else if (keyCode == GetOffsettedKeyCode(Config::MediumDistanceKey, device)) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellMidDistButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellMidDistButton);}
            }
            else if (keyCode == GetOffsettedKeyCode(Config::LongDistanceKey, device)) {
                if (button->IsDown()) {player->AddSpell(kratos->SpellLongDistButton);}
                else if (button->IsUp()) {player->RemoveSpell(kratos->SpellLongDistButton);}
            }
        }
    } return EventChecker::kContinue;
}
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
    case RE::INPUT_DEVICE::kGamepad:
        a_keyCode += kGamepadOffset;
        break;
    case RE::INPUT_DEVICE::kMouse:
        a_keyCode += kMouseOffset;
        break;
    default:
        break;
    } return a_keyCode;
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