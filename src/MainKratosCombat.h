#pragma once
#include "settings.h"
#include "hook.h"

class WeaponIdentify
{
public:
    static inline bool isLeviathanAxe;
    static inline bool isBladeOfChaos;
    static inline bool isDraupnirSpear;
    static inline bool isBladeOfOlympus;
    static inline bool isRelic;
    static inline bool isKratos;

    static inline bool skipEquipAnim;

    static inline RE::NiAVObject* RHandBone         = nullptr;
    static inline RE::NiAVObject* LHandBone         = nullptr;
    static inline RE::NiAVObject* WeaponBone        = nullptr;
    static inline RE::NiAVObject* ShieldBone        = nullptr;
    static inline RE::TESObjectWEAP* LeviathanAxe   = nullptr;
    static inline RE::TESObjectWEAP* BladeOfChaos   = nullptr;
    static inline RE::TESObjectWEAP* DraupnirSpear  = nullptr;
    static inline RE::TESObjectWEAP* BladeOfOlympus = nullptr;
    static inline RE::TESObjectARMO* GuardianShield = nullptr;

    static inline float DamageMult = 18.f;

    static bool IsRelic(RE::Projectile *a_proj)                                                 {return IsRelic(a_proj, false, false);}
    static bool IsRelic(RE::Projectile *a_proj, bool a_onlyLevi)                                {return IsRelic(a_proj, a_onlyLevi, false);}
    static bool IsRelic(RE::Projectile *a_proj, bool a_onlyLevi, bool a_onlyDraupnir)           {return IsRelic(a_proj->GetProjectileBase(), a_onlyLevi, a_onlyDraupnir);}
    static bool IsRelic(RE::BGSProjectile *a_baseProj)                                          {return IsRelic(a_baseProj, false, false);}
    static bool IsRelic(RE::BGSProjectile *a_baseProj, bool a_onlyLevi)                         {return IsRelic(a_baseProj, a_onlyLevi, false);}
    static bool IsRelic(RE::BGSProjectile *a_baseProj, bool a_onlyLevi, bool a_onlyDraupnir);
    
    static void WeaponCheck();

private:

    static void WeaponIdentifier(RE::PlayerCharacter* a_player, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectARMO* a_shield);
};
/**/

class Kratos
{
public:
    static Kratos* GetSingleton() {static Kratos singleton; return &singleton;}

    bool Initialize();
//--------------------------
    enum class Relic : std::uint8_t {
        kNone = 0u,
        kLeviathanAxe = 1u,
        kBladeOfChaos = 2u,
        kDraupnirSpear = 3u,
        kBladeOfOlympus = 4u,
    };

    enum class Skill : std::uint8_t {
        kNone = 0u,
        kRage = 1u,
        kWeaponCharge = 2u,
        kRunicAttack = 3u,
    };

    enum class Rage : std::uint8_t {
        kSpartan = 0u,
        kValor = 1u,
        kWrath = 2u,
        kOlympus = 3u,
    };
//--------------------------
    struct Actions {
        RE::BGSAction* normalAttack = nullptr;
        RE::BGSAction* powerAttack = nullptr;
    };

    struct VFXeffects {
        RE::BGSArtObject* handFrost = nullptr;      //  FrostHandFX01 [ARTO:00020BF5], FrostBoltHandFX01 [ARTO:000334B9]
        RE::BGSArtObject* handFrostBright = nullptr;//  FXIceCloakHandEffects [ARTO:00042854],
        RE::BGSArtObject* handFlame = nullptr;      //  Fireball01HandEffects [ARTO:0001B211], FireCloakHandEffects [ARTO:00036342]
        RE::BGSArtObject* iceCloak = nullptr;       //  FXIceCloak01 [ARTO:0004253F]
        RE::BGSArtObject* fireCloak = nullptr;      //  FXFireCloak01 [ARTO:0002ACD7]
        RE::BGSArtObject* spartanRage = nullptr;
        RE::BGSArtObject* olympusRage = nullptr;
        RE::BGSArtObject* valorRage = nullptr;
    };

