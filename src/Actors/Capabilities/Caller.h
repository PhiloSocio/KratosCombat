#pragma once

#include "ActorCapability.h"
#include "Types.h"

class BaseActor;
class RelicWeapon;

class Caller : public ActorCapability
{
public:
    Caller();
    virtual ~Caller() = default;

    void HandleAction(const ActionType a_action) override;

    [[nodiscard]] ActorType GetCapabilityType() const override { return ActorType::kCaller; }

    RelicWeapon* GetCallableRelic();
    void CallWeapon();

protected:
    float alterationLevel = 0.f;

private:
    RelicWeapon* _weaponToCall = nullptr;
};
