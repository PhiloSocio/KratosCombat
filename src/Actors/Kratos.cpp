#include "Kratos.h"
#include "Thrower.h"
#include "Charger.h"
#include "Caller.h"
#include "Rager.h"

Kratos::Kratos(RE::ActorHandle a_actorHandle) : 
    BaseActor(a_actorHandle), Thrower(a_actorHandle), Caller(a_actorHandle), Rager(a_actorHandle) , Charger(a_actorHandle) 
{
    if (IsValid()) {
        alterationLevel = GetActor()->GetActorValue(RE::ActorValue::kAlteration);
    }
}

/*
using namespace Util;
#pragma region KRATOS
void Kratos::Update(RE::Actor* a_actor, const float a_delta)
{
    if (IsInRage()) RestoreRage(a_actor, -(*values.rageDamageAmount * (*g_deltaTimeRealTime)), true);

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
    if (a_amount == 0.f) return 0.f;
    else if (a_amount > 0.f) {
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
        if ((*values.rage - *values.rageDamageAmount * 10.f) < 0.f) return;
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
                a_actor->AsActorValueOwner()->RestoreActorValue(RE::ActorValue::kHealth, (*values.rageBuffAmount * 10.f));
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
*/