    struct SoundEffects {
        RE::BGSSoundDescriptorForm* catchLevi = nullptr;    //  WPNImpactBladeVsIceSD [SNDR:0002398A]
        RE::BGSSoundDescriptorForm* callLevi = nullptr;     //  telekinesis,0x7D013
        RE::BGSSoundDescriptorForm* chargeLevi = nullptr;   //  MAGIcicleChargeSD [SNDR:0003EDD5], MAGIcicleReadyLPSD [SNDR:0003F1F0]
        RE::BGSSoundDescriptorForm* chargeLeviEnd = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
    };

    struct ChargeData {
        RE::FormID lastBuffedLevi;
        RE::FormID lastChargedLevi;
        RE::FormID lastBuffedBoC;
        RE::FormID lastChargedBoC;
        RE::FormID lastBuffedDraup;
        RE::FormID lastChargedDraup;
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
        RE::FormID rageCoolDown;
        RE::FormID spartanRage;
    };

    Actions action;
    VFXeffects VFXeffect;
    SoundEffects soundEffect;
    ChargeData chargeData;
    SpellID spellID;
//-------------------------- Functions
    bool IsEquipped(const Relic a_relic);
    bool IsCanCallAxe(RE::Actor* a_actor);
    bool IsCanRage(RE::Actor* a_actor);
    bool IsCanCharge(RE::Actor* a_actor, Relic a_relic = Relic::kLeviathanAxe); // 1: levi, 2: blade of chaos, 3: draupnir
    bool IsShieldOpened();

    void SetIsCanCallAxe(RE::Actor* a_actor, const bool a_isCan = true);
    void SetIsCanRage(RE::Actor* a_actor, const bool a_isCan = true);
    void SetIsCanCharge(RE::Actor* a_actor, const bool a_isCan = true, Relic a_relic = Relic::kLeviathanAxe); // 1: levi, 2: blade of chaos, 3: draupnir
    void SetCoolDownOf(const Skill a_skill, const uint8_t a_coolDownSec);

    void OpenShield(RE::Actor* a_actor);
    void CloseShield(RE::Actor* a_actor);
    void StartRage(RE::Actor* a_actor, const Rage a_rage);
    void EndRage(RE::Actor* a_actor);
//--------------------------
    RE::SpellItem*      SpellAxeThrownState = nullptr;
    RE::SpellItem*      SpellSpartanRage    = nullptr;
    RE::SpellItem*      SpellRageCoolDown   = nullptr;
    RE::SpellItem*      SpellLeviChargeCD   = nullptr;
    RE::EffectSetting*  EffectAxeThrownState= nullptr;
    RE::EffectSetting*  EffectSpartanRage   = nullptr;
    RE::EffectSetting*  EffectRageCoolDown  = nullptr;
    RE::EffectSetting*  EffectLeviChargeCD  = nullptr;

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

private:
    bool _isShieldOpened;
    Kratos() = default;
    ~Kratos() = default;
};
class LeviathanAxe
{
public:
    static LeviathanAxe* GetSingleton();// {static LeviathanAxe singleton; return &singleton;}

    struct Data {
        RE::TESObjectWEAP* weap     = nullptr;
        RE::EnchantmentItem* ench   = nullptr;
        RE::AlchemyItem* poison     = nullptr;
        RE::NiPoint3 position       = {0.f, 0.f, 0.f};
        RE::NiAVObject* model       = nullptr;
        RE::NiNode* stuckedBone     = nullptr;
        RE::Actor* stuckedActor     = nullptr;
        RE::Actor* lastHitActor     = nullptr;
        float* enchMag              = nullptr;
        float defaultEnchMag;
        float damage            = 0.f;
        float yAngle            = 0.35f;
        float throwedTime       = 0.f;
        float arrivalSpeed      = Config::MinArrivalSpeed;
    };

