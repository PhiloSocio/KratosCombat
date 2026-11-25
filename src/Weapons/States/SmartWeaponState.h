#pragma once

#include "ThrowableWeaponState.h"
#include "util.h"

class SmartWeaponState : public ThrowableWeaponState
{
public:
    explicit SmartWeaponState(ThrownSmartWeapon& a_weapon);

    Status Update(float a_delta) override final;

protected:
    void Attack(RE::Actor* a_target, RE::NiPoint3& a_vel, float a_delta);

    virtual void   RefreshTargets() = 0;
    virtual Status MainProcess(float a_delta, RE::Actor* a_target, RE::NiPoint3& a_vel) = 0;
    virtual void   OnAttackEnd(RE::NiPoint3& a_vel) {}

    virtual void BeginTransition(Process a_from);
    virtual void OnTransition(float a_delta, RE::NiPoint3& a_vel);

    [[nodiscard]] float GetTransitionElapsed() const noexcept { return livingTime - _transitionStartTime; }

    Process transitionFrom    = Process::kMain;
    float transitionDuration  = 0.1f;

private:
    void         UpdateAttackRoute(RE::Actor* a_target, const RE::NiPoint3& a_vel);
    RE::NiPoint3 SampleAttackRouteDirection() const;
    void         ResetAttackRoute();

    MathUtil::Algebra::BezierCurve _attackRoute;
    uint16_t                       _attackRouteClosestIndex = 0u;
    static constexpr uint16_t      _attackRouteResolution   = 48u;

    RE::Actor*             _lastAttackTarget    = nullptr;
    float                  _attackOnsetTime     = -1.f;
    static constexpr float _attackOnsetDuration = 0.36f;

    void ResetAttackOnset() {
        _lastAttackTarget = nullptr;
        _attackOnsetTime  = -1.f;
        _smoothedSpeed    = -1.f;
    }

    float _smoothedSpeed       = -1.f;
    float _transitionStartTime = 0.f;
};
