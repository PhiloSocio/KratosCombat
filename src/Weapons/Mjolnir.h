#pragma once

#include "Weapons/SmartRelicWeapon.h"
#include "Weapons/States/HomingState.h"
#include "Weapons/States/ArrivingState.h"

class Caller;

class Mjolnir : public SmartRelicWeapon
{
public:
    ~Mjolnir() override;
    explicit Mjolnir(RE::TESBoundObject* a_object);

    bool Initialize() override;
    void Update() override;
    void SetState(RelicWeaponState::Type a_type) override;
    void OnMenuOpenCloseEvent(const bool a_opening) override;

    RelicType GetType() const override {return RelicType::kMjolnir;}
    bool PreThrow() override;
    void PostThrow() override;
    bool Call(Caller* a_caller, const bool a_justDestroy = false, std::optional<float> a_delay = std::nullopt) override;
    void Catch(bool a_justDestroy = false) override;
    void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) override;
    void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) override;

};

class MjolnirHomingState : public HomingState
{
public:
    virtual ~MjolnirHomingState() = default;
    MjolnirHomingState(
        Mjolnir& a_weapon,
        uint8_t a_hitCount = 2u,
        bool a_isBoomerang = true,
        float a_speed = 2000.f,
        float a_angularVelocity = 5.f)
        : HomingState(a_weapon, a_hitCount, a_isBoomerang, a_speed, a_angularVelocity)
        {};
protected:
};

class MjolnirArrivingState : public ArrivingState
{
public:
    ~MjolnirArrivingState() override = default;
    MjolnirArrivingState(Mjolnir& a_weapon) : ArrivingState(a_weapon) {};

protected:
//    void UpdateRotation() override;
//    void UpdateAI(RE::NiPoint3& a_outVel) override;
//    void UpdateArrivingDirection(bool a_initial = false) override;
//    void UpdateArrivingRoute() override;
};
