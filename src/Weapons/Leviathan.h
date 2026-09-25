#pragma once
#include "util.h"
#include "Assets.h"
#include "settings.h"
#include "SmartRelicWeapon.h"
#include "Weapons/States/HomingState.h"
#include "Weapons/States/ArrivingState.h"

class LeviathanAxe : public SmartRelicWeapon
{
public:
    ~LeviathanAxe() override = default;
    explicit LeviathanAxe(RE::TESBoundObject* a_object);

    struct RuntimeData : public SmartRelicWeapon::RuntimeData {
    };

    RuntimeData runtimeData;

    bool Initialize() override;
    void Update() override;
    void SetState(RelicWeaponState::Type a_type) override;
//    bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) override;
//    void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

//    void GetPosition(RE::NiPoint3& a_point) override;
    RelicType GetType() const override {return RelicType::kLeviathanAxe;}
//    void Throw(const bool isVertical, const bool justContinue = false, const bool isHoming = false) override;
    bool PreThrow() override;
    void PostThrow() override;
    void Call(const bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt) override;
    void Catch(bool a_justDestroy = false) override;
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override;
//    void SetHitRotation(RE::NiMatrix3& a_matrix, const bool a_vertical) override;
//    void SetHitRotation(RE::NiPoint3& a_angles, const RE::NiPoint3& a_direction, const bool a_vertical) override;
//    void TweakHitPosition(RE::NiPoint3& a_position, const RE::NiPoint3& a_direction, const float a_offset, const bool a_vertical) override;

    bool isAxeCalled;
    bool isAxeThrowed;
private:
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

    RE::EffectSetting* EffCatchLevi = nullptr;
};

class LeviathanHomingState : public HomingState
{
public:
    virtual ~LeviathanHomingState() = default;

    LeviathanHomingState(
        LeviathanAxe& a_weapon,
        uint8_t a_hitCount = 2u,
        bool a_isBoomerang = true,
        float a_speed = 2000.f,
        float a_angularVelocity = 5.f)
        : HomingState(a_weapon, a_hitCount, a_isBoomerang, a_speed, a_angularVelocity)
        {};
protected:
};

class LeviathanArrivingState : public ArrivingState
{
public:
    ~LeviathanArrivingState() override = default;

    LeviathanArrivingState(
        LeviathanAxe& a_weapon,
        const RE::NiPoint3& a_startPosition,
        RE::NiAVObject** a_targetBone)
        : ArrivingState(a_weapon, a_startPosition, a_targetBone)
        {};
    LeviathanArrivingState(
        const ArrivingState& a_previous,
        const RE::NiPoint3& a_startPosition)
        : ArrivingState(a_previous, a_startPosition)
        {};

protected:
//    void UpdateRotation() override;
//    void UpdateAI(RE::NiPoint3& a_outVel) override;
//    void UpdateArrivingDirection(bool a_initial = false) override;
//    void UpdateArrivingRoute() override;
};