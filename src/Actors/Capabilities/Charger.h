#pragma once

#include "ActorCapability.h"
#include "Types.h"

class Charger : public ActorCapability
{
public:
    inline static constexpr auto kType = ActorType::kCharger;

    Charger();
    virtual ~Charger() = default;

    void OnAttach(BaseActor* a_actor) override;

    void HandleAction(const ActionType a_action) override;

    [[nodiscard]] ActorType GetType() const override { return ActorType::kCharger; }

    void StartWeaponCharging();
    void StopWeaponCharging();

    bool IsCanCharge(const RelicType a_relic) const;

protected:
    float enchantingLevel = 0.f;
    std::array<bool, static_cast<std::size_t>(RelicType::kCount)> canCharge{};
};