    enum class ThrowState : std::uint_fast8_t {
        kNone = 0,
        kArrived = 1,
        kThrowable = 2,
        kThrown = 3,
        kCanArrive = 4,
        kArriving = 5
    };

    Data data;

    void SetThrowState(const ThrowState a_throwState);
    ThrowState GetThrowState() const;
    void SetStartPos(RE::NiPoint3& a_point, RE::PlayerCharacter* a_caller);
    void Throw(const bool isVertical);
    void Call();
    void Catch(RE::Projectile* a_levi, RE::PlayerCharacter* a_player, const bool a_justDestroy = false);
    //  experimental:
    void Charge(const uint8_t a_chargeHitCount = 10u, const float a_magnitude = 1.5f, const uint8_t a_coolDown = 10u);
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false);
    void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical);
    void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical);
    void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical);

//  RE::Projectile::LaunchData* LeviThrowData   = nullptr;

    RE::Projectile* LeviathanAxeProjectileL = nullptr;
    RE::Projectile* LeviathanAxeProjectileH = nullptr;
    RE::Projectile* LeviathanAxeProjectileA = nullptr;
    RE::Projectile* LastLeviProjectile      = nullptr;

    RE::BGSProjectile* LeviProjBaseL    = nullptr;
    RE::BGSProjectile* LeviProjBaseH    = nullptr;
    RE::BGSProjectile* LeviProjBaseA    = nullptr;

    RE::SpellItem* SpellLeviProjL   = nullptr;
    RE::SpellItem* SpellLeviProjH   = nullptr;
    RE::SpellItem* SpellLeviProjA   = nullptr;
    RE::SpellItem* SpellCatchLevi   = nullptr;
    RE::SpellItem* SpellChargeCD    = nullptr;

    RE::EffectSetting* EffCatchLevi = nullptr;
    RE::EnchantmentItem* EnchCharge = nullptr;

    bool isAxeCalled;
    bool isAxeThrowed;
    bool isAxeStucked;
private:
    uint8_t chargeHitCount = 0;
    ThrowState throwState;

    LeviathanAxe()  = default;
    ~LeviathanAxe() = default;
};

