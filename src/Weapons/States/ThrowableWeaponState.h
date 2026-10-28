#pragma once

#include "RelicWeaponState.h"

using Targets = std::vector<RE::ActorHandle>;
using ProjectileRuntimeData = RE::Projectile::PROJECTILE_RUNTIME_DATA;

class ActorCapability;
class SmartRelicWeapon;

class ThrowableWeaponState : public RelicWeaponState
{
public:
    explicit ThrowableWeaponState(SmartRelicWeapon& a_weapon);
    ~ThrowableWeaponState() override = default;

    void OnHit(RE::Actor* a_victim) override final;

    [[nodiscard]] virtual bool IsTargetValid(RE::Actor* a_target) const { return true; }

    [[nodiscard]] bool       IsReadyToHit()  const noexcept { return _isReadyToHit; }
    [[nodiscard]] float      GetLivingTime() const noexcept;
    [[nodiscard]] Targets&   GetTargets()    noexcept       { return targets; }
    [[nodiscard]] RE::Actor* GetNextTarget() const noexcept;
    [[nodiscard]] RE::NiPoint3 GetTargetBonePosition(RE::Actor* a_target) const;
    [[nodiscard]] RE::Actor*   GetControllerActor() const;

protected:
    SmartRelicWeapon& weapon;
    ActorCapability* controller = nullptr;

    Targets targets;
    float   startingTime      = 0.f;
    float   livingTime        = 0.f;
    float   projLivingTime    = 0.f;
    float   tau               = 0.5f;
    float   hitInterval       = 0.5f;
    float   minBlendDuration  = 0.2f;
    float   blendDuration     = 1.f;
    float   postHitLivingTime = 0.f;
    float   speed             = 0.5f;
    float   defaultGravity    = 1.f;
    Process process           = Process::kMain;

    void RemoveInvalidTargets();
    void RefreshHitReady();
    void SortTargets(RE::NiPoint3 a_origin);
    void CacheTimers(RE::Projectile* a_proj);
    void DampVerticalVelocity(RE::NiPoint3& a_vel, const RE::NiPoint3& a_refPos, float a_minHeight = 10.f, float a_dampingRange = 90.f) const;

private:
    bool  _isReadyToHit = true;
    float _lastHitTime  = 0.f;

};
