#pragma once

#include "SmartWeaponState.h"

class HomingState : public SmartWeaponState
{
public:
    inline static constexpr auto kType = Type::kHoming;

    ~HomingState() override = default;
    HomingState(
        SmartRelicWeapon& a_weapon,
        uint8_t a_hitCount      = 2u,
        bool a_isBoomerang      = true,
        float a_speed           = 2000.f,
        float a_angularVelocity = 5.f);

    void Enter() override;
    void Exit() override;

    [[nodiscard]] Type GetType() const override { return Type::kHoming; };

protected:
    void   InitializeTargets();
    void   RefreshTargets() override {}
    void   OnTransition(float a_delta, RE::NiPoint3& a_vel) override;
    Status MainProcess(float a_delta, RE::Actor* a_target, RE::NiPoint3& a_vel) override;

private:
    uint8_t hitCount        = 2u;
    bool    isBoomerang     = true;
    float   angularVelocity = 5.f;
    float   waveAmplitude   = 400.f;
    float   waveFrequency   = 20.f;

};
