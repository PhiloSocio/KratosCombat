#include "ArrivingState.h"
#include "Weapons/SmartRelicWeapon.h"
#include "Actors/BaseActor.h"
#include "Actors/Capabilities/Caller.h"

ArrivingState::ArrivingState(SmartRelicWeapon& a_weapon)
    : SmartWeaponState(a_weapon)
{
}

RE::NiAVObject* ArrivingState::GetCallerHandBone() const
{
    return callerHandBoneSource ? *callerHandBoneSource : nullptr;
}
RE::NiAVObject* ArrivingState::GetCallerWeaponBone() const
{
    return callerWeaponBoneSource ? *callerWeaponBoneSource : nullptr;
}

bool ArrivingState::IsInCallingAnimation()
{
    if (callerActor)
        callerActor->GetGraphVariableBool("bIsInCallingAnimation", isInCallingAnimation);
    return isInCallingAnimation;
}
bool ArrivingState::IsTargetValid(RE::Actor* a_target) const
{
    const auto targetPos = a_target->GetPosition() + (a_target->GetBoundMax() + a_target->GetBoundMin()) * 0.75f;
    auto targetDir = targetPos - position;
    targetDir.Unitize();

    const float distance = a_target->GetPosition().GetDistance(position);
    if (distance > linearDistance) return false;
    if (currentDir.Dot(targetDir) < std::cos(PI8)) return false;
    if (currentDir.Dot(linearArrivingDir) < std::cos(PI8)) return false;
    return true;
}

void ArrivingState::RefreshTargets()
{
    if (!callerActor) return;

    targets = ObjectUtil::Actor::GetNearCombatTargetHandles<Targets>(callerActor, linearDistance, true);
    SortTargets(position);
}

void ArrivingState::OnAttackEnd(RE::NiPoint3& a_vel)
{
    speed *= 0.73f;
    a_vel *= 0.73f;
    timeToArrive = std::min(timeToArrive + hitDurationBudget, Config::ArrivalTime * 2.f);
}

void ArrivingState::InitializeCallerData()
{
    if (!weapon.GetCaller() ||
        !weapon.GetCaller()->GetParent() ||
        !weapon.GetCaller()->GetParent()->IsValid())
    {
        callerActor = nullptr;
        return;
    }

    callerActor = weapon.GetCaller()->GetParent()->GetActor();

    callerBreastBone.reset(callerActor->GetNodeByName("NPC Spine2 [Spn2]"));

    callerWeaponBoneSource = &weapon.GetCaller()->GetParent()->GetWeaponBone();
    if (callerWeaponBoneSource)
        callerWeaponBone = *callerWeaponBoneSource;

    callerHandBoneSource = &weapon.GetCaller()->GetParent()->GetRHandBone();
    if (callerHandBoneSource)
        callerHandBone = *callerHandBoneSource;

    const auto callerHandPosition =
        callerHandBone
            ? callerHandBone->world.translate
            : callerActor
                ? callerActor->GetPosition()
                : RE::NiPoint3();

    linearArrivingDir = callerHandPosition - startPosition;
    linearArrivingDir.Unitize();

    linearDistanceFromStart      = startPosition.GetDistance(callerHandPosition) + 1.f;
    linearDistanceFromLastCallPos = linearDistanceFromStart;
}
void ArrivingState::InitializeRoute()
{
    arrivingRoute             = MathUtil::Algebra::BezierCurve();
    arrivingRouteClosestIndex = 0;

    bezierControlPoints[0] = startPosition;
    bezierControlPoints[1] = startPosition + linearArrivingDir * linearDistanceFromLastCallPos * 0.33f;
}
void ArrivingState::InitiateTransform()
{
    if (_transformInitiated) return;
    if (!weapon.weaponParentNode || !weapon.weaponParentNode->parent || !weapon.replacedProjectileModel) return;

    auto& replacedPMParent = weapon.weaponParentNode;
    auto& localRotation    = replacedPMParent->local.rotate;
    const auto& desiredParentWorldRotate = weapon.GetWorldTransform().rotate;
    const auto& parentWorldRotate        = replacedPMParent->parent->world.rotate;
    localRotation = parentWorldRotate.Transpose() * desiredParentWorldRotate;
    startRotation = localRotation;
    _transformInitiated = true;
}

