#pragma once
#include "ThrowableWeaponState.h"
#include "Settings.h"
#include "Util.h"

template <class TWeapon>
class ArrivingState : public ThrowableWeaponState<TWeapon>
{
public:
    using Base = ThrowableWeaponState<TWeapon>;
    using Base::weapon;

    static constexpr std::array<float, 4> arrivingDirections {
        0.f, PI2, PI, ONEANDHALF_PI     //  b, r, f, l
    };

protected:
    RE::NiAVObject** callerHandBoneSource = nullptr;
    RE::NiAVObject* callerHandBone = nullptr;
    RE::NiAVObject** callerWeaponBoneSource = nullptr;
    RE::NiAVObject* callerWeaponBone = nullptr;
    RE::NiPointer<RE::NiAVObject> callerBreastBone;
    RE::NiPointer<RE::NiAVObject> model;
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
    std::vector<RE::Actor *> targets;
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
        callerBreastBone.reset(
            weapon.data.caller
                ? weapon.data.caller->GetNodeByName("NPC Spine2 [Spn2]")
                : nullptr);

        callerWeaponBoneSource = weapon.data.caller->GetNodeByName("WEAPON");
        callerWeaponBone = GetCallerWeaponBone();
        callerHandBone = GetCallerHandBone();

        const auto callerHandPosition =
            callerHandBone
                ? callerHandBone->world.translate
                : weapon.data.caller
                    ? weapon.data.caller->GetPosition()
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
        targets = ObjectUtil::Actor::GetNearCombatTargets<std::vector<RE::Actor*>>(weapon.data.caller.get(), linearDistance, true);
        CheckTargets(a_origin.has_value() ? *a_origin : startPosition);
    }
    void CheckTargets(const RE::NiPoint3& a_origin) {
        if (!targets.empty()) {
            std::erase_if(targets, [this](const RE::Actor* actor) {
                bool result = false;
                if (!actor || actor->IsDead() || std::find(weapon.data.lastHitActors.begin(), weapon.data.lastHitActors.end(), actor) != weapon.data.lastHitActors.end()) {
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
                    return a->GetPosition().GetDistance(a_origin) < 
                        b->GetPosition().GetDistance(a_origin);
                }
            );
        };
    }
    RE::Actor* GetNextTarget(std::optional<RE::NiPoint3> a_origin = std::nullopt) {
        if (a_origin) {
            UpdateTargets(a_origin);
        }
        return !targets.empty() ? targets.front() : nullptr;
    }
    bool IsInCallingAnimation() {
        if (weapon.data.caller) {
            weapon.data.caller->GetGraphVariableBool("bIsInCallingAnimation", isInCallingAnimation);
        } return isInCallingAnimation;
    }

    void Enter() override;
    Status Update(float a_delta) override;
    void Exit() override;

    [[nodiscard]] Type GetType() const override { return Type::kArriving; };

    void UpdateRotation();
    void UpdateAI(RE::NiPoint3& a_outVel);
    void UpdateArrivingDirection(bool a_initial = false);
    void UpdateArrivingRoute();

    virtual ~ArrivingState() = default;

    ArrivingState(
        TWeapon& a_weapon,
        const RE::NiPoint3& a_startPosition,
        RE::NiAVObject** a_targetBone);
    ArrivingState(
        const ArrivingState& a_previous,
        const RE::NiPoint3& a_startPosition);

private:
    bool _justContinue = false;
};
