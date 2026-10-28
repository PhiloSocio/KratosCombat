#include "SmartWeaponState.h"
#include "Weapons/SmartRelicWeapon.h"

SmartWeaponState::SmartWeaponState(SmartRelicWeapon& a_weapon)
    : ThrowableWeaponState(a_weapon)
{
}

Status SmartWeaponState::Update(float a_delta)
{
    auto proj = weapon.projectile;
    if (!proj) return Status::kCancelled;

    CacheTimers(proj);
    RefreshTargets();
    RemoveInvalidTargets();
    RefreshHitReady();

    auto& vel   = proj->GetProjectileRuntimeData().linearVelocity;
    auto target = IsReadyToHit() ? GetNextTarget() : nullptr;

    switch (process) {
    case Process::kMain:
        if (target) process = Process::kAttack;
        break;

    case Process::kAttack:
        if (!target) {
            OnAttackEnd(vel);
            BeginTransition(Process::kAttack);
        }
        break;

    case Process::kTransition:
        OnTransition(a_delta, vel);
        break;
    }

    return MainProcess(a_delta, process == Process::kAttack ? target : nullptr, vel);
}

void SmartWeaponState::Attack(RE::Actor* a_target, RE::NiPoint3& a_vel)
{
    const auto aimPos = GetTargetBonePosition(a_target);
    auto aimDir = aimPos - weapon.position;
    aimDir.Unitize();

    const float dot = std::clamp(weapon.direction.Dot(aimDir), -1.f, 1.f);
    const float t   = std::clamp((dot + 1.f) * 0.5f, 0.f, 1.f);
    const float angleFactor = 0.03f + 0.97f * t * t;

    const float targetSpeed = speed * angleFactor;
    if (_smoothedSpeed < 0.f) _smoothedSpeed = targetSpeed;
    else _smoothedSpeed += (targetSpeed - _smoothedSpeed) * 0.02f;

    a_vel = MathUtil::Angle::BlendVectors(weapon.direction, aimDir, tau, true) * _smoothedSpeed;
    DampVerticalVelocity(a_vel, a_target->GetPosition());
}

void SmartWeaponState::BeginTransition(Process a_from)
{
    transitionFrom       = a_from;
    _transitionStartTime = livingTime;
    process              = Process::kTransition;
}
void SmartWeaponState::OnTransition(float a_delta, RE::NiPoint3& a_vel)
{
    weapon.projState = ProjectileState::kLaunched;
    if (GetTransitionElapsed() < transitionDuration) return;
    weapon.projState = ProjectileState::kHoming;
    process = (IsReadyToHit() && GetNextTarget()) ? Process::kAttack : Process::kMain;
}
