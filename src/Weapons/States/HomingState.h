#pragma once
#include "util.h"
#include "ThrowableWeaponState.h"

class HomingState : public ThrowableWeaponState
{
public:
    ~HomingState() override = default;

    HomingState(
        SmartRelicWeapon& a_weapon,
        uint8_t a_hitCount = 2u,
        bool a_isBoomerang = true,
        float a_speed = 2000.f,
        float a_angularVelocity = 5.f);

protected:
    std::vector<RE::ActorHandle> targets;

    uint8_t hitCount = 2u;
    bool isBoomerang = true;

    float speed = 2000.f;
    float angularVelocity = 5.f;
    float waveAmplitude = 400.f;
    float waveFrequency = 20.f;

    float startingTime = 0.f;

    void RemoveInvalidTargets();

private:
    void InitializeTargets();

public:
    float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - startingTime;}
    //  give an origin point if you want to get nearest target
    RE::Actor* GetNextTarget(RE::NiPoint3 a_origin = RE::NiPoint3());

    void Enter() override;
    Status Update(float a_delta) override;
    void Exit() override;

    [[nodiscard]] Type GetType() const override { return Type::kHoming; };

    void UpdateRotation() {};
    void UpdateHomingDirection(float a_delta) {};
    void UpdateHomingRoute(float a_delta) {};

};
