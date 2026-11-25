#pragma once

#include "Weapons/SmartRelicWeapon.h"
#include "Weapons/States/HomingState.h"
#include "Weapons/States/ArrivingState.h"

class LeviathanAxe : public SmartRelicWeapon
{
public:
    explicit LeviathanAxe(RE::TESBoundObject* a_object);
    ~LeviathanAxe() override;

    bool Initialize() override;

    RelicType GetType() const override {return RelicType::kLeviathanAxe;}
    bool PreThrow(ThrownWeapon& a_weapon, RE::Projectile::LaunchData& a_launchData) override;
    void PostThrow(ThrownWeapon& a_weapon) override;
    void SetState(ThrownSmartWeapon& a_weapon, RelicWeaponState::Type a_type) override;
    bool Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    bool ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override;

};

class LeviathanHomingState : public HomingState
{
public:
    LeviathanHomingState(
        ThrownSmartWeapon& a_weapon,
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
    LeviathanArrivingState(ThrownSmartWeapon& a_weapon) : ArrivingState(a_weapon) {};

protected:
//    void UpdateRotation() override;
//    void UpdateAI(RE::NiPoint3& a_outVel) override;
//    void UpdateArrivingDirection(bool a_initial = false) override;
//    void UpdateArrivingRoute() override;
};