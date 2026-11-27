#pragma once

#include "Types.h"
#include "settings.h"
#include "SoundManager.h"

class ThrowableRelicWeapon;
class Thrower;
class BaseActor;

class ThrownWeapon
{
public:
    explicit ThrownWeapon(ThrowableRelicWeapon& a_weapon);
    virtual ~ThrownWeapon() = default;

    enum class ProjectileState : std::uint8_t {
        kNone,
        kLaunched,
        kHoming,
        kTransition,
        kStucked,
        kHavok
    };
    enum class ThrowState : std::uint8_t {
        kNone,
        kThrowable,
        kThrown,
        kCanArrive,
        kArriving,
        kArrived
    };
    struct TrailData {
        TrailData(const std::string a_meshDirectory = Config::TrailModelPathDef, const float a_intensity = 1.f, const uint32_t a_colorIndex = 0u);

        enum class TrailColor : std::uint32_t {
            kWhite,
            kIceBlue,
            kSkyBlue,
            kBlue,
            kYellow,
            kGold,
            kSilver
        };

        const RE::NiColorA& GetColorByIndex(const uint32_t a_index) const noexcept;
        const std::string& GetTrailMeshByEnchantment(const RE::EnchantmentItem* a_ench) const noexcept;

        TrailOverride trailOverride;
        TrailTransformOverride transformOverride = TrailTransformOverride();
        RE::NiPointer<RE::NiNode> trailParentNode;
    };
    static constexpr RelicAbility kType = RelicAbility::kThrowable;

    static constexpr RE::NiMatrix3 RotationAngle(RotationType a_type) noexcept
    {
        switch (a_type) {
        case RotationType::kNone:         return RE::NiMatrix3();
        case RotationType::kSpinLateral:  return RE::NiMatrix3(0.f, PI8, PI2 + PI8);
        case RotationType::kSpinVertical: return RE::NiMatrix3(0.f, PI2 - PI32, PI2 + PI8);
        }
        return RE::NiMatrix3();
    }
    static constexpr RE::NiPoint3 TranslationOffset(RotationType a_type, float a_length, float a_cosZ = Config::HitRotationZcos) noexcept
    {
        switch (a_type) {
        case RotationType::kNone:
            return RE::NiPoint3(0.f, -a_length + 10.f, 0.f);
        case RotationType::kSpinLateral:
            return RE::NiPoint3(-a_length * std::sqrt(1.0 - a_cosZ * a_cosZ) * 0.69f, -a_length * a_cosZ + 10.f, 0.f);
        case RotationType::kSpinVertical:
            return RE::NiPoint3(0.f, -a_length * a_cosZ + 10.f, -a_length * std::sqrt(1.0 - a_cosZ * a_cosZ) * 0.69f);
        }
        return RE::NiPoint3(0.f, 20.f, 0.f);
    }

    bool Throw(Thrower* a_thrower, RotationType a_rotationType, std::optional<ProjectileRot> a_pRot, std::optional<RE::NiPoint3> a_origin, ThrowType a_throwType, float a_chargeDuration);
    virtual void Update(float a_delta);
    virtual void UpdateProjectile(RE::Projectile* a_projectile, float a_delta);
    virtual bool OnHit(RE::ArrowProjectile* a_projectile, RE::hkpAllCdPointCollector* a_collector);
    virtual void PostImpact(RE::ArrowProjectile* a_projectile, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable);
    virtual void OnTrailDelete() {}

    [[nodiscard]] virtual RelicAbility GetType() const { return RelicAbility::kThrowable; }
    [[nodiscard]] RE::Actor* GetThrowerActor() const;
    [[nodiscard]] RE::NiTransform GetWorldTransform();
    [[nodiscard]] RE::NiTransform GetLocalTransform();
    [[nodiscard]] float GetLivingTime() const noexcept { return AsyncUtil::GameTime::GetEngineTime() - launchTime; }
    [[nodiscard]] ThrowState GetThrowState() const noexcept { return throwState; }
    [[nodiscard]] Thrower* GetThrower() const noexcept { return thrower; }
    [[nodiscard]] bool IsEquipped() const noexcept;
    [[nodiscard]] BaseActor* GetCarrier() const noexcept;
    [[nodiscard]] SoundManager& GetSoundManager() const noexcept;
    void MarkHitTransformApplied() noexcept { _hitTransformApplied = true; }
    template <typename T>
    [[nodiscard]] T* As() noexcept
    {
        return const_cast<T*>(static_cast<const ThrownWeapon*>(this)->As<T>());
    }
    template <typename T>
    [[nodiscard]] const T* As() const noexcept
    {
        static_assert(std::is_base_of_v<ThrownWeapon, T>);
        static_assert(requires { T::kType; });
        return GetType() == T::kType ? static_cast<const T*>(this) : nullptr;
    }

