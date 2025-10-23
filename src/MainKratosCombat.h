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
        kNone = 0u,             //  or any
        kLeviathanAxe = 1u,
        kBladeOfChaos = 2u,
        kDraupnirSpear = 3u,
        kBladeOfOlympus = 4u,
        kMjolnir = 5u,
        kTrident = 6u,
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
        RE::BGSArtObject* frostCloak = nullptr;       //  FXIceCloak01 [ARTO:0004253F]
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
        RE::BGSSoundDescriptorForm* chargeLeviLoop = nullptr;// MAGFrostBiteFireLPMSD [SNDR:0003E5CB]
        RE::BGSSoundDescriptorForm* chargeLeviEndT = nullptr;
        RE::BGSSoundDescriptorForm* chargeLeviEnd = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
        RE::BGSSoundDescriptorForm* catchMjolnir = nullptr; //  WPNImpactBladeVsIceSD [SNDR:0002398A]
        RE::BGSSoundDescriptorForm* callMjolnir = nullptr;  //  telekinesis,0x7D013
        RE::BGSSoundDescriptorForm* fingerSnap = nullptr;   //  recorded by me with whatsapp voice message
        RE::BGSSoundDescriptorForm* chargeMjolnir = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
        RE::BGSSoundDescriptorForm* chargeMjolnirEnd = nullptr;//  MAGIcicleFire2DSD [SNDR:0003EAC6]
    };

    struct ChargeData {
        RE::FormID lastBuffedLevi;
        RE::FormID lastChargedLevi;
        RE::FormID lastBuffedBoC;
        RE::FormID lastChargedBoC;
        RE::FormID lastBuffedDraup;
        RE::FormID lastChargedDraup;
        RE::FormID lastBuffedMjolnir;
        RE::FormID lastChargedMjolnir;
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
    VanillaSpells vanillaSpell;
    KittyTailSpells kittyTailSpell;
    VFXeffects VFXeffect;
    SoundEffects soundEffect;
    ChargeData chargeData;
    SpellID spellID;
    KratosValues values;
//-------------------------- Functions
    void Update(RE::Actor* a_actor, const float a_delta);

    float CalcRageDamageOrBuffAmount(const float a_amount, const float a_mult = 1.f);

    bool IsEquipped(const Relic a_relic) const;
    bool IsCanCallAxe(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) const;
    bool IsCanCallMjolnir(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) const;
    bool IsCanRage(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton()) const;
    bool IsInRage(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    bool IsWantFinishRage() const {return _isWantFinishRage;}
    bool IsCanCharge(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), Relic a_relic = Relic::kLeviathanAxe) const;
    bool IsChargingThrow() const {return _isChargingThrow;}
    bool IsShieldOpened() const;
    bool IsAiming() const {return _isAiming;}

    Kratos::Relic GetEquippedRelic() const;
    Kratos::Relic GetLastEquippedRelic() const;
    Kratos::Rage GetRageType() const;
    Kratos::Rage GetLastTriggeredRageType() const;
    Kratos::Relic GetNextWeaponToCall() const;

    void RestoreRage(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const float a_value = 0.f, const bool a_justRestore = false);

    void SetIsCanCallAxe(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const bool a_isCan = true);
    void SetIsCanCallMjolnir(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const bool a_isCan = true);
    void SetIsCanRage(const bool a_isCan = true, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void SetIsCanCharge(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const bool a_isCan = true, Relic a_relic = Relic::kLeviathanAxe); // 1: levi, 2: blade of chaos, 3: draupnir, 5: mjolnir
    void SetIsChargingThrow(const bool a_isCharging = true) {
        _isChargingThrow = a_isCharging;
        if (_isChargingThrow) {
            spdlog::debug("Charging throw is started");
        } else {
            _soundHandle.Stop();
            spdlog::debug("Charging throw is ended");
        }
    }
    void Aim(const bool a_startAim) {_isAiming = a_startAim;}

    void OpenShield(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void CloseShield(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void StartRage(const Rage a_rage = Rage::kFury, const bool a_justAnim = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void EndRage(const Rage a_rage = Rage::kFury, const bool a_fromAnnotation = false, const bool a_playAnim = true, const bool a_justAnim = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void DoKratosAction(const Action a_action, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
//--------------------------
    RE::SpellItem*      SpellAxeThrownState = nullptr;
    RE::SpellItem*      SpellMjolnirThrownState = nullptr;
    RE::SpellItem*      SpellSpartanRage    = nullptr;
    RE::SpellItem*      SpellStrenghtBuff   = nullptr;
    RE::SpellItem*      SpellLeviChargeCD   = nullptr;
    RE::SpellItem*      SpellMjolnirChargeCD= nullptr;
    RE::EffectSetting*  EffectAxeThrownState= nullptr;
    RE::EffectSetting*  EffectSpartanRage   = nullptr;
    RE::EffectSetting*  EffectStrenghtBuff  = nullptr;
    RE::EffectSetting*  EffectLeviChargeCD  = nullptr;
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
    RE::BSSoundHandle _soundHandle;
private:
friend class AttackHook;
    bool _isShieldOpened;
    bool _isInRage;
    bool _isWantFinishRage;
    bool _isChargingThrow;
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
    static inline bool isMjolnir;
    static inline bool isTrident;
    static inline bool isGuardianShield;
    static inline bool isRelic;
    static inline bool isKratos;
    static inline bool isThor;
    static inline bool isPoseidon;
    static inline bool isBarehanded;

    static inline Kratos::Relic lastEquippedRelic;
    static inline Kratos::Relic lastThrownRelic;

    static inline bool skipEquipAnim;
    static inline bool unequipWhenAnimEnds;

    static inline RE::NiAVObject* RHandBone         = nullptr;
    static inline RE::NiAVObject* LHandBone         = nullptr;
    static inline RE::NiAVObject* WeaponBone        = nullptr;
    static inline RE::NiAVObject* ShieldBone        = nullptr;
    static inline RE::TESObjectWEAP* LeviathanAxe   = nullptr;
    static inline RE::TESObjectWEAP* BladeOfChaos   = nullptr;
    static inline RE::TESObjectWEAP* DraupnirSpear  = nullptr;
    static inline RE::TESObjectWEAP* BladeOfOlympus = nullptr;
    static inline RE::TESObjectWEAP* Mjolnir        = nullptr;
    static inline RE::TESObjectWEAP* Trident        = nullptr;
    static inline RE::TESObjectARMO* GuardianShield = nullptr;
    static inline RE::TESBoundObject* EquippedObjR  = nullptr;
    static inline RE::TESBoundObject* EquippedObjL  = nullptr;

    static inline float DamageMult = 1.f;

    static bool IsRelic(RE::Projectile *a_proj, const Kratos::Relic a_relic = Kratos::Relic::kNone) {return IsRelic(a_proj->GetProjectileBase(), a_relic);}
    static bool IsRelic(RE::BGSProjectile *a_baseProj, const Kratos::Relic a_relic = Kratos::Relic::kNone);

    static void Initialize(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    static void ResetRegistrations();
    static void WeaponCheck(const bool a_specialityCheck = false);
    static void SpecialityCheck(RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectWEAP* a_LHandWeapon, RE::TESObjectARMO* a_shield, const Kratos::Relic a_relic, const Kratos::Shield a_specialShield);
private:
    static void WeaponIdentifier(RE::Actor* a_actor, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectWEAP* a_LHandWeapon, RE::TESObjectARMO* a_shield);
};

class LeviathanAxe
{
public:
    static LeviathanAxe* GetSingleton();// {static LeviathanAxe singleton; return &singleton;}
    bool Initialize();

    enum class ThrowState : std::uint_fast8_t {
        kNone = 0,
        kThrowable = 1,
        kThrown = 2,
        kCanArrive = 3,
        kArriving = 4,
        kArrived = 5
    };
    enum class ProjectileState : std::uint_fast8_t {
        kNone = 0,
        kLaunched = 1,
        kStucked = 2,
        kHavok = 3
    };

    struct Data {
        RE::Projectile* proj;
        RE::TESObjectWEAP* weap     = nullptr;
        RE::EnchantmentItem* ench   = nullptr;
        RE::AlchemyItem* poison     = nullptr;
        RE::NiPoint3 position       = {0.f, 0.f, 0.f};
        RE::NiPoint3 lastVelocity   = {0.f, 0.f, 0.f};
        RE::NiPoint3 lastOrientation= {0.f, 0.f, 0.f};
        RE::NiAVObject* model       = nullptr;
        RE::NiAVObject* weaponModel = nullptr;
        std::vector<RE::NiPointer<RE::BSTempEffectParticle>> projTrails;
        RE::NiTransform trailTransform;
        std::vector<RE::Actor*>         lastHitActors;  //  keeps last 3 hit actor from the last throw
        std::vector<RE::TESObjectREFR*> lastHitForms;   //  keeps last 3 hit object from the last throw
        RE::NiNode* stuckedBone     = nullptr;
        RE::Actor* stuckedActor     = nullptr;
        ProjectileState projState   = ProjectileState::kNone;
        float* enchMag              = nullptr;
        float defaultEnchMag;
        float damage            = 0.f;
        float yAngle            = 0.35f;
        float throwedTime       = 0.f;
        float rotationSpeed     = Config::ThrowRotationSpeed; //  rad/s
        float gravity           = 3.69f;
        float throwingChargeDuration = 0.f;
    };

    struct ArrivingLeviathan {
        const LeviathanAxe* parent;
        RE::Projectile* proj;
        RE::Actor* caller;
        RE::NiAVObject* callerBone;
        RE::NiPoint3 startPosition;
        RE::NiPoint3 currentDir;
        RE::NiPoint3 desiredDir;
        RE::NiPoint3 linearArrivingDir;
        std::pair<std::vector<RE::NiPoint3>, float> arrivingRoute;
        std::vector<RE::Actor *> targets;
        float throwedTime = 0.f;
        float livingTime = 0.f;
        float timeToArrive = Config::ArrivalTime;
        float linearDistance = 0.f;
        float linearDistanceFromStart = 0.f;
        float linearDistanceFromLastCallPos = 0.f;
        float arrivingRelativeAngleZ = 0.f;
        float speed = 2400.f;

        float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - throwedTime;}
        std::vector<RE::Actor*> GetTargets(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
            targets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(caller, linearDistance, true);
            CheckTargets(a_origin.has_value() ? *a_origin : startPosition);
            return targets;
        }
        void CheckTargets(RE::NiPoint3& a_origin) {
            if (!targets.empty()) {
                std::erase_if(targets, [=](const RE::Actor* actor) {
                    bool result = false;
                    if (!actor || actor->IsDead() || !parent || std::find(parent->data.lastHitActors.begin(), parent->data.lastHitActors.end(), actor) != parent->data.lastHitActors.end()) {
                        result = true;
                    } else {
                        auto targetPos = actor->GetPosition() + (actor->GetBoundMax() + actor->GetBoundMin()) * 0.75f;
                        auto targetDir = targetPos - a_origin;
                        targetDir.Unitize();
                        float distance = actor->GetPosition().GetDistance(a_origin);
                        result = distance < 100.f || 
                            distance > linearDistance || 
                            currentDir.Dot(targetDir) > std::cos(PI8) || 
                            currentDir.Dot(linearArrivingDir) > std::cos(PI8);
                    }
                    return result;
                });
                std::sort(targets.begin(), targets.end(), 
                    [&](const auto& a, const auto& b) {
                        return a->GetPosition().GetDistance(a_origin) < 
                            b->GetPosition().GetDistance(a_origin);
                    }
                );
            };
        }

        RE::Actor* GetNextTarget(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
            if (a_origin) {
                GetTargets(a_origin);
            }
            return !targets.empty() ? targets.front() : nullptr;
        }

        virtual ~ArrivingLeviathan() = default;
        ArrivingLeviathan() = default;
        ArrivingLeviathan(const LeviathanAxe* a_parent, RE::Projectile* a_proj, 
            RE::Actor* a_caller, 
            RE::NiAVObject* a_callerBone, 
            RE::NiPoint3& a_startPosition) : parent(a_parent), proj(a_proj),
            caller(a_caller), callerBone(a_callerBone), startPosition(a_startPosition)
        {
            throwedTime = AsyncUtil::GameTime::GetEngineTime();
            auto callerPosition = callerBone ? callerBone->world.translate : caller ? caller->GetPosition() : RE::NiPoint3();
            linearArrivingDir = (callerPosition - startPosition);
            linearArrivingDir.Unitize();
            linearDistanceFromStart = startPosition.GetDistance(callerPosition) + 1.f;
            linearDistanceFromLastCallPos = linearDistanceFromStart;
            speed = linearDistanceFromStart / timeToArrive;
            speed = std::clamp(speed, Config::MinArrivalSpeed, Config::MaxArrivalSpeed);
            timeToArrive = linearDistanceFromStart / speed;
            targets = GetTargets();
        }
    };

    struct HomingLeviathan {
        RE::Projectile* proj;
        std::vector<RE::Actor *> targets;
        RE::Actor* shooter;
        uint8_t hitCount;           //  how many times it will hit the targets?
        bool isBoomerang;           //  it will come back after hit all targets?
        float speed;
        float throwedTime;
        float angularVelocity;
        float waveAmplitude = 400.f;
        float waveFrequency = 20.f;

        float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - throwedTime;}
        //  give an origin point if you want to get nearest target
        RE::Actor* GetNextTarget(RE::NiPoint3 a_origin = RE::NiPoint3()) {
            std::erase_if(targets, [](const RE::Actor* actor) {
                return !actor || (actor && actor->IsDead());
            });
    //        if (targets.size() > 2u) {
    //            if (a_origin != RE::NiPoint3()) {
    //                std::sort(targets.begin(), targets.end(), 
    //                    [&](const auto& a, const auto& b) {
    //                        return a->GetPosition().GetDistance(a_origin) < 
    //                               b->GetPosition().GetDistance(a_origin);
    //                    }
    //                );
    //            } else {
    //                std::vector<RE::Actor*> sortedTargets;
    //                sortedTargets.push_back(targets.front());
    //                targets.erase(targets.begin());
    //                while (!targets.empty()) {
    //                    auto closestIt = std::min_element(targets.begin(), targets.end(), 
    //                        [&](const auto& a, const auto& b) {
    //                            return a->GetPosition().GetDistance(targets.back()->GetPosition()) < 
    //                                   b->GetPosition().GetDistance(targets.back()->GetPosition());
    //                        }
    //                    );
    //                    sortedTargets.push_back(*closestIt);
    //                    targets.erase(closestIt);
    //                }
    //                targets = std::move(sortedTargets);
    //            }
//
    //            for (size_t i = 0; i < targets.size() - 1; i++) {
    //                if (targets[i] == targets[i + 1]) {
    //                    RE::Actor* duplicate = targets[i];
    //                    targets.erase(targets.begin() + i);
    //                    targets.push_back(duplicate);
    //                }
    //            }
    //        }
            return !targets.empty() ? targets.front() : nullptr;
        }

        virtual ~HomingLeviathan() = default;
        HomingLeviathan() = default;
        HomingLeviathan(RE::Projectile* a_proj, 
            std::vector<RE::Actor *> a_targets, 
            RE::Actor* a_shooter, 
            const uint8_t a_hitCount = 1u, 
            bool a_isBoomerang = true, 
            float a_speed = 2000.f, 
            float a_angularVelocity = 5.f) : proj(a_proj), 
            targets(a_targets), 
            shooter(a_shooter), 
            hitCount(a_hitCount), 
            isBoomerang(a_isBoomerang), 
            speed(a_speed), 
            angularVelocity(a_angularVelocity)
        {
            throwedTime = AsyncUtil::GameTime::GetEngineTime();
            if (targets.size() == 1u) {hitCount = 4u;}
            else if (hitCount < 1u) hitCount = 1u;
            if (!targets.empty()) {
                spdlog::debug("homing leviathan have {} times {} combat targets", hitCount, targets.size());

                size_t originalSize = targets.size();
                targets.resize(originalSize * hitCount);

                for (size_t i = 1; i < hitCount; i++) {
                    std::copy_n(targets.begin(), originalSize, targets.begin() + (i * originalSize));
                }
            }
        }
    };

    Data data;
    ArrivingLeviathan arrivingLevi;
    HomingLeviathan homingLevi;

    void Update(const float a_delta);
    void SetThrowState(const ThrowState a_throwState);
    ThrowState GetThrowState() const;
    void GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Throw(const bool isVertical, const bool justContinue = false, const bool isHoming = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Call(const bool a_justDestroy = false, const bool a_justContinue = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Catch(bool a_justDestroy = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    //  experimental:
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_coolDown = 15u);
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false);
    void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical);
    void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical);
    void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical);
    bool IsArriving(RE::Projectile* a_proj) const;
    bool IsHoming(RE::Projectile* a_proj) const;
    bool IsCharged() const {return _isCharged;}
    void StartChargingThrow(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());

    bool isAxeCalled;
    bool isAxeThrowed;
    bool isAxeStucked;
//  RE::Projectile::LaunchData* LeviThrowData   = nullptr;

    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
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

    bool _isCharged = false;
    uint8_t chargeHitCount = 0;
    ThrowState throwState;
    PRECISION_API::CollisionDefinition collisionDefinition;

    LeviathanAxe()  = default;
    ~LeviathanAxe() = default;
};
class BladeOfChaos
{
public:
    static  BladeOfChaos* GetSingleton() {static BladeOfChaos singleton; return &singleton;}

    void    Update(const float a_delta) {_lastChargeTime = AsyncUtil::GameTime::GetEngineTime();}
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

    struct Data {
        float gravity = 2.69f;
        float throwingChargeDuration = 0.f;
    };

    static inline Data data;

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

    static void Update(const float a_delta);
    static void Throw();
    static void MeleeThrow();
    static void ArtilleryOfTheAncients(const float a_delay, const float a_duration);
    static void RainOfDraupnir();
    static void AddSpearHit(RE::NiNode* a_bone, RE::Actor* a_actor = nullptr);
    static void AddSpearHit(RE::Projectile* a_proj);
    static void Call(const float a_damage, const float a_force);
    static void StartExplosions(const float a_delay);
    static void SetExplosionMagnitude(const float a_magnitude) {explosionMagnitude = a_magnitude;}
    static void StartChargingThrow(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());

    /* forced detonation?
    forced detonation needed for living targets, because timing projectile explosions not working after hitting to actors.
    */
private:
friend class ProjectileHook;
    static inline float explosionMagnitude = 1.f;
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
class Mjolnir
{
public:
    static Mjolnir* GetSingleton();// {static Mjolnir singleton; return &singleton;}
    bool Initialize();

    enum class ThrowState : std::uint_fast8_t {
        kNone = 0,
        kThrowable = 1,
        kThrown = 2,
        kCanArrive = 3,
        kArriving = 4,
        kArrived = 5
    };
    enum class ProjectileState : std::uint_fast8_t {
        kNone = 0,
        kLaunched = 1,
        kStucked = 2,
        kHavok = 3
    };

    struct Data {
        RE::TESObjectWEAP* weap     = nullptr;
        RE::EnchantmentItem* ench   = nullptr;
        RE::AlchemyItem* poison     = nullptr;
        RE::NiPoint3 position       = {0.f, 0.f, 0.f};
        RE::NiPoint3 lastEulerAngles= {0.f, 0.f, 0.f};
        RE::NiPoint3 lastVelocity   = {0.f, 0.f, 0.f};
        RE::NiPoint3 lastOrientation= {0.f, 0.f, 0.f};
        RE::NiAVObject* model       = nullptr;
        std::vector<RE::NiPointer<RE::BSTempEffectParticle>> projTrails;
        RE::NiTransform trailTransform;
        std::vector<RE::Actor*>         lastHitActors;  //  keeps last 3 hit actor from the last throw
        std::vector<RE::TESObjectREFR*> lastHitForms;   //  keeps last 3 hit object from the last throw
        ProjectileState projState   = ProjectileState::kNone;
        float* enchMag              = nullptr;
        float defaultEnchMag;
        float damage            = 0.f;
        float yAngle            = 0.35f;
        float throwedTime       = 0.f;
        float gravity           = 4.f;
        float throwingChargeDuration = 0.f;
        bool isPenetrating;
    };

    struct ArrivingMjolnir {
        const Mjolnir* parent;
        RE::Projectile* proj;
        RE::Actor* caller;
        RE::NiAVObject* callerBone;
        RE::NiPoint3 startPosition;
        RE::NiPoint3 startVelocity = {0.f, 0.f, 0.f};
        RE::NiPoint3 lastVelocity  = {0.f, 0.f, 0.f};
        RE::NiPoint3 currentDir;
        RE::NiPoint3 desiredDir;
        RE::NiPoint3 desiredVelocity;
        RE::NiPoint3 linearArrivingDir;
        std::pair<std::vector<RE::NiPoint3>, float> arrivingRoute;
        std::vector<RE::Actor *> targets;
        float callTime      = 0.f;
        float launchTime    = 0.f;
        float timeToArrive  = 0.f;
        float linearDistance = 0.f;
        float linearDistanceFromStart = 0.f;
        float linearDistanceFromLastCallPos = 0.f;
        float arrivingRelativeAngleZ = 0.f;
        float speed = 2400.f;
        uint16_t routeResolution;

        float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - (launchTime > callTime ? launchTime : callTime);}
        std::vector<RE::Actor*> GetTargets(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
            targets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(caller, linearDistance, true);
            CheckTargets(a_origin.has_value() ? *a_origin : startPosition);
            return targets;
        }
        void CheckTargets(RE::NiPoint3& a_origin) {
            if (!targets.empty()) {
                std::erase_if(targets, [=](const RE::Actor* actor) {
                    bool result = false;
                    if (!actor || actor->IsDead() || !parent || std::find(parent->data.lastHitActors.begin(), parent->data.lastHitActors.end(), actor) != parent->data.lastHitActors.end()) {
                        result = true;
                    } else {
                        auto targetPos = actor->GetPosition() + (actor->GetBoundMax() + actor->GetBoundMin()) * 0.75f;
                        auto targetDir = targetPos - a_origin;
                        targetDir.Unitize();
                        float distance = actor->GetPosition().GetDistance(a_origin);
                        result = distance < 100.f || 
                            distance > linearDistance || 
                            currentDir.Dot(targetDir) > std::cos(PI8) || 
                            currentDir.Dot(linearArrivingDir) > std::cos(PI8);
                    }
                    return result;
                });
                std::sort(targets.begin(), targets.end(), 
                    [&](const auto& a, const auto& b) {
                        return a->GetPosition().GetDistance(a_origin) < 
                            b->GetPosition().GetDistance(a_origin);
                    }
                );
            };
        }

        RE::Actor* GetNextTarget(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
            if (a_origin) {
                GetTargets(a_origin);
            }
            return !targets.empty() ? targets.front() : nullptr;
        }

        virtual ~ArrivingMjolnir() = default;
        ArrivingMjolnir() = default;
        ArrivingMjolnir(RE::Actor* a_caller, 
            RE::NiAVObject* a_callerBone, 
            RE::NiPoint3& a_startPosition) : caller(a_caller), callerBone(a_callerBone), startPosition(a_startPosition)
        {
            callTime = AsyncUtil::GameTime::GetEngineTime();
            auto callerPosition = callerBone ? callerBone->world.translate : caller ? caller->GetPosition() : RE::NiPoint3();
            linearArrivingDir = (callerPosition - startPosition);
            linearArrivingDir.Unitize();
            linearDistanceFromLastCallPos = startPosition.GetDistance(callerPosition);
            linearDistanceFromStart = linearDistanceFromLastCallPos;
            float blendTime = (Config::MjolnirArrivingDelay.has_value() ? *Config::MjolnirArrivingDelay : 0.f);
            speed = (linearDistanceFromStart - Config::CatchingTreshold - 100.f) / (Config::ArrivalTime + blendTime);
            if (speed < Config::MinArrivalSpeed) speed = Config::MinArrivalSpeed;
            if (speed > Config::MaxArrivalSpeed) speed = Config::MaxArrivalSpeed;
            timeToArrive = linearDistanceFromStart / speed;

            if (auto spineNode = a_caller->GetNodeByName("NPC Spine2 [Spn2]"); spineNode && arrivingRelativeAngleZ != 0.5f) {
                auto spineForwardDir = spineNode->world.rotate * RE::NiPoint3(frontVec);
                spineForwardDir.z = 0.f;  //  ignore vertical direction
                spineForwardDir.Unitize();
                RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
                float dot = spineForwardDir.Dot(linearDir2D);
                float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;

                float arrivingRelativeAngle = atan2(det, dot);  //  angle between spine forward direction and axe direction
                arrivingRelativeAngle = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngle);  //  normalize angle to [0, PI]
                arrivingRelativeAngle = MathUtil::Angle::RadianToDegree(arrivingRelativeAngle) / 360.f;  //  normalize angle to [0, 1]
                std::vector<float> targets = {0.f, 0.25f, 0.5f, 0.75f, 1.f};
                float snapStrength = Config::ArrivalAngleSnap;
                if ((arrivingRelativeAngle < 0.124f || arrivingRelativeAngle > 0.876f) && snapStrength < 0.8f) snapStrength += 0.3f;
                arrivingRelativeAngle = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngle, targets, snapStrength);
                arrivingRelativeAngleZ = arrivingRelativeAngle;
                a_caller->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngleZ);
            }
        }
        ArrivingMjolnir(const Mjolnir* a_parent, RE::Projectile* a_proj, 
            RE::Actor* a_caller, 
            RE::NiAVObject* a_callerBone, 
            RE::NiPoint3& a_startPosition) : parent(a_parent), proj(a_proj),
            caller(a_caller), callerBone(a_callerBone), startPosition(a_startPosition)
        {
            launchTime = AsyncUtil::GameTime::GetEngineTime();
            auto callerPosition = callerBone ? callerBone->world.translate : caller ? caller->GetPosition() : RE::NiPoint3();
            linearArrivingDir = (callerPosition - startPosition);
            linearArrivingDir.Unitize();
            linearDistanceFromLastCallPos = linearDistanceFromStart = startPosition.GetDistance(callerPosition);
            RE::NiMatrix3 handRot   = callerBone ? callerBone->world.rotate : RE::NiMatrix3();
            RE::NiPoint3 palmDir    = handRot * RE::NiPoint3(backVec);
            RE::NiPoint3 handForward= handRot * RE::NiPoint3(upVec);
            const float handSideOffsetMult = 0.3f;

            RE::NiPoint3 p0 = startPosition;
            RE::NiPoint3 p3 = callerPosition;
            RE::NiPoint3 p1 = p0 + linearArrivingDir * linearDistanceFromStart / 3.f;
            RE::NiPoint3 p2 = p3 + palmDir * (linearDistanceFromLastCallPos / 3.f + 20.f) + handForward * (linearDistanceFromLastCallPos * handSideOffsetMult + 10.f);
            routeResolution = static_cast<uint16_t>(Config::ArrivalTime / *g_deltaTime);
            arrivingRoute = MathUtil::Algebra::DrawAndMeasureBezier(p0, p1, p2, p3, routeResolution);
            auto resolutionDistance = arrivingRoute.first[0].GetDistance(arrivingRoute.first[1]);
            auto speedLimit = resolutionDistance / *g_deltaTime;
            speedLimit -= 1.f;
            spdlog::debug("arrival route resolution is {}, speed limit is {}", routeResolution, speedLimit);
            speed = arrivingRoute.second / Config::ArrivalTime;
            speed = std::clamp(speed, Config::MinArrivalSpeed, Config::MaxArrivalSpeed);//(Config::MaxArrivalSpeed < speedLimit) ? Config::MaxArrivalSpeed : speedLimit);
            timeToArrive = arrivingRoute.second / speed;
            targets = GetTargets();
        }
    };

    struct HomingMjolnir {
        RE::Projectile* proj;
        std::vector<RE::Actor *> targets;
        RE::Actor* shooter;
        uint8_t hitCount;           //  how many times it will hit the targets?
        bool isBoomerang;           //  it will come back after hit all targets?
        float speed;
        float throwedTime;
        float angularVelocity;
        float waveAmplitude = 400.f;
        float waveFrequency = 20.f;

        float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - throwedTime;}
        //  give an origin point if you want to get nearest target
        RE::Actor* GetNextTarget(RE::NiPoint3 a_origin = RE::NiPoint3()) {
            std::erase_if(targets, [](const RE::Actor* actor) {
                return !actor || actor->IsDead();
            });
    //        if (targets.size() > 2u) {
    //            if (a_origin != RE::NiPoint3()) {
    //                std::sort(targets.begin(), targets.end(), 
    //                    [&](const auto& a, const auto& b) {
    //                        return a->GetPosition().GetDistance(a_origin) < 
    //                               b->GetPosition().GetDistance(a_origin);
    //                    }
    //                );
    //            } else {
    //                std::vector<RE::Actor*> sortedTargets;
    //                sortedTargets.push_back(targets.front());
    //                targets.erase(targets.begin());
    //                while (!targets.empty()) {
    //                    auto closestIt = std::min_element(targets.begin(), targets.end(), 
    //                        [&](const auto& a, const auto& b) {
    //                            return a->GetPosition().GetDistance(targets.back()->GetPosition()) < 
    //                                   b->GetPosition().GetDistance(targets.back()->GetPosition());
    //                        }
    //                    );
    //                    sortedTargets.push_back(*closestIt);
    //                    targets.erase(closestIt);
    //                }
    //                targets = std::move(sortedTargets);
    //            }
//
    //            for (size_t i = 0; i < targets.size() - 1; i++) {
    //                if (targets[i] == targets[i + 1]) {
    //                    RE::Actor* duplicate = targets[i];
    //                    targets.erase(targets.begin() + i);
    //                    targets.push_back(duplicate);
    //                }
    //            }
    //        }
            return !targets.empty() ? targets.front() : nullptr;
        }

        virtual ~HomingMjolnir() = default;
        HomingMjolnir() = default;
        HomingMjolnir(RE::Projectile* a_proj, 
            std::vector<RE::Actor *> a_targets, 
            RE::Actor* a_shooter, 
            const uint8_t a_hitCount = 1u, 
            bool a_isBoomerang = true, 
            float a_speed = 2000.f, 
            float a_angularVelocity = 5.f) : proj(a_proj), 
            targets(a_targets), 
            shooter(a_shooter), 
            hitCount(a_hitCount), 
            isBoomerang(a_isBoomerang), 
            speed(a_speed), 
            angularVelocity(a_angularVelocity)
        {
            throwedTime = AsyncUtil::GameTime::GetEngineTime();
            if (targets.size() == 1u) {hitCount = 4u;}
            else if (hitCount < 1u) hitCount = 1u;
            if (!targets.empty()) {
                spdlog::debug("homing mjolnir have {} times {} combat targets", hitCount, targets.size());

                size_t originalSize = targets.size();
                targets.resize(originalSize * hitCount);

                for (size_t i = 1; i < hitCount; i++) {
                    std::copy_n(targets.begin(), originalSize, targets.begin() + (i * originalSize));
                }
            }
        }
    };

    Data data;
    ArrivingMjolnir arrivingMjolnir;
    HomingMjolnir homingMjolnir;

    void Update(const float a_delta);
    void SetThrowState(const ThrowState a_throwState);
    ThrowState GetThrowState() const;
    bool GetPosition(RE::NiPoint3& a_point, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Throw(const bool justContinue, const bool isVertical = false, const bool isHoming = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Call(const bool a_justDestroy = false, const bool a_justContinue = false, std::optional<float> a_delay = std::nullopt, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Catch(bool a_justDestroy = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u);
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false);
    bool IsArriving(RE::Projectile* a_proj) const;
    bool IsHoming(RE::Projectile* a_proj) const;
    bool IsCharged() const {return _isCharged;}
    void StartChargingThrow(RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());

    bool isMjolnirCalled;
    bool isMjolnirArriving;
    bool isMjolnirThrowed;
//  RE::Projectile::LaunchData* LeviThrowData   = nullptr;

    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime callUpdate;
private:
friend class WeaponIdentify;
friend class AnimationEventTracker;
friend class ProjectileHook;

    RE::Projectile* MjolnirProjectileT      = nullptr;
    RE::Projectile* MjolnirProjectileA      = nullptr;
    RE::Projectile* LastMjolnirProjectile   = nullptr;

    RE::BGSProjectile* MjolnirProjBaseT = nullptr;
    RE::BGSProjectile* MjolnirProjBaseA = nullptr;

    RE::SpellItem* SpellMjolnirProjT    = nullptr;
    RE::SpellItem* SpellMjolnirProjA    = nullptr;
    RE::SpellItem* SpellCatchMjolnir    = nullptr;
    RE::SpellItem* SpellChargeCD        = nullptr;

    RE::EffectSetting* EffCatchMjolnir  = nullptr;
    RE::EnchantmentItem* EnchCharge     = nullptr;

    bool _isCharged = false;
    uint8_t chargeHitCount = 0;
    ThrowState throwState;
    PRECISION_API::CollisionDefinition collisionDefinition;

    Mjolnir()  = default;
    ~Mjolnir() = default;
};
#ifdef TRIDENT
class Trident
{
public:
    static Trident* GetSingleton() {static Trident singleton; return &singleton;}
    bool Initialize();

    struct Data {
        RE::TESObjectWEAP* weap     = nullptr;
        RE::EnchantmentItem* ench   = nullptr;
        RE::AlchemyItem* poison     = nullptr;
        RE::NiPoint3 position       = {0.f, 0.f, 0.f};
        RE::NiPoint3 lastVelocity   = {0.f, 0.f, 0.f};
        RE::NiAVObject* model       = nullptr;
        RE::Projectile* proj        = nullptr;
        RE::BGSExplosion* explosionThrow    = nullptr;
        RE::BGSExplosion* explosionCall     = nullptr;
        RE::NiNode* lastHitBone     = nullptr;          //  keeps last hit bone
        std::vector<RE::Actor*>         lastHitActors;  //  keeps last 3 hit actor from the last throw
        std::vector<RE::TESObjectREFR*> lastHitForms;   //  keeps last 3 hit object from the last throw
        float* enchMag              = nullptr;
        float defaultEnchMag;
        float damage            = 0.f;
        float yAngle            = 0.35f;
        float throwedTime       = 0.f;
    };

    Data data;

    void Update(const float a_delta);
    void Throw(const bool justContinue = false, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton());
    void Call(const float a_damage, const float a_force, RE::Actor* a_actor = RE::PlayerCharacter::GetSingleton(), const bool a_justEquip = false);

    void TrishulsMight(const float a_delay, const float a_duration);
    void RainOfTrident();
private:
friend class Kratos;
friend class WeaponIdentify;
friend class ProjectileHook;
friend class AnimationEventTracker;
friend class MagicEffectApplyTracker;
    RE::SpellItem*        SpellTridentProjL     = nullptr;
    RE::Projectile*       TridentProjectileL    = nullptr;
    RE::BGSProjectile*    TridentProjBaseL      = nullptr;
    RE::SpellItem*        SpellTridentsCallProjL    = nullptr;
    RE::BGSProjectile*    TridentCallProjBaseL      = nullptr;
    RE::SpellItem*        SpellTrishulsMightProjL   = nullptr;
    RE::BGSProjectile*    TrishulsMightProjBaseL    = nullptr;

    bool isTridentThrowable;

    void TriggerExplosion(float a_force, RE::ProjectileHandle* a_pHandle);
    void TriggerExplosionAtLocation(RE::NiNode* a_bone, RE::ProjectileHandle* a_pHandle, RE::Actor* a_target);
    void TriggerExplosionAtLocation(RE::Projectile* a_proj, RE::ProjectileHandle* a_pHandle);

    float nextLaunchTime = 0.0f;
    float nextLaunchDelay = 0.0f;
    float lastLaunchTime = 0.0f;
    bool tridentRainStarted = false;
};
#endif
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
    static std::uint32_t GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key);
    std::uint32_t GetOffsettedKeyCode(std::uint32_t a_keyCode, RE::INPUT_DEVICE a_inputDevice) const;

    InputEventTracker() = default;
    InputEventTracker(const InputEventTracker&) = delete;
    InputEventTracker(InputEventTracker&&) = delete;
    virtual ~InputEventTracker() = default;
    InputEventTracker& operator=(const InputEventTracker&) = delete;
    InputEventTracker& operator=(InputEventTracker&&) = delete;
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