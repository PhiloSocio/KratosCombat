#include "ThrowableWeaponState.h"
#include "Weapons/SmartRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/ActorCapability.h"

ThrowableWeaponState::ThrowableWeaponState(SmartRelicWeapon& a_weapon)
    : weapon(a_weapon)
{
}

float ThrowableWeaponState::GetLivingTime() const noexcept
{
    return AsyncUtil::GameTime::GetEngineTime() - startingTime;
}
RE::Actor* ThrowableWeaponState::GetNextTarget() const noexcept
{
    return !targets.empty() ? targets.front().get().get() : nullptr;
}
RE::Actor* ThrowableWeaponState::GetControllerActor() const
{
    if (!controller) return nullptr;
    auto parent = controller->GetParent();
    return parent && parent->IsValid() ? parent->GetActor() : nullptr;
}
RE::NiPoint3 ThrowableWeaponState::GetTargetBonePosition(RE::Actor* a_target) const
{
    if (!a_target) return {};
    if (auto bone = a_target->GetNodeByName("NPC Spine2 [Spn2]"))
        return bone->world.translate;
    return a_target->GetPosition() + (a_target->GetBoundMax() + a_target->GetBoundMin()) * 0.75f;
}
void ThrowableWeaponState::DampVerticalVelocity(RE::NiPoint3& a_vel, const RE::NiPoint3& a_refPos, float a_minHeight, float a_dampingRange) const
{
    if (a_vel.z >= 0.f) return;
    const float height     = weapon.position.z - a_refPos.z;
    const float dampFactor = std::clamp((height - a_minHeight) / a_dampingRange, 0.f, 1.f);
    a_vel.z *= dampFactor;
}

void ThrowableWeaponState::RemoveInvalidTargets()
{
    std::erase_if(targets, [this](const RE::ActorHandle actorHandle) {
        auto actor = actorHandle.get().get();
        if (!actor || actor->IsDead() || actor->IsUnconscious()) return true;
        return !IsTargetValid(actor);
    });
}
void ThrowableWeaponState::RefreshHitReady()
{
    _isReadyToHit = AsyncUtil::GameTime::GetEngineTime() - _lastHitTime > hitInterval;
}
void ThrowableWeaponState::CacheTimers(RE::Projectile* a_proj)
{
    projLivingTime    = a_proj->GetProjectileRuntimeData().livingTime;
    livingTime        = GetLivingTime();
    postHitLivingTime = AsyncUtil::GameTime::GetEngineTime() - _lastHitTime;
    tau               = std::clamp((std::min(postHitLivingTime, livingTime) - minBlendDuration) / blendDuration, 0.f, 1.f);
}
void ThrowableWeaponState::SortTargets(RE::NiPoint3 a_origin)
{
    std::sort(targets.begin(), targets.end(),
        [&](const RE::ActorHandle a_lhs, const RE::ActorHandle a_rhs) {
            auto lhs = a_lhs.get().get();
            auto rhs = a_rhs.get().get();
            const float dl = lhs ? lhs->GetPosition().GetDistance(a_origin) : FLT_MAX;
            const float dr = rhs ? rhs->GetPosition().GetDistance(a_origin) : FLT_MAX;
            return dl < dr;
        });
}

void ThrowableWeaponState::OnHit(RE::Actor* a_victim)
{
    if (!_isReadyToHit || !controller || !a_victim || !weapon.projectile) return;

    auto controllerActor = GetControllerActor();
    if (!controllerActor) return;

    GetTargets().erase(GetTargets().begin());
    ObjectUtil::Actor::ProcessHitImpl(controllerActor, a_victim, weapon.projectile, false);
    _lastHitTime = AsyncUtil::GameTime::GetEngineTime();
    _isReadyToHit = false; 
}
