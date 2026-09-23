#pragma once
#include "util.h"
#include "settings.h"
#include "RelicWeapon.h"
#include "Actors/Thrower.h"

class Thrower;

class ThrowableRelicWeapon : public RelicWeapon
{
public:
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

    float yAngle            = 0.35f;
    float throwedTime       = 0.f;
    float rotationSpeed     = Config::ThrowRotationSpeed; //  rad/s
    float gravity           = 3.21f;
    float throwingChargeDuration = 0.f;
    RotationType rotationType = RotationType::kNone;
    ImpactType   impactType   = ImpactType::kBlunt;

    struct RuntimeData : public RelicWeapon::RuntimeData {
        Thrower* thrower = nullptr;

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
        RE::NiPoint3 startLocalTranslation;
        RE::NiPoint3 targetLocalTranslation;
        RE::NiPoint3 velocity;
        RE::NiPoint3 direction;
        RE::NiPoint3 angles;

        bool isThrown = false;
    };

    bool Initialize() override;
    void Update() override;
    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
    void OnImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

    [[nodiscard]] bool IsCharged(const bool a_forLastThrow = false) const {return RelicWeapon::IsCharged() ? true : (a_forLastThrow ? _isLastThrowCharged : false);}

    [[nodiscard]] RuntimeData& GetThrableRuntimeData() {return throwableWeaponRuntimeData;}
    [[nodiscard]] Thrower* GetThrower() {return throwableWeaponRuntimeData.thrower;}

    virtual void Throw(const bool isVertical, const bool justContinue = false, const bool isHoming = false);
    virtual void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical);
    virtual void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical);
    virtual void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical);
    virtual void StartChargingThrow();
    virtual void AddProjectileTrail();
    virtual void FadeProjectileTrail();
    virtual void DeleteProjectileTrail();

protected:
    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime trailRemoveUpdate;

    ThrowableRelicWeapon()  = default;
    virtual ~ThrowableRelicWeapon() = default;

    bool _isLastThrowCharged = false;
    PRECISION_API::CollisionDefinition collisionDefinition;

    RuntimeData throwableWeaponRuntimeData;
};
