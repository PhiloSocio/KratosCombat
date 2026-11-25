#include "SmartWeaponState.h"
#include "Weapons/ThrownSmartWeapon.h"

SmartWeaponState::SmartWeaponState(ThrownSmartWeapon& a_weapon)
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
void SmartWeaponState::Attack(RE::Actor* a_target, RE::NiPoint3& a_vel, float a_delta)
{
    if (!a_target) {
        ResetAttackOnset();
        return;
    }

    //  Yeni hedefe ilk girişte onset'i başlat
    if (a_target != _lastAttackTarget) {
        _lastAttackTarget = a_target;
        _attackOnsetTime  = 0.f;
        _smoothedSpeed    = std::max(a_vel.Length(), speed * 0.2f);  //  mevcut hızdan başla
    }

    const RE::NiPoint3 incomingVel = a_vel;  //  MainProcess'ten gelen (transition/rutin) hız

    //  Bezier pursuit
    UpdateAttackRoute(a_target, incomingVel);
    RE::NiPoint3 desiredDir = SampleAttackRouteDirection();
    if (desiredDir.SqrLength() < 1e-6f) {
        desiredDir = GetTargetBonePosition(a_target) - weapon.position;
        desiredDir.Unitize();
    }

    //  Hedef hız
    RE::NiPoint3 currentDir = incomingVel;
    if (currentDir.SqrLength() < 1.f) currentDir = weapon.direction;
    currentDir.Unitize();

    const float headingDot  = std::clamp(currentDir.Dot(desiredDir), -1.f, 1.f);
    const float turnFactor  = (headingDot + 1.f) * 0.5f;
    const float turnSeverity = 1.f - turnFactor;   //  0 = düz, 1 = tam ters

    const float speedFactor  = 0.35f + 0.65f * turnFactor;
    const float distToTarget = weapon.position.GetDistance(GetTargetBonePosition(a_target));
    const float diveBoost    = 1.f + std::clamp(1.f - distToTarget / 300.f, 0.f, 1.f) * 0.3f;
    const float targetSpeed  = speed * speedFactor * diveBoost;

    //  Frame-bağımsız hız yumuşatma
    constexpr float speedSmoothTau = 0.12f;
    const float a = 1.f - std::exp(-a_delta / speedSmoothTau);
    _smoothedSpeed += (targetSpeed - _smoothedSpeed) * a;
    _smoothedSpeed  = std::max(_smoothedSpeed, speed * 0.15f);

    const RE::NiPoint3 attackVel = desiredDir * _smoothedSpeed;

    //  Onset ramp: dönüş şiddetine göre uzunluk
    const float onsetDuration = _attackOnsetDuration * (0.5f + 0.5f * turnSeverity);

if (_attackOnsetTime >= 0.f && _attackOnsetTime < onsetDuration) {
        _attackOnsetTime += a_delta;
        const float rawT = std::clamp(_attackOnsetTime / onsetDuration, 0.f, 1.f);
        const float w    = rawT * rawT * (3.f - 2.f * rawT);

        RE::NiPoint3 inDir = incomingVel; inDir.Unitize();
        RE::NiPoint3 outDir = attackVel;  outDir.Unitize();
        const float  blendMag = std::lerp(incomingVel.Length(), attackVel.Length(), w);

        a_vel = MathUtil::Algebra::SlerpDirection(inDir, outDir, w) * blendMag;
    } else {
        _attackOnsetTime = onsetDuration;
        a_vel = attackVel;
    }

    DampVerticalVelocity(a_vel, a_target->GetPosition());
}
void SmartWeaponState::UpdateAttackRoute(RE::Actor* a_target, const RE::NiPoint3& a_vel)
{
    const RE::NiPoint3 targetPos  = GetTargetBonePosition(a_target);
    const RE::NiPoint3 fromTarget = weapon.position;

    const float distToTarget = fromTarget.GetDistance(targetPos);
    if (distToTarget < 1e-3f) { ResetAttackRoute(); return; }

    RE::NiPoint3 dirToTarget = targetPos - fromTarget;
    dirToTarget.Unitize();

    RE::NiPoint3 currentDir = a_vel;
    if (currentDir.SqrLength() < 1.f) currentDir = weapon.direction;
    currentDir.Unitize();

    const float turnCos      = std::clamp(currentDir.Dot(dirToTarget), -1.f, 1.f);
    const float turnSeverity = (1.f - turnCos) * 0.5f;

    const float  startBlend   = turnSeverity * 0.35f;
    RE::NiPoint3 startTangent = currentDir * (1.f - startBlend) + dirToTarget * startBlend;
    startTangent.Unitize();

    RE::NiPoint3 endTangent = dirToTarget;
    const float heightDiff = fromTarget.z - targetPos.z;
    if (heightDiff > 100.f)        endTangent.z = std::min(endTangent.z, -0.3f);
    else if (heightDiff < -100.f)  endTangent.z = std::max(endTangent.z,  0.1f);
    endTangent.Unitize();

    const float startLead = distToTarget * (0.4f + turnSeverity * 0.6f);
    const float endLead   = distToTarget * 0.3f;

    RE::NiPoint3 targetVel;
    a_target->GetLinearVelocity(targetVel);
    const float approachSpeed = std::max(_smoothedSpeed, 100.f);
    const float predictTime   = std::min(distToTarget / approachSpeed, 0.5f);
    const RE::NiPoint3 predictedPos = targetPos + targetVel * predictTime;

    const RE::NiPoint3 P0 = fromTarget;
    const RE::NiPoint3 P1 = P0 + startTangent * startLead;
    const RE::NiPoint3 P2 = predictedPos - endTangent * endLead;
    const RE::NiPoint3 P3 = predictedPos;

    _attackRoute = MathUtil::Algebra::CalculateAndMeasureBezier(P0, P1, P2, P3, _attackRouteResolution);

    _attackRouteClosestIndex = 0u;
}
RE::NiPoint3 SmartWeaponState::SampleAttackRouteDirection() const
{
    if (_attackRoute.samples.empty()) return {};

    const float lookAhead  = std::clamp(std::fabs(_smoothedSpeed) * 0.1f, 50.f, 300.f);
    const float targetDist = _attackRoute.samples[_attackRouteClosestIndex].distanceFromStart + lookAhead;

    uint16_t idx = _attackRouteClosestIndex;
    while (idx + 1 < (int)_attackRoute.samples.size() &&
           _attackRoute.samples[idx].distanceFromStart < targetDist) {
        idx++;
    }

    RE::NiPoint3 dir = _attackRoute.samples[idx].point - weapon.position;
    dir.Unitize();
    return dir;
}
void SmartWeaponState::ResetAttackRoute()
{
    _attackRoute = MathUtil::Algebra::BezierCurve();
    _attackRouteClosestIndex = 0u;
}
void SmartWeaponState::BeginTransition(Process a_from)
{
    transitionFrom       = a_from;
    _transitionStartTime = livingTime;
    process              = Process::kTransition;
    ResetAttackRoute();
    ResetAttackOnset();
}
void SmartWeaponState::OnTransition(float a_delta, RE::NiPoint3& a_vel)
{
    weapon.projState = ProjectileState::kLaunched;
    if (GetTransitionElapsed() < transitionDuration) return;
    weapon.projState = ProjectileState::kHoming;
    process = (IsReadyToHit() && GetNextTarget()) ? Process::kAttack : Process::kMain;
}
