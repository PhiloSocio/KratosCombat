#pragma once

#include "ActorCapability.h"
#include "Types.h"

class Charger : public ActorCapability
{
public:
    Charger();
    virtual ~Charger() = default;

    void HandleAction(const ActionType a_action) override;

    [[nodiscard]] ActorType GetCapabilityType() const override { return ActorType::kCharger; }

    void StartWeaponCharging();

    bool IsCanCharge(const RelicType a_relic) const;

protected:
    std::array<bool, static_cast<std::size_t>(RelicType::kCount)> canCharge{};
};
