#pragma once
#include "util.h"
#include "settings.h"
#include "RelicWeapon.h"
#include "Actors/Thrower.h"
#include "SoundManager.h"

class Thrower;

class ThrowableRelicWeapon : public RelicWeapon
{
public:
    ~ThrowableRelicWeapon() override = default;
    explicit ThrowableRelicWeapon(RE::TESBoundObject* a_object);

    static constexpr RE::NiMatrix3 RotationAngle(RotationType a_type) noexcept
    {
        switch (a_type) {
        case RotationType::kNone:         return RE::NiMatrix3();
        case RotationType::kSpinLateral:  return RE::NiMatrix3(0.f, PI8, PI2);
        case RotationType::kSpinVertical: return RE::NiMatrix3(0.f, PI2 - PI32, PI2);
        }
        return RE::NiMatrix3();
    }
    static constexpr RE::NiPoint3 TranslationOffset(RotationType a_type, const float a_length, const float a_cosZ = Config::HitRotationZcos) noexcept
    {
        switch (a_type) {
        case RotationType::kNone:         return RE::NiPoint3(0.f, -a_length, 0.f);
        case RotationType::kSpinLateral:  return RE::NiPoint3(0.f, -a_length * a_cosZ, 0.f);
        case RotationType::kSpinVertical: return RE::NiPoint3(0.f, -a_length * a_cosZ, 0.f);
        }
        return RE::NiPoint3(0.f, 20.f, 0.f);
    }

    enum class ProjectileState : std::uint8_t {
        kNone,
        kLaunched,
        kStucked,
        kHavok,
        kHoming
    };
    enum class TrailColor : std::uint32_t {
        kWhite,
        kIceBlue,
        kSkyBlue,
        kBlue,
        kYellow,
        kGold,
        kSilver
    };

    struct TrailData {
        TrailOverride trailOverride;
        TrailTransformOverride transformOverride = TrailTransformOverride();

        RE::NiColorA GetColorByIndex(const uint32_t a_index);
        TrailData(const std::string a_meshDirectory = Config::TrailModelPathDef, const float a_intensity = 1.f) {
            trailOverride = TrailOverride(
                Config::TrailLifetimeMult * a_intensity, 
                GetColorByIndex(Config::TrailColorIndexMjolnir), 
                Config::TrailColorScaleMult * a_intensity, 
                a_meshDirectory
            );
        }
    };

    struct RuntimeData : public RelicWeapon::RuntimeData {
        Thrower* thrower = nullptr;

        RE::ObjectRefHandle droppedWeaponKeep;

        RE::ProjectileHandle*   projectileHandle = nullptr;
        RE::Projectile*         projectile = nullptr;
        RE::NiNode*             projectileModel = nullptr;
        RE::NiNode*             weaponParentNode = nullptr;
        RE::NiPointer<RE::NiNode>   weaponModelCopy;
        RE::NiPointer<RE::NiNode>   weaponModelSterilizedCopy;
        RE::NiPointer<RE::NiNode>   replacedProjectileModel;

        ProjectileRot projectileRotation;
        RE::NiMatrix3 startLocalRotation;
        RE::NiMatrix3 targetLocalRotation;
        RE::NiPoint3 startLocalTranslation  = {0.f, 0.f, 0.f};
        RE::NiPoint3 targetLocalTranslation = {0.f, 0.f, 0.f};
        RE::NiPoint3 velocity       = {0.f, 0.f, 0.f};
        RE::NiPoint3 direction      = {0.f, 0.f, 0.f};
        RE::NiPoint3 angles         = {0.f, 0.f, 0.f};
        RE::NiPoint3 position       = {0.f, 0.f, 0.f};
        RE::NiTransform transformPW = RE::NiTransform();
        RE::NiTransform transformPL = RE::NiTransform();
        RE::NiTransform transformW  = RE::NiTransform();
        RE::NiTransform transformL  = RE::NiTransform();

        bool isThrown = false;
        bool isCountless = false;
        float isPenetrating = false;

        ProjectileState projState = ProjectileState::kNone;
        ImpactType impactType = ImpactType::kSharp;
        RE::ImpactResult impactResult = RE::ImpactResult::kBounce;
    };

    RuntimeData runtimeData;
    TrailData trailData;

    float yAngle            = 0.35f;
    float throwedTime       = 0.f;
    float rotationSpeed     = Config::ThrowRotationSpeed; //  rad/s
    float gravity           = 3.21f;
    float throwingChargeDuration = 0.f;
    RotationType rotationType = RotationType::kNone;
    ImpactType   impactType   = ImpactType::kBlunt;

    bool Initialize() override;
    void Update() override;
//    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override {};
    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

    [[nodiscard]] RE::BGSProjectile* CreateBaseProjectile(const char* a_editorID, const char* a_name);
    [[nodiscard]] RE::TESAmmo* CreateBaseAmmo(RE::BGSProjectile* a_baseProjectile, const char* a_editorID, const char* a_name);

    [[nodiscard]] SoundManager GetSoundManager() const {return soundData;}
    [[nodiscard]] bool IsCharged(const bool a_forLastThrow = false) const {return RelicWeapon::IsCharged() ? true : (a_forLastThrow ? _isLastThrowCharged : false);}

    [[nodiscard]] RuntimeData& GetThrableRuntimeData() {return runtimeData;}
    [[nodiscard]] Thrower* GetThrower() {return runtimeData.thrower;}
    [[nodiscard]] RE::Actor* GetThrowerActor() {return GetThrower() ? GetThrower()->GetActor() : nullptr;}
    [[nodiscard]] RE::TESObjectREFR* GetWeaponContainer() const noexcept { return runtimeData.droppedWeaponKeep.get().get(); }
    [[nodiscard]] RE::NiTransform GetWorldTransform();
    [[nodiscard]] RE::NiTransform GetLocalTransform();

    bool Throw(const RotationType a_rotationType, std::optional<ProjectileRot> a_pRot = std::nullopt, std::optional<RE::NiPoint3> a_origin = std::nullopt);
    void AddTrail();
    void FadeTrail();
    void RemoveTrail();

    virtual bool PreThrow() {return true;};
    virtual void PostThrow() {};
    virtual void OnTrailDelete() {};
//    virtual void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical);
//    virtual void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical);
//    virtual void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical);

protected:
    SoundManager soundData{
        SoundDefinition{
            .chargingLoop0 = nullptr,
            .throwingStart = nullptr,
            .throwingLoop0 = nullptr,
            .throwingLoop1 = nullptr,
            .callStart = nullptr,
            .arrivingStart = nullptr,
            .arrivingLoop0 = nullptr,
            .arrivingLoop1 = nullptr,
            .arrivingLoop2 = nullptr,
            .arrivingNear = nullptr,
            .catching = nullptr
        }
    };

    RE::BGSProjectile* ThrowableWeaponDummyProjectile = nullptr;
    RE::TESAmmo*       ThrowableWeaponDummyAmmo = nullptr;
    RE::TESObjectLIGH* ThrowableWeaponLight = nullptr;

    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime trailRemoveUpdate;

    bool _isLastThrowCharged = false;
    PRECISION_API::CollisionDefinition collisionDefinition;
};

using ProjectileState = ThrowableRelicWeapon::ProjectileState;