    void SetThrowState(ThrowState a_state) noexcept { throwState = a_state; }

    bool InitiateTransform() noexcept;
    void InitiateModel() noexcept;
    bool InitiateTrail() noexcept;
    void UpdateRotation(float a_delta, float a_livingTime, RE::NiPoint3& a_angles) noexcept;
    void UpdateTranslation(float a_delta, float a_livingTime) noexcept;
    void FadeTrail();
    void RemoveTrail();
    void ResetProjectile();

    ThrowableRelicWeapon& weapon;
    RE::TESObjectWEAP* weap = nullptr;
    RE::EnchantmentItem* ench = nullptr;
    RE::TESObjectREFR*& currentReference;
    std::unique_ptr<InventoryEntryData>& weapIED;
    RE::BGSProjectile* ThrowableWeaponDummyProjectile = nullptr;
    RE::TESAmmo* ThrowableWeaponDummyAmmo = nullptr;
    float length = 0.f;
    Thrower* thrower = nullptr;
    BaseActor* throwerParent = nullptr;

    RE::ProjectileHandle projectileHandle;
    RE::Projectile* projectile = nullptr;
    RE::NiPointer<RE::BSFadeNode> projectileModel;
    RE::NiPointer<RE::NiNode> weaponParentNode;
    RE::NiPointer<RE::NiNode> weaponModelCopy;
    RE::NiPointer<RE::NiNode> weaponModelSterilizedCopy;
    RE::NiPointer<RE::NiNode> replacedProjectileModel;

    ProjectileRot projectileRotation{};
    RE::NiMatrix3 startWorldRotation;
    RE::NiMatrix3 startLocalRotation;
    RE::NiMatrix3 targetLocalRotation;
    RE::NiMatrix3 hitLocalRotation;
    RE::NiPoint3 startLocalTranslation{};
    RE::NiPoint3 targetLocalTranslation{};
    RE::NiPoint3 velocity{};
    RE::NiPoint3 direction{};
    RE::NiPoint3 angles{};
    RE::NiPoint3 position{};
    RE::NiTransform transformPW = RE::NiTransform();
    RE::NiTransform transformPL = RE::NiTransform();
    RE::NiTransform transformW = RE::NiTransform();
    RE::NiTransform transformL = RE::NiTransform();

    TrailData trailData;
    AsyncUtil::GameTime trailRemoveUpdate;

    bool isThrown = false;
    bool isPenetrating = false;
    std::atomic_bool isDestroyed = false;
    std::uint64_t retireUpdate = 0;
    bool isAuxiliary = false;
    bool isLastThrowCharged = false;

    ProjectileState projState = ProjectileState::kNone;
    ImpactType impactType = ImpactType::kSharp;
    std::atomic<float> currentGravity{1.f};
    std::atomic_bool gravityFrozen{false};
    bool destructionHandled = false;

    float throwableDamage = 0.f;
    float yAngle = 0.35f;
    float launchTime = 0.f;
    float livingTime = 0.f;
    float rotationSpeed = Config::ThrowRotationSpeed;
    float gravity = 3.21f;
    float aeroDragCoeff = 3.2e-4f;
    float throwingChargeDuration = 0.f;
    ThrowType throwingType = ThrowType::kNormalThrow;
    RotationType rotationType = RotationType::kNone;

    ThrowState throwState = ThrowState::kNone;

protected:
    bool _transformInitiated = false;
    bool _collisionInitiated = false;
    bool _modelInitiated = false;
    bool _trailInitiated = false;
    bool _rotationBlended = false;
    bool _hitTransformApplied = false;
    bool _translationBlended = false;
};

using ProjectileState = ThrownWeapon::ProjectileState;
using ThrowState = ThrownWeapon::ThrowState;
using TrailColor = ThrownWeapon::TrailData::TrailColor;
