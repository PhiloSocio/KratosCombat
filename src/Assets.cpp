#include "Assets.h"
#include "Settings.h"
#include "Util.h"

bool Assets::Initialize()
{
    if (auto AnArchos = RE::PlayerCharacter::GetSingleton(); AnArchos) {
        AnArchos->GetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);    //  Taking default values
        AnArchos->GetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);     //  Taking default values
        AnArchos->GetGraphVariableBool("Skip3DLoading", _skipLoad3D);            //  Taking default values
    } else spdlog::warn("WEIRD, the player character can't found");

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
    soundEffects.throwLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85E, Config::KratosCombatESP);
    soundEffects.callLevi    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x86B, Config::KratosCombatESP);
    soundEffects.arrivingLeviStart   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85A, Config::KratosCombatESP);
    soundEffects.arrivingLeviLoop0   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85B, Config::KratosCombatESP);
    soundEffects.arrivingLeviLoop1   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85C, Config::KratosCombatESP);
    soundEffects.arrivingLeviLoop2   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x85D, Config::KratosCombatESP);
    soundEffects.arrivingLeviNear    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x86C, Config::KratosCombatESP);
    soundEffects.catchLevi   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x84E, Config::KratosCombatESP);
    soundEffects.throwingLeviLoop0   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x863, Config::KratosCombatESP);
    soundEffects.throwingLeviLoop1   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x864, Config::KratosCombatESP);
    soundEffects.chargeLevi  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EDD5, "Skyrim.esm");
    soundEffects.chargeLeviLoop  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3E5CB, "Skyrim.esm");
    soundEffects.chargeLeviEndT  = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EDD5, "Skyrim.esm");
    soundEffects.chargeLeviEnd   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EAC6, "Skyrim.esm");
    soundEffects.catchMjolnir    = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x2398A, "Skyrim.esm");
    soundEffects.callMjolnir     = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x7D013, "Skyrim.esm");
    soundEffects.fingerSnap      = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x84A, Config::KratosCombatESP);
    soundEffects.chargeMjolnir   = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3F205, "Skyrim.esm");
    soundEffects.chargeMjolnirEnd= dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x3EAC6, "Skyrim.esm");
    vanillaSpells.frostCloak     = dataHandler->LookupForm<RE::SpellItem>(0x3AEA2, "Skyrim.esm");
    vanillaSpells.fireCloak      = dataHandler->LookupForm<RE::SpellItem>(0x3AE9F, "Skyrim.esm");
    vanillaSpells.lightningBolt  = dataHandler->LookupForm<RE::SpellItem>(0x2DD29, "Skyrim.esm");
    vanillaSpells.lightningCloak = dataHandler->LookupForm<RE::SpellItem>(0x3AEA3, "Skyrim.esm");
    vanillaSpells.thunderbolt    = dataHandler->LookupForm<RE::SpellItem>(0x10F7EE, "Skyrim.esm");
    vanillaSpells.sparks         = dataHandler->LookupForm<RE::SpellItem>(0x2DD2A, "Skyrim.esm");
    kittyTailSpells.magesFury    = dataHandler->LookupForm<RE::SpellItem>(0x809, "StormCalling.esl");
    kittyTailSpells.streak       = dataHandler->LookupForm<RE::SpellItem>(0x80A, "StormCalling.esl");
    kittyTailSpells.teslaSphere  = dataHandler->LookupForm<RE::SpellItem>(0x833, "StormCalling.esl");
    kittyTailSpells.tonitrus     = dataHandler->LookupForm<RE::SpellItem>(0x955, "StormCalling.esl");
    kittyTailSpells.maelstrom    = dataHandler->LookupForm<RE::SpellItem>(0xA04, "StormCalling.esl");
    kittyTailSpells.lightningFlood   = dataHandler->LookupForm<RE::SpellItem>(0x80E, "StormCalling.esl");
    VFXeffects.handFrost         = dataHandler->LookupForm<RE::BGSArtObject>(0x42854, "Skyrim.esm");
    VFXeffects.handFrostBright   = dataHandler->LookupForm<RE::BGSArtObject>(0x334B9, "Skyrim.esm");
    VFXeffects.handShock         = dataHandler->LookupForm<RE::BGSArtObject>(0x58E9B, "Skyrim.esm");
    VFXeffects.handShockCloak    = dataHandler->LookupForm<RE::BGSArtObject>(0x5B1BD, "Skyrim.esm");
    VFXeffects.handFlame     = dataHandler->LookupForm<RE::BGSArtObject>(0x1B211, "Skyrim.esm");
    VFXeffects.frostCloak    = dataHandler->LookupForm<RE::BGSArtObject>(0x4253F, "Skyrim.esm");
    VFXeffects.fireCloak     = dataHandler->LookupForm<RE::BGSArtObject>(0x2ACD7, "Skyrim.esm");
    actions.normalAttack     = dataHandler->LookupForm<RE::BGSAction>(0x13005, "Skyrim.esm");
    actions.powerAttack      = dataHandler->LookupForm<RE::BGSAction>(0x13383, "Skyrim.esm");
    actions.dualPowerAttack  = dataHandler->LookupForm<RE::BGSAction>(0x2E2F7, "Skyrim.esm");

    gLeviathanAxeFormID     = dataHandler->LookupForm<RE::TESGlobal>(0x839, Config::KratosCombatESP);
    gBladeOfChaosFormID     = dataHandler->LookupForm<RE::TESGlobal>(0x83A, Config::KratosCombatESP);
    gDraupnirSpearFormID    = dataHandler->LookupForm<RE::TESGlobal>(0x83B, Config::KratosCombatESP);
    gBladeOfOlympusFormID   = dataHandler->LookupForm<RE::TESGlobal>(0x83C, Config::KratosCombatESP);
    gMjolnirFormID          = dataHandler->LookupForm<RE::TESGlobal>(0x83E, Config::KratosCombatESP);
#ifdef TRIDENT
    gTridentFormID          = dataHandler->LookupForm<RE::TESGlobal>(0x800, Config::TridentModESP);
#endif
    gGuardianShieldFormID   = dataHandler->LookupForm<RE::TESGlobal>(0x83D, Config::KratosCombatESP);

    if (!SpellAxeThrownState || !SpellSpartanRage || !SpellStrenghtBuff || !SpellMjolnirChargeCD || !VFXeffects.handFrost)
        {spdlog::error("Check Kratos's spell addresses"); found = false;}

//    values.rage = &Config::RageAmount->value;
//    values.rageLimit = &Config::RageLimit->value;
//    values.rageBuffAmount = &Config::RageBuffRate->value;
//    values.rageDamageAmount = &Config::RageDamageRate->value;

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
        spellIDs.aim = EffectAimButton->GetFormID();
        spellIDs.call = EffectAxeCallButton->GetFormID();
        spellIDs.runic = EffRunicButton->GetFormID();
        spellIDs.finisher = EffFinisherButton->GetFormID();
        spellIDs.midDist = EffMidDistButton->GetFormID();
        spellIDs.longDist = EffLongDistButton->GetFormID();
        spellIDs.leviChargeCoolDown = EffectLeviChargeCD->GetFormID();
        spellIDs.AxeThrownState = EffectAxeThrownState->GetFormID();
        spellIDs.strenghtBuff = EffectStrenghtBuff->GetFormID();
        spellIDs.spartanRage = EffectSpartanRage->GetFormID();
    } return found;
}

