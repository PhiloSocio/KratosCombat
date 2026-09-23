#pragma once

#include "BaseActor.h"

class ThrowableRelicWeapon;

class Thrower : virtual public BaseActor
{
public:
    explicit Thrower(RE::ActorHandle a_actorHandle) :
        BaseActor(a_actorHandle)
    {}
    virtual ~Thrower() = default;

    void ThrowWeapon(const bool isVertical, const bool isHoming = false);

    void Update(float a_delta) override;
    [[nodiscard]] float GetImpulsePower(const float a_mass) noexcept;     //  [mN.s] (linear impulse)
    [[nodiscard]] float GetChargeMultiplier() const noexcept;

    bool IsThrowing(const ThrowType a_type = ThrowType::kAny) noexcept;
    void StartChargingThrow() noexcept;
    void StopChargingThrow() noexcept;

    ThrowableRelicWeapon* GetLastThrownRelic() const {return lastThrownRelic;}

private:
    bool _isNormalThrowing = false;
    bool _isPowerThrowing = false;
    bool _isCharging = false;
    float _chargeDuration = 0.f;
    float _chargeImpulseDuration = 0.f;

protected:
    ThrowableRelicWeapon* lastThrownRelic = nullptr;

    float explosiveStrength = 80.f;     //  [N]
    float peakStrength = 120.f;         //  [N]
};