/*  old
class Leviathan
{
public:
    static Leviathan* GetSingleton() {static Leviathan singleton; return &singleton;}

    enum class ThrowState : std::uint_fast8_t
    {
        kNone = 0,
        kArrived = 1,
        kThrowable = 2,
        kThrown = 3,
        kCanArrive = 4,
        kArriving = 5,
    };
    static inline ThrowState throwState;

    static inline RE::NiPoint3 leviPosition     = {0.f, 0.f, 0.f};
    static inline RE::NiNode* leviStuckedBone   = nullptr;
    static inline RE::Actor* leviStuckedActor   = nullptr;
    static inline RE::Actor* leviLastHitActor   = nullptr;
    static inline float throwedTime             = 0.f;
    static inline float arrivalSpeed            = Config::MinArrivalSpeed;

    static inline RE::Projectile::LaunchData* LeviThrowData = nullptr;

    static inline RE::Projectile* LeviathanAxeProjectileL   = nullptr;
    static inline RE::Projectile* LeviathanAxeProjectileH   = nullptr;
    static inline RE::Projectile* LeviathanAxeProjectileA   = nullptr;
    static inline RE::Projectile* LastLeviProjectile        = nullptr;

    static inline RE::BGSProjectile* LeviProjBaseL  = nullptr;  //  ligth attack throw
    static inline RE::BGSProjectile* LeviProjBaseH  = nullptr;  //  heavy attack throw
    static inline RE::BGSProjectile* LeviProjBaseA  = nullptr;  //  arriving leviathan
    static inline RE::SpellItem* SpellLeviProjL     = nullptr;  //  light throw
    static inline RE::SpellItem* SpellLeviProjH     = nullptr;  //  heavy throw
    static inline RE::SpellItem* SpellLeviProjA     = nullptr;  //  call levi

    static inline RE::SpellItem* SpellCatchLevi     = nullptr;
    static inline RE::EffectSetting* EffCatchLevi   = nullptr;

    static inline RE::EnchantmentItem* EnchCharge   = nullptr;

    static inline bool isAxeCalled  = false;
    static inline bool isAxeThrowed = false;
    static inline bool isAxeStucked = false;

    static void SetThrowState(const ThrowState a_throwState);
    static inline void SetStartPos(RE::NiPoint3& a_point, RE::PlayerCharacter* a_caller);
    static void Throw(bool isVertical);
    static void Arrive();
    static void Catch(RE::Projectile* a_levi, RE::PlayerCharacter* a_player) {return Catch(a_levi, a_player, false);}
    static void Catch(RE::Projectile* a_levi, RE::PlayerCharacter* a_player, bool a_justDestroy);
    static void Charge(const int a_DurationSec, const float a_Magnitude);
    static void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical);
    static void SetHitRotation(RE::NiPoint3& a_angles, const bool a_vertical);
};
*/
class Draupnir
{
public:
    static inline RE::SpellItem*        SpellDraupnirProjL          = nullptr;
    static inline RE::SpellItem*        SpellDraupnirsCallProjL     = nullptr;
    static inline RE::Projectile*       DraupnirSpearProjectiles[9];                            //  they will be explode after Draupnir's Call move
    static inline RE::Projectile*       DraupnirsCallProjectiles[9];                            //  hard to explain
    static inline RE::NiNode*           DraupnirSpearHitBones[9];                               //  keeps attached nodes of throwed draupnirs for forced detonation*
    static inline RE::Actor*            DraupnirSpearHitActors[9];                              //  keeps hitten actors
    static inline RE::Projectile*       DraupnirSpearProjectileL    = nullptr;
    static inline RE::BGSProjectile*    DraupnirSpearProjBaseL      = nullptr;
    static inline RE::BGSProjectile*    DraupnirsCallProjBaseL      = nullptr;

    static inline RE::BGSExplosion* StuckedDraupnir         = nullptr;
    static inline RE::BGSExplosion* DraupnirExplosion       = nullptr;

    static void Throw();
    static void Call(const float a_damage, const float a_force);

    /* forced detonation?
    forced detonation needed for living targets, because timing projectile explosions not working after hitting to actors.
    */
};

using EventChecker = RE::BSEventNotifyControl;
class AnimationEventTracker : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    static AnimationEventTracker* GetSingleton() {static AnimationEventTracker singleton; return &singleton;};

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;
};
class MagicEffectApplyTracker : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>
{
public:
    static MagicEffectApplyTracker* GetSingleton() {static MagicEffectApplyTracker singleton; return &singleton;};

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource) override;
};
/*
class InputEventTracker : public RE::BSTEventSink<RE::InputEvent*>
{
public:
    static InputEventTracker* GetSingleton() {static InputEventTracker singleton; return &singleton;}

    static void Register();

    virtual EventChecker ProcessEvent(RE::InputEvent* const *a_event, RE::BSTEventSource<RE::InputEvent*> *a_eventSource) override;

    static inline uint32_t  AimButton       = 0u;
    static inline uint32_t  AxeCallButton   = 0u;
};
*/

class SpellCastTracker  // : public RE::BSTEventSink<RE::TESSpellCastEvent>
{
public:
//  static SpellCastTracker* GetSingleton() {static SpellCastTracker singleton; return &singleton;};
//
//  static void Register();
//
//  virtual EventChecker ProcessEvent(const RE::TESSpellCastEvent* a_event, RE::BSTEventSource<RE::TESSpellCastEvent>* a_eventSource) override;

    static inline RE::SpellItem*        SpellAimButton      = nullptr;
    static inline RE::SpellItem*        SpellAxeCallButton  = nullptr;
    static inline RE::EffectSetting*    EffectAimButton     = nullptr;
    static inline RE::EffectSetting*    EffectAxeCallButton = nullptr;
};
