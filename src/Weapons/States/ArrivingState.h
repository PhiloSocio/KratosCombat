#pragma once

#include "SmartWeaponState.h"
#include "Settings.h"
#include "util.h"

class ArrivingState : public SmartWeaponState
{
public:
    inline static constexpr auto kType = Type::kArriving;

    static constexpr std::array<float, 4> arrivingDirections {
        0.f, PI2, PI, ONEANDHALF_PI     //  b, r, f, l
    };

    ~ArrivingState() override = default;
    ArrivingState(SmartRelicWeapon& a_weapon);

    void Enter() override;
    void Exit() override;
    
    [[nodiscard]] Type GetType() const override { return Type::kArriving; };

    [[nodiscard]] bool            IsInCallingAnimation();
    [[nodiscard]] RE::NiAVObject* GetCallerHandBone() const;
    [[nodiscard]] RE::NiAVObject* GetCallerWeaponBone() const;

protected:
    void   RefreshTargets() override;
    Status MainProcess(float a_delta, RE::Actor* a_target, RE::NiPoint3& a_vel) override;
    void   OnAttackEnd(RE::NiPoint3& a_vel) override;
    [[nodiscard]] bool IsTargetValid(RE::Actor* a_target) const override;

    void InitializeCallerData();
    void InitializeRoute();
    void InitiateTransform();
    void UpdateRotation();
    void UpdateArrivingDirection(bool a_initial = false);
    void UpdateArrivingRoute(const float a_delta);

    static constexpr float hitDurationBudget = 0.3f;

    RE::Actor*       callerActor = nullptr;
    RE::NiAVObject** callerHandBoneSource = nullptr;
    RE::NiAVObject*  callerHandBone = nullptr;
    RE::NiAVObject** callerWeaponBoneSource = nullptr;
    RE::NiAVObject*  callerWeaponBone = nullptr;
    RE::NiPointer<RE::NiAVObject> callerBreastBone;
    RE::NiAVObject* model = nullptr;
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
    bool  isNear = false;
    bool  isAlmostArrived = false;
    bool  isCatchable = false;
    bool  isInCallingAnimation = false;
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

private:
    bool _transformInitiated = false;
};
