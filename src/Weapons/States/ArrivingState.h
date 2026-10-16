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
    ArrivingState(SmartRelicWeapon& a_weapon);

protected:
    RE::ActorHandle  callerActorHandle;
    RE::Actor*       callerActor = nullptr;
    RE::NiAVObject** callerHandBoneSource = nullptr;
    RE::NiAVObject*  callerHandBone = nullptr;
    RE::NiAVObject** callerWeaponBoneSource = nullptr;
    RE::NiAVObject*  callerWeaponBone = nullptr;
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
    uint16_t arrivingRouteClosestIndex = 0u;
    uint16_t routeResolution = 64u;
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

public:
    float GetLivingTime() const {return AsyncUtil::GameTime::GetEngineTime() - startingTime;}
    RE::NiAVObject* GetCallerHandBone() const {return callerHandBoneSource ? *callerHandBoneSource : nullptr;}
    RE::NiAVObject* GetCallerWeaponBone() const {return callerWeaponBoneSource ? *callerWeaponBoneSource : nullptr;}

    void Enter() override;
    Status Update(float a_delta) override;
    void Exit() override;

    [[nodiscard]] Type GetType() const override { return Type::kArriving; };
    [[nodiscard]] std::vector<RE::ActorHandle>& GetTargets() { return targets; };
    [[nodiscard]] RE::Actor* GetNextTarget(std::optional<RE::NiPoint3> a_origin = std::nullopt);
    [[nodiscard]] bool IsInCallingAnimation();

    void InitializeCallerData();
    void InitializeRoute();
    void InitiateTransform();
    void UpdateRotation();
    void UpdateAI(RE::NiPoint3& a_outVel);
    void UpdateArrivingDirection(bool a_initial = false);
    void UpdateArrivingRoute();
    void UpdateTargets(std::optional<RE::NiPoint3> a_origin = std::nullopt);
    void CheckTargets(const RE::NiPoint3& a_origin);

private:
    bool _transformInitiated = false;
};
