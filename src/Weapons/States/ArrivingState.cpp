#include "ArrivingState.h"
#include "Weapons/SmartRelicWeapon.h"

ArrivingState::ArrivingState(SmartRelicWeapon& a_weapon, const RE::NiPoint3& a_startPosition, RE::NiAVObject** a_targetBone)
    : ThrowableWeaponState(a_weapon),
        callerHandBoneSource(a_targetBone),
        startPosition(a_startPosition),
        _justContinue(false)
{
}
ArrivingState::ArrivingState(const ArrivingState& a_previous, const RE::NiPoint3& a_startPosition)
    : ThrowableWeaponState(a_previous.weapon),
        callerHandBoneSource(
            a_previous.callerHandBoneSource),
        startPosition(a_startPosition),
        startingTime(a_previous.startingTime),
        callerBreastBone(a_previous.callerBreastBone),
        arrivingRoute(a_previous.arrivingRoute),
        arrivingRouteClosestIndex(a_previous.arrivingRouteClosestIndex),
        _justContinue(true)
{
}

void ArrivingState::UpdateRotation()
{
    if (weapon.runtimeData.replacedProjectileModel && weapon.runtimeData.replacedProjectileModel->parent) {
        auto& replacedPMParent = weapon.runtimeData.replacedProjectileModel->parent;
        auto& localRotation = replacedPMParent->local.rotate;
        RE::NiMatrix3 targetLocalRotation;
    //    float targetAngleZ;
        if (replacedPMParent->parent) {
            targetLocalRotation = replacedPMParent->parent->world.rotate.Transpose() * callerWeaponBone->world.rotate;
        //    targetAngleZ = std::atan2(
        //        targetLocalRotation.entry[0][1],
        //        targetLocalRotation.entry[0][0]
        //    );
        } else {
            targetLocalRotation = callerWeaponBone->world.rotate;
        //    targetAngleZ = std::atan2(
        //        targetLocalRotation.entry[0][1],
        //        targetLocalRotation.entry[0][0]
        //    );
        }
        const float angleZ = livingTime * rotationSpeed;
        MathUtil::Angle::NormalAbsoluteAngle(angleZ);
        arrivalSpin = (int)(std::floor(1 + (livingTime + remainingTimeToArrive) * rotationSpeed / TWO_PI)) * TWO_PI;
        const float blendZ = std::lerp(angleZ, arrivalSpin, tReal);
        const auto& startRotationC = startRotation;
        localRotation = MathUtil::Algebra::InterpolateRotation(startRotationC, targetLocalRotation, tReal);
        localRotation = localRotation * RE::NiMatrix3(0.f, 0.f, blendZ);
    //    spdlog::debug("target angle: {} calculated target angle: {} blended angle: {} tau: {}", angleZ, arrivalSpin, blendZ, tReal);
    }
//    leviAngle.x = asin(desiredDir.z);
//    leviAngle.z = atan2(desiredDir.x, desiredDir.y);
//    if (leviAngle.z < 0.0) {
//        leviAngle.z += PI;
//    }
//    if (desiredDir.x < 0.0) {
//        leviAngle.z += PI;
//    }
}
void ArrivingState::UpdateAI(RE::NiPoint3& a_outVel)
{
    float height = position.z - callerActor->GetPosition().z;
    if (!Config::DontDamageWhileArrive && tReal < 0.69f) {
        if (auto aTarget = GetNextTarget(position); aTarget) {
            auto targetPos = aTarget->GetPosition() + (aTarget->GetBoundMax() + aTarget->GetBoundMin()) * 0.75f;
            auto targetDir = (targetPos - position);
            targetDir.Unitize();
            height = position.z - aTarget->GetPosition().z;
            a_outVel = targetDir * speed;
        }
    }
}
void ArrivingState::UpdateArrivingDirection(const bool a_initial)
{
    if (RE::PlayerCamera::GetSingleton()->IsInFirstPerson()) {

    } else if (callerActor && callerBreastBone) {
        if (weapon.GetThrowState() == ThrowState::kThrowable || isCatchable) {

        } else if (a_initial || linearDistance > 100.f) {
            RE::NiPoint3  spineForwardDir = callerBreastBone->world.rotate * RE::NiPoint3(frontVec3);
            spineForwardDir.z = 0.f;  //  ignore vertical direction
            spineForwardDir.Unitize();

            RE::NiPoint3 linearDir2D(linearArrivingDir.x, linearArrivingDir.y, 0.f);
            linearDir2D.Unitize();

            float dot = spineForwardDir.Dot(linearDir2D);
            float det = spineForwardDir.x * linearDir2D.y - spineForwardDir.y * linearDir2D.x;

            arrivingRelativeAngleZ = atan2(det, dot);  //  angle between spine forward direction and arriving weapon direction
            arrivingRelativeAngleZ = MathUtil::Angle::NormalAbsoluteAngle(arrivingRelativeAngleZ);      //  normalize angle to [0, 2PI]

            float snapStrength = arrivingRelativeAngleSnapStrength;
            if (a_initial) snapStrength = 1.f;
            else if ((arrivingRelativeAngleZ < PI4 || arrivingRelativeAngleZ > ONEANDHALF_PI) && snapStrength < 0.69f) snapStrength += 0.69f;
            snapStrength = std::min(snapStrength, 1.f);
            if (snapStrength > 0.f) {
                arrivingRelativeAngleZ = MathUtil::Algebra::AttractToNearest(arrivingRelativeAngleZ, arrivingDirections, snapStrength);    //  for helping to the blender generator
            }

            if (!a_initial) {
                float previousAngle; callerActor->GetGraphVariableFloat("fArrivingWeaponDirection", previousAngle); previousAngle *= TWO_PI;
                const float delta = MathUtil::Angle::NormalizeSignedAngle(arrivingRelativeAngleZ - previousAngle);
                constexpr float smoothTime = 0.369f;
                const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
                float smoothedArrivingRelativeAngle = previousAngle;
                smoothedArrivingRelativeAngle += delta * alpha;
                arrivingRelativeAngleZ = smoothedArrivingRelativeAngle;
                
            }
            arrivingRelativeAngleZ /= TWO_PI;
        }
        callerActor->SetGraphVariableFloat("fArrivingWeaponDirection", arrivingRelativeAngleZ);
    }
}
void ArrivingState::UpdateArrivingRoute()
{
    RE::NiMatrix3 handRot   = callerHandBone->world.rotate;
    const float alphaHandRot = 1.f - std::exp(-*g_deltaTimeRealTime / 0.169f);
    RE::NiMatrix3 smoothedHandRot = 
        MathUtil::Algebra::QuaternionToMatrix(MathUtil::Algebra::Slerp(
            MathUtil::Algebra::MatrixToQuaternion(callerHandBone->previousWorld.rotate),
            MathUtil::Algebra::MatrixToQuaternion(callerHandBone->world.rotate), alphaHandRot));
    RE::NiPoint3 palmDir    = smoothedHandRot * RE::NiPoint3(backVec3);
    RE::NiPoint3 handForward= smoothedHandRot * RE::NiPoint3(upVec3);
    palmDir.Unitize();
    handForward.Unitize();

    const float handSideOffsetMult = MathUtil::Algebra::ParabolicClamp(arrivingRelativeAngleZ, 0.f, 0.25f);

    RE::NiPoint3 handVelocity = (callerHandBone->world.translate - callerHandBone->previousWorld.translate) / *g_deltaTimeRealTime;
    const float predictionTime = std::clamp(*g_deltaTimeRealTime, 0.0f, 0.1f);
    RE::NiPoint3 predictedHandPos = handPosition + handVelocity * predictionTime;
    bezierControlPoints[3] = predictedHandPos;
    bezierControlPoints[2] = predictedHandPos + (float)IsInCallingAnimation() * (palmDir * (linearDistanceFromLastCallPos * 0.33f + 20.f) + handForward * (linearDistanceFromLastCallPos * handSideOffsetMult + 10.f));

    RE::NiPoint3& p0 = bezierControlPoints[0];
    RE::NiPoint3& p1 = bezierControlPoints[1];
    RE::NiPoint3& p3 = bezierControlPoints[3];
    RE::NiPoint3& p2 = bezierControlPoints[2];

    arrivingRoute = MathUtil::Algebra::CalculateAndMeasureBezier(
        p0, p1, p2, p3,
        routeResolution);
    float bestDist2 = FLT_MAX;

    for (int i = 0; i < arrivingRoute.samples.size() - 1; i++) {
        const float d2 = (arrivingRoute.samples[i].point - position).SqrLength();
        if (d2 < bestDist2)
        {
            bestDist2 = d2;
            arrivingRouteClosestIndex = i;
        }
    }

    closestSample = arrivingRoute.samples[arrivingRouteClosestIndex];
    const float t = closestSample.t;

    const float lookAheadDistance = std::clamp(std::abs(speed) * 0.15f, 50.f, 200.f);
    const float targetDistance = closestSample.distanceFromStart + lookAheadDistance;

    int targetIndex = arrivingRouteClosestIndex;
    while (targetIndex + 1 <
        static_cast<int>(arrivingRoute.samples.size()) &&
        arrivingRoute.samples[targetIndex].distanceFromStart < targetDistance)
    {
        targetIndex++;
    }

    bezierDir = arrivingRoute.samples[targetIndex].point - position;
    bezierDir.Unitize();
}

