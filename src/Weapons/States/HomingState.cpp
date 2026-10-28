#include "HomingState.h"
#include "Weapons/SmartRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Thrower.h"
#include "Util.h"

HomingState::HomingState(
    SmartRelicWeapon& a_weapon,
    uint8_t a_hitCount,
    bool    a_isBoomerang,
    float   a_speed,
    float   a_angularVelocity)
    : SmartWeaponState(a_weapon), 
        hitCount(a_hitCount), 
        isBoomerang(a_isBoomerang), 
        angularVelocity(a_angularVelocity)
{
    speed = a_speed;
}

void HomingState::InitializeTargets()
{
    auto throwerActor = GetControllerActor();
    if (!throwerActor) return;

    targets = ObjectUtil::Actor::GetNearCombatTargetHandles<Targets>(throwerActor, Config::HProjectileTargetRange, true);
    if (targets.empty()) return;

    if (targets.size() == 1u) hitCount = 4u;
    else if (hitCount < 1u)   hitCount = 1u;

    spdlog::debug("homing weapon have {} times {} combat targets", hitCount, targets.size());

    const size_t n = targets.size();
    targets.resize(n * hitCount);
    for (size_t i = 1; i < hitCount; i++)
        std::copy_n(targets.begin(), n, targets.begin() + i * n);
}

void HomingState::Enter()
{
    controller  = weapon.GetActiveController();
    hitInterval = 0.1f;
    process     = Process::kMain;
    blendDuration      = 0.69f;
    minBlendDuration   = 0.1f;
    transitionDuration = 0.1f;

    startingTime = AsyncUtil::GameTime::GetEngineTime();

    weapon.isPenetrating = true;
    weapon.targetLocalRotation = weapon.targetLocalRotation * RE::NiMatrix3(PI4 - PI, 0.f, 0.f);
    weapon.targetLocalTranslation.x *= -1.f;

    auto baseProjectile = weapon.projectile ? weapon.projectile->GetProjectileBase() : nullptr;
    if (baseProjectile) {
        auto& gravity = baseProjectile->data.gravity;
        defaultGravity = gravity;
        gravity = 0.f;
    }

    InitializeTargets();
}
void HomingState::Exit()
{
    weapon.isPenetrating = false;
    weapon.targetLocalRotation    = weapon.targetLocalRotation * RE::NiMatrix3(PI + PI4, 0.f, 0.f);
    weapon.targetLocalTranslation.x *= -1.f;

    auto baseProjectile = weapon.projectile ? weapon.projectile->GetProjectileBase() : nullptr;
    if (baseProjectile) {
        auto& gravity = weapon.projectile->GetProjectileBase()->data.gravity;
        gravity = defaultGravity;
    }

    if (!isBoomerang) return;

    auto thrower       = weapon.thrower;
    auto throwerParent = thrower ? thrower->GetParent() : nullptr;
    if (throwerParent)
        throwerParent->DoAction(ActionType::kWeaponCall);
}

void HomingState::OnTransition(float a_delta, RE::NiPoint3& a_vel)
{
    weapon.projState = ProjectileState::kLaunched;
    if (GetTransitionElapsed() < transitionDuration) return;
    weapon.projState = ProjectileState::kHoming;

    if (IsReadyToHit() && GetNextTarget()) {
        process = Process::kAttack;
        return;
    }

    if (GetTransitionElapsed() >= 0.2f) {
        process = Process::kMain;
    }
}

Status HomingState::MainProcess(float a_delta, RE::Actor* a_target, RE::NiPoint3& a_vel)
{
    if (a_target && projLivingTime > 0.69f) {
        Attack(a_target, a_vel);
        return Status::kRunning;
    }

    if (livingTime > 1.69f && targets.empty())
        return Status::kCompleted;

    auto throwerActor = GetControllerActor();
    if (!throwerActor) return Status::kCancelled;

    const float waveSin = waveAmplitude * std::cos(waveFrequency * livingTime);
    const float waveCos = waveAmplitude * std::sin(waveFrequency * livingTime);

    const auto  targetPos = throwerActor->GetPosition() + (throwerActor->GetBoundMax() + throwerActor->GetBoundMin()) * 0.75f;
    const float distance  = targetPos.GetDistance(weapon.position);
    auto        targetDir = targetPos - weapon.position;
    targetDir.Unitize();

    RE::NiPoint3 originVelocity;
    throwerActor->GetLinearVelocity(originVelocity);

    RE::NiPoint3 circularVel;
    circularVel.x = -speed * targetDir.y + originVelocity.x + waveSin;
    circularVel.y =  speed * targetDir.x + originVelocity.y + waveCos;
    circularVel.z =  speed * targetDir.z + originVelocity.z + waveCos;
    circularVel += targetDir * (distance - 100.f) / 0.2f;
    circularVel.Unitize();

    const float targetSpeed = speed * (livingTime > 0.5f && projLivingTime < 1.f ? projLivingTime + 0.2f : 1.f);
    a_vel = MathUtil::Angle::BlendVectors(weapon.direction, circularVel, tau, true) * targetSpeed;
    DampVerticalVelocity(a_vel, throwerActor->GetPosition());
    return Status::kRunning;
}
