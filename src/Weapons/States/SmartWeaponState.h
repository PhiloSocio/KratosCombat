#pragma once

#include "ThrowableWeaponState.h"

class SmartWeaponState : public ThrowableWeaponState
{
public:
    explicit SmartWeaponState(SmartRelicWeapon& a_weapon);
    ~SmartWeaponState() override = default;

    Status Update(float a_delta) override final;

protected:
    void Attack(RE::Actor* a_target, RE::NiPoint3& a_vel);

    virtual void   RefreshTargets() = 0;
    virtual Status MainProcess(float a_delta, RE::Actor* a_target, RE::NiPoint3& a_vel) = 0;
    virtual void   OnAttackEnd(RE::NiPoint3& a_vel) {}


    virtual void BeginTransition(Process a_from);
    virtual void OnTransition(float a_delta, RE::NiPoint3& a_vel);

    [[nodiscard]] float GetTransitionElapsed() const noexcept { return livingTime - _transitionStartTime; }

    Process transitionFrom    = Process::kMain;
    float transitionDuration  = 0.1f;

private:
    float _smoothedSpeed       = -1.f;
    float _transitionStartTime = 0.f;
};