void ArrivingState::UpdateRotation()
{
    if (!weapon.weaponParentNode) return;

    auto& replacedPMParent = weapon.weaponParentNode;
    auto& localRotation    = replacedPMParent->local.rotate;

    RE::NiMatrix3 targetLocalRotation;
    if (replacedPMParent->parent)
        targetLocalRotation = replacedPMParent->parent->world.rotate.Transpose() * callerWeaponBone->world.rotate;
    else
        targetLocalRotation = callerWeaponBone->world.rotate;

    const float angleZ = livingTime * rotationSpeed;
    MathUtil::Angle::NormalAbsoluteAngle(angleZ);

    float tZrot = tReal;
    if (tReal < 0.33333f) {
        arrivalSpin = (int)(std::floor(1 + (livingTime + remainingTimeToArrive) * rotationSpeed / TWO_PI)) * TWO_PI;
        if (tReal < 0.16666667f)
            tZrot = 6.f * tReal * tReal;
    }

    const float blendZ = std::lerp(angleZ, arrivalSpin, tReal * tZrot);
    localRotation = MathUtil::Algebra::InterpolateRotation(const_cast<const RE::NiMatrix3&>(startRotation), targetLocalRotation, tReal);
    localRotation = localRotation * RE::NiMatrix3(0.f, 0.f, blendZ);
}
void ArrivingState::UpdateArrivingDirection(const bool a_initial)
{
    if (RE::PlayerCamera::GetSingleton()->IsInFirstPerson()) return;
    if (!callerActor || !callerBreastBone) return;

    if (weapon.GetThrowState() == ThrowState::kThrowable || isCatchable) {
        //  no-op
    } else if (a_initial || linearDistance > 100.f) {
        RE::NiPoint3 spineForwardDir = callerBreastBone->world.rotate * RE::NiPoint3(frontVec3);
        spineForwardDir.z = 0.f;
        spineForwardDir.Unitize();

        RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
        linearDir2D.Unitize();

        const float dot = spineForwardDir.Dot(linearDir2D);
        const float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;
        arrivingRelativeAngleZ = std::atan2(det, dot);
        arrivingRelativeAngleZ = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngleZ);

        float snapStrength = arrivingRelativeAngleSnapStrength;
        if (a_initial) snapStrength = 1.f;
        else if ((arrivingRelativeAngleZ < PI4 || arrivingRelativeAngleZ > ONEANDHALF_PI) && snapStrength < 0.69f)
            snapStrength += 0.69f;
        snapStrength = std::min(snapStrength, 1.f);
        if (snapStrength > 0.f)
            arrivingRelativeAngleZ = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngleZ, arrivingDirections, snapStrength);

        if (!a_initial) {
            float previousAngle = 0.f;
            if (callerActor->GetGraphVariableFloat("fArrivingWeaponDirection", previousAngle)) previousAngle *= TWO_PI;
            if (previousAngle == 0.f) {
                spdlog::error("weird, could not get graph variable value!");
                return;
            }
            const float delta = MathUtil::Angle::NormalizeSignedAngle(arrivingRelativeAngleZ - previousAngle);
            constexpr float smoothTime = 0.369f;
            const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
            arrivingRelativeAngleZ = previousAngle + delta * alpha;
        }
        arrivingRelativeAngleZ /= TWO_PI;
    }
    callerActor->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngleZ);
}
void ArrivingState::UpdateArrivingRoute(const float a_delta)
{
    const auto& handRot         = callerHandBone->world.rotate;
    const auto& previousHandRot = callerHandBone->previousWorld.rotate;
    const float alphaHandRot    = 1.f - std::exp(-a_delta / 0.169f);
    RE::NiMatrix3 smoothedHandRot =
        MathUtil::Algebra::QuaternionToMatrix(MathUtil::Algebra::Slerp(
            MathUtil::Algebra::MatrixToQuaternion(previousHandRot),
            MathUtil::Algebra::MatrixToQuaternion(handRot), alphaHandRot));
    RE::NiPoint3 palmDir     = smoothedHandRot * RE::NiPoint3(backVec3);
    RE::NiPoint3 handForward = smoothedHandRot * RE::NiPoint3(upVec3);
    palmDir.Unitize();
    handForward.Unitize();

    const float handSideOffsetMult = MathUtil::Algebra::ParabolicClamp(arrivingRelativeAngleZ, 0.1f, 0.3f);

    const RE::NiPoint3 handVelocity = (callerHandBone->world.translate - callerHandBone->previousWorld.translate) / a_delta;
    const float predictionTime = std::clamp(a_delta, 0.0f, 0.1f);
    const RE::NiPoint3 predictedHandPos = handPosition + handVelocity * predictionTime;

    bezierControlPoints[3] = predictedHandPos;
    bezierControlPoints[2] = predictedHandPos
        + (float)IsInCallingAnimation() * (palmDir * (linearDistanceFromLastCallPos * 0.33f + 20.f)
                                         + handForward * (linearDistanceFromLastCallPos * handSideOffsetMult + 10.f));

    RE::NiPoint3& p0 = bezierControlPoints[0];
    RE::NiPoint3& p1 = bezierControlPoints[1];
    RE::NiPoint3& p2 = bezierControlPoints[2];
    RE::NiPoint3& p3 = bezierControlPoints[3];

    arrivingRoute = MathUtil::Algebra::CalculateAndMeasureBezier(p0, p1, p2, p3, routeResolution);

    float bestDist2 = FLT_MAX;
    for (int i = 0; i < (int)arrivingRoute.samples.size() - 1; i++) {
        const float d2 = (arrivingRoute.samples[i].point - position).SqrLength();
        if (d2 < bestDist2) {
            bestDist2 = d2;
            arrivingRouteClosestIndex = i;
        }
    }

    closestSample = arrivingRoute.samples[arrivingRouteClosestIndex];

    const float lookAheadDistance = std::clamp(std::abs(speed) * 0.15f, 50.f, 200.f);
    const float targetDistance    = closestSample.distanceFromStart + lookAheadDistance;

    uint16_t targetIndex = arrivingRouteClosestIndex;
    while (targetIndex + 1 < (int)arrivingRoute.samples.size()
        && arrivingRoute.samples[targetIndex].distanceFromStart < targetDistance)
    {
        targetIndex++;
    }

    bezierDir = arrivingRoute.samples[targetIndex].point - position;
    bezierDir.Unitize();
}

