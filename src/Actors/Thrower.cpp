#include "Thrower.h"
#include "Weapons/ThrowableRelicWeapon.h"
#include "util.h"

using namespace MathUtil;

constexpr float maxEffectiveChargeDuration = 1.5f;
constexpr float maxChargeDuration = 3.f;

bool Thrower::IsThrowing(const ThrowType a_type) noexcept
{
    bool ret = false;

    actor->GetGraphVariableBool("bIsThrowing", _isNormalThrowing);
    actor->GetGraphVariableBool("bIsPowerThrowing", _isPowerThrowing);

    switch (a_type) {
    case ThrowType::kAny:                   ret = _isNormalThrowing || _isPowerThrowing;    break;
    case ThrowType::kNormalThrowing:        ret = _isNormalThrowing;                        break;
    case ThrowType::kPowerThrowing:         ret = _isPowerThrowing;                         break;
    case ThrowType::kChargingThrow:         ret = _isCharging;                              break;
    case ThrowType::kPowerChargingThrow:    ret = _isCharging && _isPowerThrowing;          break;
    default:                                ret = false;                                    break;
    }

    return ret;
}
void Thrower::StartChargingThrow() noexcept
{
    _chargeDuration = 0.f;
    _isCharging = true;
}
void Thrower::StopChargingThrow() noexcept
{
    _isCharging = false;
}

float Thrower::GetImpulsePower(const float a_mass) noexcept
{
    const float t = std::clamp(a_mass / 33.f, 0.f, 1.f);
    const float overallForce = Algebra::Interpolate(t * t, explosiveStrength, peakStrength);
    const float baseImpulseDuration = Algebra::Interpolate(std::sqrtf(t), 0.02f, 0.3f);
    _chargeImpulseDuration = baseImpulseDuration * GetChargeMultiplier();
    return 1000.f * overallForce * _chargeImpulseDuration;
}
float Thrower::GetChargeMultiplier() const noexcept
{
    return 1.f + maxEffectiveChargeDuration * _chargeDuration / (maxChargeDuration * maxChargeDuration);
}

void Thrower::ThrowWeapon(const bool isVertical, const bool isHoming)
{
    auto relic = GetRightHandRelic();
    if (auto throwableRelic = dynamic_cast<ThrowableRelicWeapon*>(relic)) {
        throwableRelic->Throw(isVertical, false, isHoming);
    }
}

void Thrower::Update(float a_delta)
{
    if (_isCharging) {
        _chargeDuration += a_delta;
        if (_chargeDuration > maxChargeDuration) _chargeDuration = maxChargeDuration;
    }
}