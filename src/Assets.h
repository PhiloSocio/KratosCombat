#pragma once

class Assets
{
public:
    static Assets* GetSingleton() {static Assets singleton; return &singleton;}

    bool Initialize();
//--------------------------
//--------------------------
    struct VanillaActions {
        RE::BGSAction* normalAttack = nullptr;
        RE::BGSAction* powerAttack = nullptr;
        RE::BGSAction* dualPowerAttack = nullptr;
    };

    struct VanillaSpells {
        RE::SpellItem* frostCloak = nullptr;
        RE::SpellItem* fireCloak = nullptr;
        RE::SpellItem* lightningCloak = nullptr;
        RE::SpellItem* lightningBolt = nullptr;
        RE::SpellItem* thunderbolt = nullptr;
        RE::SpellItem* sparks = nullptr;
    };

    struct KittyTailSpells {
        RE::SpellItem* magesFury = nullptr;
        RE::SpellItem* streak = nullptr;
        RE::SpellItem* lightningFlood = nullptr;
        RE::SpellItem* teslaSphere = nullptr;
        RE::SpellItem* tonitrus = nullptr;
        RE::SpellItem* maelstrom = nullptr;
    };

    struct VFXeffects {
        RE::BGSArtObject* handFrost = nullptr;      //  FrostHandFX01 [ARTO:00020BF5], FrostBoltHandFX01 [ARTO:000334B9]
        RE::BGSArtObject* handFrostBright = nullptr;//  FXIceCloakHandEffects [ARTO:00042854],
        RE::BGSArtObject* handFlame = nullptr;      //  Fireball01HandEffects [ARTO:0001B211], FireCloakHandEffects [ARTO:00036342]
        RE::BGSArtObject* handShock = nullptr;      //  FXShockCloakHandEffects [ARTO:0005B1BD]
        RE::BGSArtObject* handShockCloak = nullptr;
        RE::BGSArtObject* frostCloak = nullptr;      //  FXIceCloak01 [ARTO:0004253F]
        RE::BGSArtObject* fireCloak = nullptr;      //  FXFireCloak01 [ARTO:0002ACD7]
        RE::BGSArtObject* fury = nullptr;
        RE::BGSArtObject* legacy = nullptr;
        RE::BGSArtObject* valor = nullptr;
        RE::BGSArtObject* wrath = nullptr;
    };

    struct SoundEffects {
        RE::BGSSoundDescriptorForm* throwLevi = nullptr;
        RE::BGSSoundDescriptorForm* callLevi = nullptr;     //  telekinesis,0x7D013
        RE::BGSSoundDescriptorForm* arrivingLeviStart = nullptr;
        RE::BGSSoundDescriptorForm* arrivingLeviLoop0 = nullptr;
        RE::BGSSoundDescriptorForm* arrivingLeviLoop1 = nullptr;
        RE::BGSSoundDescriptorForm* arrivingLeviLoop2 = nullptr;
        RE::BGSSoundDescriptorForm* arrivingLeviNear = nullptr;
        RE::BGSSoundDescriptorForm* catchLevi = nullptr;    //  WPNImpactBladeVsIceSD [SNDR:0002398A]
        RE::BGSSoundDescriptorForm* throwingLeviLoop0 = nullptr;
        RE::BGSSoundDescriptorForm* throwingLeviLoop1 = nullptr;
        RE::BGSSoundDescriptorForm* chargeLevi = nullptr;   //  MAGIcicleChargeSD [SNDR:0003EDD5], MAGIcicleReadyLPSD [SNDR:0003F1F0]
        RE::BGSSoundDescriptorForm* chargeLeviLoop = nullptr;// MAGFrostBiteFireLPMSD [SNDR:0003E5CB]
        RE::BGSSoundDescriptorForm* chargeLeviEndT = nullptr;
        RE::BGSSoundDescriptorForm* chargeLeviEnd = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
        RE::BGSSoundDescriptorForm* catchMjolnir = nullptr; //  WPNImpactBladeVsIceSD [SNDR:0002398A]
        RE::BGSSoundDescriptorForm* callMjolnir = nullptr;  //  telekinesis,0x7D013
        RE::BGSSoundDescriptorForm* fingerSnap = nullptr;   //  recorded by me with whatsapp voice message
        RE::BGSSoundDescriptorForm* chargeMjolnir = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
        RE::BGSSoundDescriptorForm* chargeMjolnirEnd = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
    };

    struct SpellID {
        //  button spell effect ID's
        RE::FormID aim;
        RE::FormID call;
        RE::FormID runic;
        RE::FormID finisher;
        RE::FormID midDist;
        RE::FormID longDist;

        //  other spell ID's
        RE::FormID leviChargeCoolDown;
        RE::FormID AxeThrownState;
        RE::FormID strenghtBuff;
        RE::FormID spartanRage;
    };

    VanillaActions actions;
    VanillaSpells vanillaSpells;
    KittyTailSpells kittyTailSpells;
    VFXeffects VFXeffects;
    SoundEffects soundEffects;
    SpellID spellIDs;
//--------------------------
    RE::SpellItem*      SpellAxeThrownState     = nullptr;
    RE::SpellItem*      SpellMjolnirThrownState = nullptr;
    RE::SpellItem*      SpellSpartanRage        = nullptr;
    RE::SpellItem*      SpellStrenghtBuff       = nullptr;
    RE::SpellItem*      SpellLeviChargeCD       = nullptr;
    RE::SpellItem*      SpellMjolnirChargeCD    = nullptr;
    RE::EffectSetting*  EffectAxeThrownState    = nullptr;
    RE::EffectSetting*  EffectSpartanRage       = nullptr;
    RE::EffectSetting*  EffectStrenghtBuff      = nullptr;
    RE::EffectSetting*  EffectLeviChargeCD      = nullptr;
    RE::EffectSetting*  EffectMjolnirChargeCD   = nullptr;

    RE::SpellItem*      SpellAimButton      = nullptr;
    RE::SpellItem*      SpellAxeCallButton  = nullptr;
    RE::SpellItem*      SpellRunicButton    = nullptr;
    RE::SpellItem*      SpellFinisherButton = nullptr;
    RE::SpellItem*      SpellMidDistButton  = nullptr;
    RE::SpellItem*      SpellLongDistButton = nullptr;
    RE::EffectSetting*  EffectAimButton     = nullptr;
    RE::EffectSetting*  EffectAxeCallButton = nullptr;
    RE::EffectSetting*  EffRunicButton      = nullptr;
    RE::EffectSetting*  EffFinisherButton   = nullptr;
    RE::EffectSetting*  EffMidDistButton    = nullptr;
    RE::EffectSetting*  EffLongDistButton   = nullptr;

    RE::TESGlobal* gLeviathanAxeFormID      = nullptr;
    RE::TESGlobal* gBladeOfChaosFormID      = nullptr;
    RE::TESGlobal* gDraupnirSpearFormID     = nullptr;
    RE::TESGlobal* gBladeOfOlympusFormID    = nullptr;
    RE::TESGlobal* gMjolnirFormID           = nullptr;
    RE::TESGlobal* gTridentFormID           = nullptr;
    RE::TESGlobal* gGuardianShieldFormID    = nullptr;

    RE::TESObjectCONT* ThrowableWeaponContainer = nullptr;
private:
    ~Assets() = default;
};
