#pragma once
#include "util.h"
#include "settings.h"
#include "hook.h"

static bool _skipEquipAnim, _skipLoad3D;
static int _load3Ddelay;

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

    enum class Shield : std::uint8_t {
        kNone = 0u,
        kGuardianShield = 1u,
        kDauntlessShield = 2u,
        kStoneWallShield = 3u,
        kShatterStarShield = 4u,
        kOnslaughtShield = 5u,
    };

    enum class Action : std::uint8_t {
        kRage = 0u,
        kWeaponCharge = 1u,
        kSpecialIdle = 2u
    };

    enum class Rage : std::uint8_t {
        kFury = 0u,
        kValor = 1u,
        kWrath = 2u,
        kLegacy = 3u,
    };
//--------------------------
    struct VanillaActions {
        RE::BGSAction* normalAttack = nullptr;
        RE::BGSAction* powerAttack = nullptr;
        RE::BGSAction* dualPowerAttack = nullptr;
    };

    struct VFXeffects {
        RE::BGSArtObject* handFrost = nullptr;      //  FrostHandFX01 [ARTO:00020BF5], FrostBoltHandFX01 [ARTO:000334B9]
        RE::BGSArtObject* handFrostBright = nullptr;//  FXIceCloakHandEffects [ARTO:00042854],
        RE::BGSArtObject* handFlame = nullptr;      //  Fireball01HandEffects [ARTO:0001B211], FireCloakHandEffects [ARTO:00036342]
        RE::BGSArtObject* iceCloak = nullptr;       //  FXIceCloak01 [ARTO:0004253F]
        RE::BGSArtObject* fireCloak = nullptr;      //  FXFireCloak01 [ARTO:0002ACD7]
        RE::BGSArtObject* fury = nullptr;
        RE::BGSArtObject* legacy = nullptr;
        RE::BGSArtObject* valor = nullptr;
        RE::BGSArtObject* wrath = nullptr;
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
        RE::FormID strenghtBuff;
        RE::FormID spartanRage;
    };

    struct KratosValues {
        float* strength         = nullptr;
        float* defense          = nullptr;
        float* runic            = nullptr;
        float* vitality         = nullptr;
        float* cooldown         = nullptr;
        float* luck             = nullptr;
        float* rage             = nullptr;
        float* rageLimit        = nullptr;
        float* rageBuffAmount   = nullptr;
        float* rageDamageAmount = nullptr;
    };

    VanillaActions action;
    VFXeffects VFXeffect;
    SoundEffects soundEffect;
    ChargeData chargeData;
    SpellID spellID;
    KratosValues values;