void ArrivingState::Enter()
{
    controller    = weapon.GetActiveController();
    hitInterval   = 0.069f;
    blendDuration = 0.2f;
    process       = Process::kMain;

    weapon.isPenetrating = true;
    weapon.GetPosition(startPosition);
    InitializeCallerData();
    RefreshTargets();
    UpdateArrivingDirection(true);
    InitializeRoute();

    startingTime = AsyncUtil::GameTime::GetEngineTime();

    auto baseProjectile = weapon.projectile ? weapon.projectile->GetProjectileBase() : nullptr;
    if (baseProjectile) {
        auto& gravity = baseProjectile->data.gravity;
        defaultGravity = gravity;
        gravity = 0.f;
    }

    const bool doBlend = weapon.projState == ProjectileState::kNone
                      || weapon.projState == ProjectileState::kLaunched
                      || weapon.projState == ProjectileState::kHoming;
    smoothedDesiredVelocity = doBlend ? weapon.velocity : linearArrivingDir * speed;
}
void ArrivingState::Exit()
{
    weapon.isPenetrating = false;

    auto baseProjectile = weapon.projectile ? weapon.projectile->GetProjectileBase() : nullptr;
    if (baseProjectile) {
        auto& gravity = weapon.projectile->GetProjectileBase()->data.gravity;
        gravity = defaultGravity;
    }
}

