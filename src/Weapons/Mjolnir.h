#pragma once
#include "util.h"
#include "Assets.h"
#include "settings.h"
#include "SmartRelicWeapon.h"
#include "Weapons/States/HomingState.h"
#include "Weapons/States/ArrivingState.h"

class Mjolnir : public SmartRelicWeapon
{
public:
    static Mjolnir* GetSingleton();

    TrailData trailData;

    bool Initialize() override;
    void Update() override;

    void GetPosition(RE::NiPoint3& a_point) override;
    RelicType GetType() const override {return RelicType::kMjolnir;}
    void Throw(const bool isVertical, const bool justContinue = false, const bool isHoming = false) override;
    void Call(const bool a_justDestroy = false, const bool a_justContinue = false, std::optional<float> a_delay = std::nullopt) override;
    void Catch(bool a_justDestroy = false) override;
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override;
    bool IsArriving() const override;
    bool IsHoming() const override;
    void StartChargingThrow() override;
    void AddProjectileTrail() override;
    void FadeProjectileTrail() override;
    void DeleteProjectileTrail() override;
    RE::NiTransform GetWorldTransform() override;
    RE::NiTransform GetLocalTransform() override;

    bool isMjolnirCalled;
    bool isMjolnirArriving;
    bool isMjolnirThrowed;

    AsyncUtil::GameTime projectileUpdate;
    AsyncUtil::GameTime trailUpdate;
    AsyncUtil::GameTime trailRemoveUpdate;
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

    Mjolnir()  = default;
    ~Mjolnir() = default;
};
