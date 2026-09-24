#include "HomingState.h"
#include "Actors/Rager.h"
#include "Actors/Caller.h"

HomingState::HomingState(
    SmartRelicWeapon& a_weapon,
    std::vector<RE::ActorHandle> a_targets,
    uint8_t a_hitCount,
    bool a_isBoomerang,
    float a_speed,
    float a_angularVelocity)
    : ThrowableWeaponState(a_weapon),
        targets(std::move(a_targets)),
        hitCount(a_hitCount),
        isBoomerang(a_isBoomerang),
        speed(a_speed),
        angularVelocity(a_angularVelocity)
{
}

void HomingState::InitializeTargets()
{
    if (targets.empty()) {
        return;
    }

    if (targets.size() == 1u) {
        hitCount = 4u;
    } else if (hitCount < 1u) {
        hitCount = 1u;
    }

    spdlog::debug(
        "homing weapon have {} times {} combat targets",
        hitCount,
        targets.size());

    const size_t originalSize = targets.size();

    targets.resize(originalSize * hitCount);

    for (size_t i = 1; i < hitCount; ++i) {
        std::copy_n(
            targets.begin(),
            originalSize,
            targets.begin() + (i * originalSize));
    }
}
void HomingState::RemoveInvalidTargets()
{
    std::erase_if(targets, [](const RE::ActorHandle actorHandle) {
        auto actor = actorHandle.get().get();
        return !actor || actor->IsDead() || actor->IsUnconscious();
    });
}
RE::Actor* HomingState::GetNextTarget(RE::NiPoint3 a_origin)
{
    RemoveInvalidTargets();
//        if (targets.size() > 2u) {
//            if (a_origin != RE::NiPoint3()) {
//                std::sort(targets.begin(), targets.end(), 
//                    [&](const auto& a, const auto& b) {
//                        return a->GetPosition().GetDistance(a_origin) < 
//                               b->GetPosition().GetDistance(a_origin);
//                    }
//                );
//            } else {
//                std::vector<RE::ActorHandle> sortedTargets;
//                sortedTargets.push_back(targets.front());
//                targets.erase(targets.begin());
//                while (!targets.empty()) {
//                    auto closestIt = std::min_element(targets.begin(), targets.end(), 
//                        [&](const auto& a, const auto& b) {
//                            return a->GetPosition().GetDistance(targets.back()->GetPosition()) < 
//                                   b->GetPosition().GetDistance(targets.back()->GetPosition());
//                        }
//                    );
//                    sortedTargets.push_back(*closestIt);
//                    targets.erase(closestIt);
//                }
//                targets = std::move(sortedTargets);
//            }
//
//            for (size_t i = 0; i < targets.size() - 1; i++) {
//                if (targets[i] == targets[i + 1]) {
//                    RE::Actor* duplicate = targets[i];
//                    targets.erase(targets.begin() + i);
//                    targets.push_back(duplicate);
//                }
//            }
//        }
    return !targets.empty() ? targets.front().get().get() : nullptr;
}

void HomingState::Enter()
{
    startingTime = AsyncUtil::GameTime::GetEngineTime();
    InitializeTargets();
}
Status HomingState::Update(float a_delta)
{
    auto proj = weapon.smartWeaponRuntimeData.projectile;
    if (!proj) return Status::kCancelled;

    auto& projectileRTD = proj->GetProjectileRuntimeData();
    auto& vel = projectileRTD.linearVelocity;
    auto& angles = proj->data.angle;
    const float livingTime = projectileRTD.livingTime;
    const float hLivingTime = GetLivingTime();

    auto hTarget = GetNextTarget(weapon.smartWeaponRuntimeData.position);
    if (hLivingTime > 1.2f && livingTime > 0.2f && hTarget) {
        auto targetPos = hTarget->GetPosition() + (hTarget->GetBoundMax() + hTarget->GetBoundMin()) * 0.75f;
        auto targetDir = (targetPos - weapon.smartWeaponRuntimeData.position);
        float height = weapon.smartWeaponRuntimeData.position.z - hTarget->GetPosition().z;
        targetDir.Unitize();
        const float targetSpeed = speed * (livingTime < 1.2f ? livingTime : 1.2f);
        vel = MathUtil::Angle::BlendVectors(weapon.smartWeaponRuntimeData.direction, targetDir, ((livingTime - 0.2f) / 3.f), true) * targetSpeed;
        float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
        if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
    } else {
        if (hLivingTime > 2.69f && livingTime > 0.3f && targets.empty()) {
            return Status::kCompleted;
        }

        // waving effect
        float waveSin = waveAmplitude * cos(waveFrequency * hLivingTime);
        float waveCos = waveAmplitude * sin(waveFrequency * hLivingTime);

        auto thrower = weapon.smartWeaponRuntimeData.thrower;
        auto throwerActor = thrower ? thrower->GetActor() : nullptr;
        if (!throwerActor) return Status::kCancelled;

        auto targetPos = throwerActor->GetPosition() + (throwerActor->GetBoundMax() + throwerActor->GetBoundMin()) * 0.75f;
        float distance = targetPos.GetDistance(weapon.smartWeaponRuntimeData.position);
        float height = weapon.smartWeaponRuntimeData.position.z - throwerActor->GetPosition().z;
        auto targetDir = targetPos - weapon.smartWeaponRuntimeData.position;
        targetDir.Unitize();
        RE::NiPoint3 circularVel;
        RE::NiPoint3 originVelocity; throwerActor->GetLinearVelocity(originVelocity);
        circularVel.x = -speed * targetDir.y + originVelocity.x + waveSin;
        circularVel.y = speed * targetDir.x + originVelocity.y + waveCos;
        circularVel.z = speed * targetDir.z + originVelocity.z + waveCos;

        circularVel += targetDir * (distance - 100.f) / 0.2f;
        circularVel.Unitize();
        const float targetSpeed = speed * (hLivingTime > 0.5f && livingTime < 1.f ? livingTime + 0.2f : 1.f);
        vel = MathUtil::Angle::BlendVectors(weapon.smartWeaponRuntimeData.direction, circularVel, ((livingTime - 0.2f) / 3.f), true) * targetSpeed;
        float dampFactor = std::clamp((height - 10.f) / 90.f, 0.f, 1.f);
        if (vel.z < 0.f) vel.z *= dampFactor;  //  damp vertical speed
    }

    RE::NiPoint3 curvyDir = vel;
    curvyDir.Unitize();
    angles.x = asin(curvyDir.z);
    angles.z = atan2(curvyDir.x, curvyDir.y);
    return Status::kRunning;
}
void HomingState::Exit()
{
    if (auto thrower = weapon.GetThrower(); isBoomerang && thrower) {
        if (auto caller = dynamic_cast<Caller*>(thrower); caller) {
            auto rager = dynamic_cast<Rager*>(thrower);
            if (!rager || !rager->IsInRage()) {
                caller->CallWeapon();
            }
        }
    }
}