Status ArrivingState::MainProcess(float a_delta, RE::Actor* a_target, RE::NiPoint3& a_vel)
{
    if (!callerActor) return Status::kCancelled;

    model = weapon.projectileModel;
    if (!model) return Status::kCancelled;

    callerHandBone = GetCallerHandBone();
    if (!callerHandBone) return Status::kCancelled;
    handPosition = callerHandBone->world.translate;

    callerWeaponBone = GetCallerWeaponBone();
    if (!callerWeaponBone) return Status::kCancelled;

    auto proj = weapon.projectile;
    if (!proj) return Status::kCancelled;

    if (weapon.projState == ProjectileState::kTransition) {
        UpdateArrivingDirection();
        return Status::kRunning;
    } else if (weapon.projState == ProjectileState::kNone) {
        weapon.projState = ProjectileState::kHoming;
        InitializeCallerData();
        RefreshTargets();
        InitializeRoute();
    }

    currentDir = a_vel; currentDir.Unitize();
    position   = proj->data.location;

    linearDistance    = handPosition.GetDistance(position);
    linearArrivingDir = handPosition - position;
    linearArrivingDir.Unitize();

    isCatchable = (linearDistance <= Config::CatchingTreshold)
               || (linearDistance <= a_delta * a_vel.Length());

    if (weapon.GetThrowState() != ThrowState::kArriving) {
        weapon.SetThrowState(ThrowState::kArriving);
        if (!isCatchable) {
            weapon.GetSoundManager().PlayArrivingStartSounds(reinterpret_cast<RE::NiAVObject**>(&weapon.projectileModel));
            weapon.GetSoundManager().PlayArrivingLoopSounds (reinterpret_cast<RE::NiAVObject**>(&weapon.projectileModel));
        }
    }
    if (isCatchable) {
        if (weapon.GetThrowState() == ThrowState::kArriving)
            weapon.SetThrowState(ThrowState::kArrived);
        weapon.Catch();
        return Status::kCompleted;
    }

    InitiateTransform();
    UpdateArrivingRoute(a_delta);

    const float minArrivalTime = a_delta * 2.f;
    float remainingRouteLength = arrivingRoute.arcLength - closestSample.distanceFromStart;
    remainingRouteLength = std::max(remainingRouteLength, linearDistance);
    remainingTimeToArrive = std::max(timeToArrive - livingTime, minArrivalTime);

    const float requiredAverageSpeed = remainingRouteLength / remainingTimeToArrive;
    if (requiredAverageSpeed < Config::MinArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MinArrivalSpeed, minArrivalTime);
    else if (requiredAverageSpeed > Config::MaxArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MaxArrivalSpeed, minArrivalTime);

    const float desiredAcceleration = 2.f * (remainingRouteLength - speed * remainingTimeToArrive)
                                    / (remainingTimeToArrive * remainingTimeToArrive);
    speed += desiredAcceleration * a_delta;
    speed = std::max(speed, Config::MinArrivalSpeed);

    remainingTimeToArrive = remainingRouteLength / speed;

    constexpr float nearArriveTimeThreshold = 0.369f;
    isNear = remainingTimeToArrive < (nearArriveTimeThreshold < timeToArrive ? nearArriveTimeThreshold : timeToArrive * 0.8f);
    constexpr float almostArrivedTimeThreshold = 0.169f;
    isAlmostArrived = remainingTimeToArrive < (almostArrivedTimeThreshold < timeToArrive ? almostArrivedTimeThreshold : timeToArrive * 0.5f);

    if (isAlmostArrived || isCatchable) {
        const uint16_t fadeDuration = (uint16_t)(almostArrivedTimeThreshold * 1000.f) + 200u;
        weapon.GetSoundManager().FadeArrivingStartSounds(fadeDuration);
        weapon.GetSoundManager().FadeArrivingLoopSounds (fadeDuration);
        weapon.GetSoundManager().FadeCallingHandSounds  (fadeDuration);
    } else if (isNear) {
        weapon.GetSoundManager().PlayArrivingNearSounds(reinterpret_cast<RE::NiAVObject**>(&weapon.projectileModel));
    }

    tReal = timeToArrive / (timeToArrive + 2.f * (remainingTimeToArrive - 0.05f));
    tReal = std::clamp(tReal * tReal, 0.f, 1.f);

    const float smoothTime = std::clamp(1.f - tReal, 0.01f, 0.069f);
    const float alpha      = 1.f - std::exp(-a_delta / smoothTime);
    smoothedDesiredVelocity += (bezierDir * speed - smoothedDesiredVelocity) * alpha;
    desiredDir = smoothedDesiredVelocity;
    desiredDir.Unitize();

    const bool doBlend = weapon.projState == ProjectileState::kNone
                      || weapon.projState == ProjectileState::kLaunched;
    const auto referenceStartVelocity = doBlend ? weapon.velocity : (linearArrivingDir * speed);
    a_vel = MathUtil::Angle::BlendVectors(referenceStartVelocity, desiredDir * speed, tau);

    DampVerticalVelocity(a_vel, handPosition, -40.f, 69.f);

    UpdateRotation();
    UpdateArrivingDirection();

    if (a_target) Attack(a_target, a_vel);

    return Status::kRunning;
}