//-------------------------- Functions
    void Update(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());

    float CalcRageDamageOrBuffAmount(const float a_amount, const float a_mult = 1.f);

    bool IsEquipped(const Relic a_relic) const;
    bool IsCanCallAxe(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) const;
    bool IsCanRage(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) const;
    bool IsInRage(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    bool IsWantFinishRage() const {return _isWantFinishRage;}
    bool IsCanCharge(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), Relic a_relic = Relic::kLeviathanAxe) const;
    bool IsShieldOpened() const;
    bool IsAiming() const {return _isAiming;}

    Kratos::Relic GetEquippedRelic() const;
    Kratos::Rage GetRageType() const;
    Kratos::Rage GetLastTriggeredRageType() const;

    void RestoreRage(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const float a_value = 0.f, const bool a_justRestore = false);

    void SetIsCanCallAxe(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const bool a_isCan = true);
    void SetIsCanRage(const bool a_isCan = true, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void SetIsCanCharge(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const bool a_isCan = true, Relic a_relic = Relic::kLeviathanAxe); // 1: levi, 2: blade of chaos, 3: draupnir
    void Aim(const bool a_startAim) {_isAiming = a_startAim;}

    void OpenShield(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void CloseShield(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void StartRage(const Rage a_rage = Rage::kFury, const bool a_justAnim = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void EndRage(const Rage a_rage = Rage::kFury, const bool a_fromAnnotation = false, const bool a_playAnim = true, const bool a_justAnim = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void DoKratosAction(const Action a_action, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
//--------------------------
    RE::SpellItem*      SpellAxeThrownState = nullptr;
    RE::SpellItem*      SpellSpartanRage    = nullptr;
    RE::SpellItem*      SpellStrenghtBuff   = nullptr;
    RE::SpellItem*      SpellLeviChargeCD   = nullptr;
    RE::EffectSetting*  EffectAxeThrownState= nullptr;
    RE::EffectSetting*  EffectSpartanRage   = nullptr;
    RE::EffectSetting*  EffectStrenghtBuff  = nullptr;
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

    RE::TESGlobal* gLeviathanAxeFormID   = nullptr;
    RE::TESGlobal* gBladeOfChaosFormID   = nullptr;
    RE::TESGlobal* gDraupnirSpearFormID  = nullptr;
    RE::TESGlobal* gBladeOfOlympusFormID = nullptr;
    RE::TESGlobal* gGuardianShieldFormID = nullptr;
private:
friend class AttackHook;
    bool _isShieldOpened;
    bool _isInRage;
    bool _isWantFinishRage;
    bool _isAiming;
    Rage _lastTriggeredRage;
    bool _gettingHittedInValor;
    RE::TESBoundObject* _LastEquippedObjectR = nullptr;
    RE::TESBoundObject* _LastEquippedObjectL = nullptr;
    Kratos() = default;
    ~Kratos() = default;
};
class WeaponIdentify
{
public:
    static inline bool isLeviathanAxe;
    static inline bool isBladeOfChaos;
    static inline bool isDraupnirSpear;
    static inline bool isBladeOfOlympus;
    static inline bool isGuardianShield;
    static inline bool isRelic;
    static inline bool isKratos;
    static inline bool isBarehanded;

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
    static inline RE::TESBoundObject* EquippedObjR  = nullptr;
    static inline RE::TESBoundObject* EquippedObjL  = nullptr;

    static inline float DamageMult = 1.f;

    static bool IsRelic(RE::Projectile *a_proj, const bool a_onlyLevi = false, const bool a_onlyDraupnir = false)           {return IsRelic(a_proj->GetProjectileBase(), a_onlyLevi, a_onlyDraupnir);}
    static bool IsRelic(RE::BGSProjectile *a_baseProj, const bool a_onlyLevi = false, const bool a_onlyDraupnir = false);

    static void Initialize(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    static void WeaponCheck(const bool a_specialityCheck = false);
    static void SpecialityCheck(RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectARMO* a_shield, const Kratos::Relic a_relic, const Kratos::Shield a_specialShield);
private:
    static void WeaponIdentifier(RE::Actor* a_actor, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectARMO* a_shield);
};
/**/

class LeviathanAxe
{
public:
    static LeviathanAxe* GetSingleton();// {static LeviathanAxe singleton; return &singleton;}
    bool Initialize();

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
        kThrowable = 1,
        kThrown = 2,
        kCanArrive = 3,
        kArriving = 4,
        kArrived = 5
    };

    Data data;

    void SetThrowState(const ThrowState a_throwState);
    ThrowState GetThrowState() const;
    void SetStartPos(RE::NiPoint3& a_point, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Throw(const bool isVertical, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Call(const bool a_justDestroy = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Catch(bool a_justDestroy = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    //  experimental:
    void Charge(const uint8_t a_chargeHitCount = 10u, const float a_magnitude = 1.5f, const uint8_t a_coolDown = 10u);
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false);
    void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical);
    void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical);
    void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical);

    bool isAxeCalled;
    bool isAxeThrowed;
    bool isAxeStucked;
//  RE::Projectile::LaunchData* LeviThrowData   = nullptr;

private:
friend class WeaponIdentify;
friend class AnimationEventTracker;
friend class ProjectileHook;

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

    uint8_t chargeHitCount = 0;
    ThrowState throwState;

    LeviathanAxe()  = default;
    ~LeviathanAxe() = default;
};
class BladeOfChaos
{
public:
    static  BladeOfChaos* GetSingleton() {static BladeOfChaos singleton; return &singleton;}

    void    Update() {_lastChargeTime = AsyncUtil::GameTime::GetEngineTime();}
    bool    IsScorching() const {return _isScorching;}
    void    SetIsScorching(const bool a_isScorching = true) {_isScorching = a_isScorching;}
    float   GetScorchingSpeed();
    void    SetScorchingSpeed(const float a_speed, const bool a_forced = false);
    void    BuffScorchingSpeed(const float a_buff = 0.05f, const bool a_forced = false);
    void    DeBuffScorchingSpeed();
    bool    IsQueueEnd();
private:
    bool    _isScorching = false;
    float   _fScorchingSpeed = 0.5f;
    float   _lastChargeTime;
};
class Draupnir
{
public:
    static Draupnir* GetSingleton() {static Draupnir singleton; return &singleton;}
    bool Initialize();

    static inline RE::SpellItem*        SpellDraupnirProjL          = nullptr;
    static inline RE::SpellItem*        SpellDraupnirsCallProjL     = nullptr;
    static inline RE::Projectile*       DraupnirSpearProjectiles[9];                            //  they will be explode after Draupnir's Call move
    static inline RE::Projectile*       DraupnirsCallProjectiles[9];                            //  hard to explain
    static inline RE::NiNode*           DraupnirSpearHitBones[9];                               //  keeps attached nodes of throwed draupnirs for forced detonation*
    static inline RE::Actor*            DraupnirSpearHitActors[9];                              //  keeps hitten actors
    static inline RE::Projectile*       DraupnirSpearProjectileL    = nullptr;
    static inline RE::BGSProjectile*    DraupnirSpearProjBaseL      = nullptr;
    static inline RE::BGSProjectile*    DraupnirsCallProjBaseL      = nullptr;

    static inline RE::BGSExplosion* StuckedDraupnir     = nullptr;
    static inline RE::BGSExplosion* DraupnirExplosion   = nullptr;
    static inline RE::BGSArtObject* DraupnirStuckedFX   = nullptr;

    static void Update();
    static void Throw();
    static void MeleeThrow();
    static void ArtilleryOfTheAncients(const float a_delay, const float a_duration);
    static void RainOfDraupnir();
    static void AddSpearHit(RE::NiNode* a_bone, RE::Actor* a_actor = nullptr);
    static void AddSpearHit(RE::Projectile* a_proj);
    static void Call(const float a_damage, const float a_force);
    static void StartExplosions(const float a_delay);

    /* forced detonation?
    forced detonation needed for living targets, because timing projectile explosions not working after hitting to actors.
    */
private:
friend class ProjectileHook;
    static inline float nextExplosionTime = 0.f;
    static inline float timeToDoneExplosions = 0.f;
    static inline float explosionDelay = 0.f;
    static inline size_t currentHitIndex = 0;
    static inline bool explosionsStarted = false;
    static inline RE::NiAVObject* spearModel = nullptr;
    static inline std::vector<RE::FormID> MeleeHitProjectileIDs;
    static inline RE::BSFixedString DefaultDraupnirModel;
    static inline std::vector<std::tuple<RE::NiNode*, RE::Actor*, RE::Projectile*>> spearHits;
    static void TriggerExplosions(float a_delay, float a_force, RE::ProjectileHandle* a_pHandle);
    static void TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target);
    static void TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle);
    
    static inline float nextLaunchTime = 0.0f;
    static inline float nextLaunchDelay = 0.0f;
    static inline float lastLaunchTime = 0.0f;
    static inline bool draupnirRainStarted = false;
};


using EventChecker = RE::BSEventNotifyControl;
class AnimationEventTracker : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
{
public:
    static AnimationEventTracker* GetSingleton() {static AnimationEventTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) override;
};
class MagicEffectApplyTracker : public RE::BSTEventSink<RE::TESMagicEffectApplyEvent>
{
public:
    static MagicEffectApplyTracker* GetSingleton() {static MagicEffectApplyTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::TESMagicEffectApplyEvent* a_event, RE::BSTEventSource<RE::TESMagicEffectApplyEvent>* a_eventSource) override;
};
class InputEventTracker : public RE::BSTEventSink<RE::InputEvent*>
{
public:
    static InputEventTracker* GetSingleton() {static InputEventTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;
private:
    enum : std::uint32_t
    {
        kInvalid = static_cast<std::uint32_t>(-1),
        kKeyboardOffset = 0,
        kMouseOffset = 256,
        kGamepadOffset = 266,
    };
    std::uint32_t GetOffsettedKeyCode(std::uint32_t a_keyCode, RE::INPUT_DEVICE a_inputDevice) const;
    InputEventTracker() = default;
    InputEventTracker(const InputEventTracker&) = delete;
    InputEventTracker(InputEventTracker&&) = delete;
    virtual ~InputEventTracker() = default;
    InputEventTracker& operator=(const InputEventTracker&) = delete;
    InputEventTracker& operator=(InputEventTracker&&) = delete;
    std::uint32_t GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key);
};
class HitEventTracker : public RE::BSTEventSink<RE::TESHitEvent>
{
public:
    static MagicEffectApplyTracker* GetSingleton() {static MagicEffectApplyTracker singleton; return &singleton;}

    static bool Register();

    virtual EventChecker ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) override;
};
inline bool RegisterEvents() 
{
    return !(
        !AnimationEventTracker::Register() ||
        !MagicEffectApplyTracker::Register() ||
        !InputEventTracker::Register()// ||
    //    HitEventTracker::Register()
    );
}