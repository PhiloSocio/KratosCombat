#pragma once

#include "ThrownWeapon.h"
#include "RelicWeapon.h"
#include "util.h"
#include "SoundManager.h"

class Thrower;

class ThrowableRelicWeapon : public RelicWeapon
{
public:
    explicit ThrowableRelicWeapon(RE::TESBoundObject* a_object);
    ~ThrowableRelicWeapon() override;

    static constexpr RelicAbility kRequiredAbility = RelicAbility::kThrowable;

    static constexpr std::size_t kUpdateArrow = 0xAB;
    static constexpr std::size_t kOnArrowKill = 0xA8;
    static constexpr std::size_t kGetCollisionArrow = 0xBE;
    static constexpr std::size_t kGetArrowImpactData = 0xBD;
    static constexpr std::size_t kGetGravityArrow = 0xB5;

    static void UpdateArrow(RE::ArrowProjectile* a_this, float a_delta);
    static void OnArrowKill(RE::ArrowProjectile* a_this);
    static void GetCollisionArrow(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_collector);
    static RE::Projectile::ImpactData* GetArrowImpactData(RE::ArrowProjectile* a_this, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable, std::uint32_t a6, std::uint32_t a7);
    static float GetGravityArrow(RE::ArrowProjectile* a_this);

    bool Initialize() override;
    void Update() override;
    void OnEquip(BaseActor* a_actor) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

    [[nodiscard]] RE::BGSProjectile* CreateBaseProjectile(const char* a_editorID, const char* a_name);
    [[nodiscard]] RE::TESAmmo* CreateBaseAmmo(RE::BGSProjectile* a_baseProjectile, const char* a_editorID, const char* a_name);

    [[nodiscard]] SoundManager& GetSoundManager() noexcept { return soundData; }
    [[nodiscard]] bool IsCharged() const override { return RelicWeapon::IsCharged(); }
    [[nodiscard]] bool IsCharged(const ThrownWeapon& a_weapon) const { return RelicWeapon::IsCharged() || a_weapon.isLastThrowCharged; }
    [[nodiscard]] Thrower* GetThrower() const noexcept;
    [[nodiscard]] RE::Actor* GetThrowerActor() const;
    [[nodiscard]] ThrowState GetThrowState() const noexcept { return lastThrownWeapon ? lastThrownWeapon->GetThrowState() : ThrowState::kNone; }
    [[nodiscard]] RE::TESObjectREFR* GetWeaponContainer() const noexcept { return droppedWeaponKeep.get().get(); }
    [[nodiscard]] ThrownWeapon* GetThrownWeapon(RE::Projectile* a_projectile) const noexcept;
    [[nodiscard]] ThrownWeapon* GetLastThrownWeapon() const noexcept { return lastThrownWeapon; }
    [[nodiscard]] bool HasActiveThrownWeapon() const noexcept;

    bool Throw(Thrower* a_thrower, RotationType a_rotationType, std::optional<ProjectileRot> a_pRot = std::nullopt, std::optional<RE::NiPoint3> a_origin = std::nullopt, ThrowType a_throwType = ThrowType::kNormalThrow, float a_chargeDuration = 0.f);
    bool RegisterAuxiliaryProjectile(RE::Projectile* a_projectile, const RE::ProjectileHandle& a_handle);
    void ResetProjectile();

    virtual std::unique_ptr<ThrownWeapon> CreateThrownWeapon();
    virtual bool OnHit(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_projectile, RE::hkpAllCdPointCollector* a_collector);
    virtual void PostImpact(ThrownWeapon& a_weapon, RE::ArrowProjectile* a_projectile, RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable);
    virtual bool PreThrow(ThrownWeapon& a_weapon, RE::Projectile::LaunchData& a_launchData);
    virtual void PostThrow(ThrownWeapon& a_weapon);

protected:
    friend class ThrownWeapon;
    friend class ThrownSmartWeapon;
    friend class RelicManager;

    bool HookProjectile(ThrownWeapon& a_weapon);
    void UnhookProjectile(RE::Projectile* a_projectile);
    void UnhookProjectiles();
    void CleanupThrownWeapons();
    bool RetireConflictingProjectile(RE::Projectile* a_projectile);
    bool RebindProjectile(ThrownWeapon& a_weapon, RE::Projectile* a_projectile, const RE::ProjectileHandle& a_handle);

    RE::ObjectRefHandle droppedWeaponKeep;
    std::unique_ptr<InventoryEntryData> weapIED;

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
    RE::TESAmmo* ThrowableWeaponDummyAmmo = nullptr;
    RE::TESObjectLIGH* ThrowableWeaponLight = nullptr;

    AsyncUtil::GameTime placeWeaponUpdate;
    std::unordered_map<RE::Projectile*, std::unique_ptr<ThrownWeapon>> thrownWeapons;
    std::vector<ThrownWeapon*> updateBuffer;
    std::vector<std::pair<std::uint64_t, std::unique_ptr<ThrownWeapon>>> retiredThrownWeapons;
    std::uint64_t updateCounter = 0;
    ThrownWeapon* lastThrownWeapon = nullptr;
    bool isCountless = false;
};
