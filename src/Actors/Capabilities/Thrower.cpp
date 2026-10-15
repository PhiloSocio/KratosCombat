#include "Thrower.h"
#include "Assets.h"
#include "util.h"
#include "Weapons/SmartRelicWeapon.h"
#include "RelicManager.h"

using namespace MathUtil;

constexpr float maxEffectiveChargeDuration = 1.5f;
constexpr float maxChargeDuration = 3.f;

Thrower::Thrower()
{
}

bool Thrower::IsThrowing(const ThrowType a_type) noexcept
{
    bool ret = false;

    auto actor = parent && parent->IsValid() ? parent->GetActor() : nullptr;
    if (actor) {
        actor->GetGraphVariableBool("bIsThrowing", _isNormalThrowing);
        actor->GetGraphVariableBool("bIsPowerThrowing", _isPowerThrowing);
    }

    switch (a_type) {
    case ThrowType::kAny:                   ret = _isNormalThrowing || _isPowerThrowing;    break;
    case ThrowType::kNormalThrow:           ret = _isNormalThrowing;                        break;
    case ThrowType::kPowerThrow:            ret = _isPowerThrowing;                         break;
    case ThrowType::kChargingThrow:         ret = _isCharging;                              break;
    case ThrowType::kPowerChargingThrow:    ret = _isCharging && _isPowerThrowing;          break;
    default:                                ret = false;                                    break;
    }

    return ret;
}
void Thrower::StartChargingThrow() noexcept
{
    if (!parent->IsValid()) return;
    _chargeDuration = 0.f;
    _isCharging = true;
    auto& rHandBone = parent->GetRHandBone();
    cachedRelic = parent->GetRightHandRelic() ? parent->GetRightHandRelic()->As<ThrowableRelicWeapon>() : nullptr;
    if (auto assets = Assets::GetSingleton(); assets && rHandBone && cachedRelic) {
        cachedRelic->GetSoundManager().PlayChargingLoopSounds(reinterpret_cast<RE::NiAVObject**>(&rHandBone));
        parent->GetActor()->ApplyArtObject(assets->VFXeffects.handFrostBright, 5.f, nullptr, false, false, rHandBone);
    }
}
void Thrower::StopChargingThrow() noexcept
{
    if (_isCharging) {
        if (cachedRelic) {
            cachedRelic->GetSoundManager().StopChargingLoopSounds(*g_deltaTimeRealTime * 1200.f);
            cachedRelic = nullptr;
        }
        _isCharging = false;
    }
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

void Thrower::ThrowWeapon(const RotationType a_rotationType, const ThrowType a_throwType)
{
    auto rRelic = parent->GetRightHandRelic();
    if (auto thrownWeapon = rRelic ? rRelic->As<ThrowableRelicWeapon>() : nullptr) {
        if(const bool success = thrownWeapon->Throw(this, a_rotationType); success) {
            StopChargingThrow();
            lastThrownRelic = thrownWeapon;
            if (a_throwType == ThrowType::kHomingThrow) {
                if (auto smartRelicWeapon = thrownWeapon->As<SmartRelicWeapon>(); smartRelicWeapon) {
                    smartRelicWeapon->SetState(RelicWeaponState::Type::kHoming);
                }
            }
            RelicManager::GetSingleton()->OnRelicThrow(thrownWeapon->projectile, thrownWeapon);
        } else {
            spdlog::error("throw failed");
        }
    } else if (parent->GetRightHandRelic()) {
        spdlog::warn("relic is not throwable!");
    } else {
        spdlog::warn("your weapon is not a relic");
    }
}

void Thrower::OnAttach(BaseActor* a_actor)
{
    ActorCapability::OnAttach(a_actor);
    if (parent && parent->IsValid()) {
        if (auto callerAVO = parent->GetActor() ? parent->GetActor()->AsActorValueOwner() : nullptr) {
            archeryLevel = callerAVO->GetActorValue(RE::ActorValue::kArchery);
        }
    }
}

void Thrower::Update(float a_delta)
{
    if (_isCharging) {
        _chargeDuration += a_delta;
        if (_chargeDuration > maxChargeDuration) _chargeDuration = maxChargeDuration;
    }
}
void Thrower::HandleAction(const ActionType a_action)
{
    // Thrower-specific actions handled via parent if needed
}
