#pragma once

#include "BaseActor.h"

class Rager : virtual public BaseActor
{
public:
    explicit Rager(RE::ActorHandle a_actorHandle) :
        BaseActor(a_actorHandle)
    {}
    virtual ~Rager() = default;

    void InitiateRage();
    void StartRage();
    void EndRage(const bool a_fromAnnotation = false, const bool a_playAnim = true, const bool a_justAnim = false);

    [[nodiscard]] bool IsInRage() const { return isInRage; }

    [[nodiscard]] RageType GetRageType() const { return rageType; }
    [[nodiscard]] float GetRageAmount() const { return rage; }
    [[nodiscard]] float GetRageLimit() const { return rageLimit; }
    [[nodiscard]] float GetBuffAmount() const { return rageBuffAmount; }
    [[nodiscard]] float GetRageDamageAmount() const { return rageDamageAmount; }

    void SetRageAmount(float a_rageAmount) noexcept { rage = a_rageAmount; }
    void SetRageLimit(float a_rageLimit) noexcept { rageLimit = a_rageLimit; }
    void SetRageBuffAmount(float a_rageBuffAmount) noexcept { rageBuffAmount = a_rageBuffAmount; }
    void SetRageDamageAmount(float a_rageDamageAmount) noexcept { rageDamageAmount = a_rageDamageAmount; }
    void SetRageType(RageType a_rageType) noexcept { rageType = a_rageType; }

protected:
    bool isInRage             = false;
    bool isWantFinishRage     = false;
    bool gettingHittedInValor = false;

    float rage             = 0.f;
    float rageLimit        = 0.f;
    float rageBuffAmount   = 0.f;
    float rageDamageAmount = 0.f;

    RageType rageType = RageType::kFury;
    RageType lastTriggeredRage = RageType::kFury;
};