void ArrivingState::Enter()
{
    InitializeCallerData();
    UpdateTargets();
    UpdateArrivingDirection(!_justContinue);
    InitializeRoute();
    smoothedDesiredVelocity = weapon.runtimeData.velocity;

    if (!_justContinue) {
        startingTime = AsyncUtil::GameTime::GetEngineTime();
        const bool doBlend = 
            weapon.runtimeData.projState == ProjectileState::kNone ||
            weapon.runtimeData.projState == ProjectileState::kLaunched;
        smoothedDesiredVelocity = doBlend ? weapon.runtimeData.velocity : linearArrivingDir * speed;
    }
}
Status ArrivingState::Update(const float a_delta)
{
    if (!weapon.GetCaller() || !weapon.GetCaller()->IsValid()) return Status::kCancelled;

    model = weapon.runtimeData.projectileModel;
    if (!model) return Status::kCancelled;

    callerHandBone = GetCallerHandBone();
    if (!callerHandBone) return Status::kCancelled;
    handPosition = callerHandBone->world.translate;

    callerWeaponBone = GetCallerWeaponBone();
    if (!callerWeaponBone) return Status::kCancelled;

    auto proj = weapon.runtimeData.projectile;
    if (!proj) return Status::kCancelled;
    auto& rtData = proj->GetProjectileRuntimeData();
    auto& vel = rtData.linearVelocity;

    currentDir = vel;
    currentDir.Unitize();

    position = proj->data.location;

    linearDistance = handPosition.GetDistance(position);
    linearArrivingDir = handPosition - position;
    linearArrivingDir.Unitize();

    isCatchable = (linearDistance <= Config::CatchingTreshold) || (linearDistance <= (*g_deltaTime * vel.Length()));

    if (weapon.GetThrowState() != ThrowState::kArriving) {  //  first frame of the arriving projectile
        weapon.SetThrowState(ThrowState::kArriving);

        if (!isCatchable) {
            weapon.GetSoundManager().PlayArrivingStartSounds(model);
            weapon.GetSoundManager().PlayArrivingLoopSounds(model);
        }
    }
    Status status = Status::kRunning;
    if (isCatchable) {
        if (weapon.GetThrowState() == ThrowState::kArriving) weapon.SetThrowState(ThrowState::kArrived);
        weapon.Catch();
        spdlog::debug("Levi proj catched");
        status = Status::kCompleted;
    }
    if (false && startRotation == RE::NiMatrix3()) {
        if (weapon.runtimeData.replacedProjectileModel && weapon.runtimeData.replacedProjectileModel->parent) {
            model->world = weapon.runtimeData.transformPW;
            model->local = weapon.runtimeData.transformPL;
            auto& replacedPMParent = weapon.runtimeData.replacedProjectileModel->parent;
            auto parentWorldInverse = replacedPMParent->world.Invert();
            auto previousWorld = weapon.runtimeData.transformW;
            auto& localRotation = replacedPMParent->local.rotate;
            auto& localPosition = replacedPMParent->local.translate;
            if (replacedPMParent->parent) {
                localRotation = parentWorldInverse.rotate * previousWorld.rotate;
            //    localPosition = parentWorldInverse.rotate * (previousWorld.translate - localPosition);
            } else {
                localRotation = previousWorld.rotate;
            //    localPosition = previousWorld.translate;
            }
            startRotation = localRotation;
            spdlog::debug("start rotation initiated");
        }
    }

    livingTime = GetLivingTime();
    UpdateArrivingRoute();

    const float minArrivalTime = *g_deltaTime * 2.f;
    float remainingRouteLength = arrivingRoute.arcLength - closestSample.distanceFromStart;
    remainingRouteLength = std::max(remainingRouteLength, linearDistance);
    remainingTimeToArrive = std::max(timeToArrive - livingTime, minArrivalTime);
    const float requiredAverageSpeed = remainingRouteLength / remainingTimeToArrive;

    if (requiredAverageSpeed < Config::MinArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MinArrivalSpeed, minArrivalTime);
    else if (requiredAverageSpeed > Config::MaxArrivalSpeed)
        remainingTimeToArrive = std::max(remainingRouteLength / Config::MaxArrivalSpeed, minArrivalTime);

    const float desiredAcceleration = 2.f * (remainingRouteLength - speed * remainingTimeToArrive) / (remainingTimeToArrive * remainingTimeToArrive);
    speed += desiredAcceleration * *g_deltaTimeRealTime;
    speed = std::max(speed, Config::MinArrivalSpeed);

    remainingTimeToArrive = remainingRouteLength / speed;
    constexpr float nearArriveTimeThreshold = 0.369f;
    isNear = remainingTimeToArrive < (nearArriveTimeThreshold < timeToArrive ? nearArriveTimeThreshold : timeToArrive * 0.8f);
    constexpr float almostArrivedTimeThreshold = 0.169f;
    isAlmostArrived = remainingTimeToArrive < (almostArrivedTimeThreshold < timeToArrive ? almostArrivedTimeThreshold : timeToArrive * 0.5f);
    if (isAlmostArrived || isCatchable) {
        uint16_t fadeDuration = (uint16_t)(almostArrivedTimeThreshold * 1000.f) + 200u;
        weapon.GetSoundManager().FadeArrivingStartSounds(fadeDuration);
        weapon.GetSoundManager().FadeArrivingLoopSounds(fadeDuration);
        weapon.GetSoundManager().FadeCallingHandSounds(fadeDuration);
    } else if (isNear) {
        weapon.GetSoundManager().PlayArrivingNearSounds(model);
    }
//    spdlog::debug(
//        "AFTER ACCEL speed={:.1f}, accel={:.1f}, L={:.1f}, T={:.3f}",
//        speed,
//        desiredAcceleration,
//        remainingRouteLength,
//        remainingTimeToArrive);
//    spdlog::debug(
//        "closest={} target={} closestT={:.3f} targetT={:.3f} "
//        "closestL={:.1f} targetL={:.1f} routeL={:.1f}",
//        arrivingRouteClosestIndex,
//        targetIndex,
//        closestSample.t,
//        arrivingRoute.samples[targetIndex].t,
//        closestSample.distanceFromStart,
//        arrivingRoute.samples[targetIndex].distanceFromStart,
//        arrivingRoute.arcLength
//    );

//    tReal = 1.f - remainingTimeToArrive / almostArrivedTimeThreshold;//timeToArrive;
    tReal = timeToArrive / (timeToArrive + 2.f * (remainingTimeToArrive - 0.05f));
    tReal = std::pow(tReal, 2.f);
    tReal = std::clamp(tReal, 0.f, 1.f);
//    tReal = tReal * tReal * (3.f - 2.f * tReal);

    const float smoothTime = std::clamp(1.f - tReal, 0.01f, 0.069f);
    const float alpha = 1.f - std::exp(-*g_deltaTimeRealTime / smoothTime);
    smoothedDesiredVelocity += (bezierDir * speed - smoothedDesiredVelocity) * alpha;
    desiredDir = smoothedDesiredVelocity;
    desiredDir.Unitize();
    const bool doBlend = 
        weapon.runtimeData.projState == ProjectileState::kNone ||
        weapon.runtimeData.projState == ProjectileState::kLaunched;
    vel = MathUtil::Angle::BlendVectors(doBlend ? weapon.runtimeData.velocity : (linearArrivingDir * speed), desiredDir * speed, livingTime / 0.2f);
    if (vel.z < 0.f) {
        constexpr float minHeight = -40.f;
        constexpr float dampingRange = 69.f;
        const float height = position.z - handPosition.z;
        const float dampFactor = std::clamp((height - minHeight) / dampingRange, 0.f, 1.f);
        vel.z *= dampFactor;
    }

    UpdateRotation();
    UpdateAI(vel);
    UpdateArrivingDirection();
    return status;
}
void ArrivingState::Exit()
{
}
