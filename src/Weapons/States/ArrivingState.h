#pragma once
#include "ThrowableWeaponState.h"
#include "Settings.h"
#include "Util.h"

class ArrivingState : public ThrowableWeaponState
{
public:
    static constexpr std::array<float, 4> arrivingDirections {
        0.f, PI2, PI, ONEANDHALF_PI     //  b, r, f, l
    };

    ~ArrivingState() override = default;

    ArrivingState(
        SmartRelicWeapon& a_weapon,
        const RE::NiPoint3& a_startPosition,
        RE::NiAVObject** a_targetBone);
    ArrivingState(
        const ArrivingState& a_previous,
        const RE::NiPoint3& a_startPosition);

protected:
    RE::ActorHandle callerActorHandle;
    RE::Actor* callerActor = nullptr;
    RE::NiAVObject** callerHandBoneSource = nullptr;
    RE::NiAVObject* callerHandBone = nullptr;
    RE::NiAVObject** callerWeaponBoneSource = nullptr;
    RE::NiAVObject* callerWeaponBone = nullptr;
    RE::NiPointer<RE::NiAVObject> callerBreastBone;
    RE::NiAVObject* model;
    RE::NiMatrix3 startRotation = RE::NiMatrix3();
    RE::NiPoint3 handPosition;
    RE::NiPoint3 startPosition;
    RE::NiPoint3 position;
    RE::NiPoint3 currentDir;
    RE::NiPoint3 bezierDir;
    RE::NiPoint3 desiredDir;
    RE::NiPoint3 smoothedDesiredVelocity;
    RE::NiPoint3 linearArrivingDir;
    MathUtil::Algebra::BezierCurve arrivingRoute;
    MathUtil::Algebra::BezierSample closestSample;
    RE::NiPoint3 bezierControlPoints[4];
    int arrivingRouteClosestIndex = 0;
    uint16_t routeResolution = 64;
    std::vector<RE::ActorHandle> targets;
    bool  isNear = false;
    bool  isAlmostArrived = false;
    bool  isCatchable = false;
    bool  isInCallingAnimation = false;
    float startingTime = 0.f;
    float livingTime = 0.f;
    float timeToArrive = Config::ArrivalTime;
    float remainingTimeToArrive = Config::ArrivalTime;
    float tReal = 0.f;
    float rotationSpeed = Config::ArrivalRotationSpeed; //  rad/s
    float arrivalSpin = 0.f; //  rad
    float linearDistance = 0.f;
    float linearDistanceFromStart = 0.f;
    float linearDistanceFromLastCallPos = 0.f;
    float arrivingRelativeAngleZ = 0.5f;
    float arrivingRelativeAngleSnapStrength = Config::ArrivalAngleSnap;
    float speed = 0.f;

private:
    void InitializeCallerData()
    {
        callerActor = (weapon.GetCaller() && weapon.GetCaller()->IsValid()) ? weapon.GetCaller()->GetActor() : nullptr;

        callerBreastBone.reset(
            callerActor
                ? callerActor->GetNodeByName("NPC Spine2 [Spn2]")
                : nullptr);

        callerWeaponBoneSource = &weapon.GetCaller()->GetWeaponBone();
        callerWeaponBone = GetCallerWeaponBone();
        callerHandBone = GetCallerHandBone();

        const auto callerHandPosition =
            callerHandBone
                ? callerHandBone->world.translate
                : callerActor
                    ? callerActor->GetPosition()
                    : RE::NiPoint3();

        linearArrivingDir = callerHandPosition - startPosition;
        linearArrivingDir.Unitize();

        linearDistanceFromStart = startPosition.GetDistance(callerHandPosition) + 1.f;

        linearDistanceFromLastCallPos = linearDistanceFromStart;
    }
    void InitializeRoute()
    {
        arrivingRoute = MathUtil::Algebra::BezierCurve();
        arrivingRouteClosestIndex = 0;

        bezierControlPoints[0] = startPosition;
        bezierControlPoints[1] =
            startPosition +
            linearArrivingDir *
            linearDistanceFromLastCallPos *
            0.33f;
    }

public:
    float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - startingTime;}
    RE::NiAVObject* GetCallerHandBone() const {return callerHandBoneSource ? *callerHandBoneSource : nullptr;}
    RE::NiAVObject* GetCallerWeaponBone() const {return callerWeaponBoneSource ? *callerWeaponBoneSource : nullptr;}
    void UpdateTargets(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
        targets = ObjectUtil::Actor::GetNearCombatTargetHandles<std::vector<RE::ActorHandle>>(callerActor, linearDistance, true);
        CheckTargets(a_origin.has_value() ? *a_origin : startPosition);
    }
    void CheckTargets(const RE::NiPoint3& a_origin) {
        if (!targets.empty()) {
            std::erase_if(targets, [this, a_origin](const RE::ActorHandle actorHandle) {
                bool result = false;
                auto& weaponRTD = weapon.GetruntimeData();
                auto actor = actorHandle.get().get();
                if (!actor || actor->IsDead() || std::find(weaponRTD.lastHitActors.begin(), weaponRTD.lastHitActors.end(), actorHandle) != weaponRTD.lastHitActors.end()) {
                    result = true;
                } else {
                    auto targetPos = actor->GetPosition() + (actor->GetBoundMax() + actor->GetBoundMin()) * 0.75f;
                    auto targetDir = targetPos - a_origin;
                    targetDir.Unitize();
                    float distance = actor->GetPosition().GetDistance(a_origin);
                    result = distance > linearDistance
                        || currentDir.Dot(targetDir) < std::cos(PI8)
                        || currentDir.Dot(linearArrivingDir) < std::cos(PI8);
                }
                return result;
            });
            std::sort(targets.begin(), targets.end(), 
                [&](const auto& a, const auto& b) {
                    auto aRaw = a.get().get();
                    auto bRaw = b.get().get();
                    return aRaw->GetPosition().GetDistance(a_origin) < 
                        bRaw->GetPosition().GetDistance(a_origin);
                }
            );
        };
    }
    RE::Actor* GetNextTarget(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
        if (a_origin) {
            UpdateTargets(a_origin);
        }
        return !targets.empty() ? targets.front().get().get() : nullptr;
    }
    bool IsInCallingAnimation() {
        if (callerActor) {
            callerActor->GetGraphVariableBool("bIsInCallingAnimation", isInCallingAnimation);
        } return isInCallingAnimation;
    }

    void Enter() override;
    Status Update(float a_delta) override;
    void Exit() override;

    [[nodiscard]] Type GetType() const override { return Type::kArriving; };
    [[nodiscard]] std::vector<RE::ActorHandle>& GetTargets() { return targets; };

    void UpdateRotation();
    void UpdateAI(RE::NiPoint3& a_outVel);
    void UpdateArrivingDirection(bool a_initial = false);
    void UpdateArrivingRoute();

private:
    bool _justContinue = false;
};
