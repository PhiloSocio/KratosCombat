#pragma once

#include "ActorCapability.h"
#include "Types.h"

class BaseActor;
class RelicWeapon;
class SmartRelicWeapon;

class Caller : public ActorCapability
{
public:
    Caller();
    virtual ~Caller() = default;

    void OnAttach(BaseActor* a_actor) override;

    void HandleAction(const ActionType a_action) override;

    [[nodiscard]] ActorType GetCapabilityType() const override { return ActorType::kCaller; }

    RelicWeapon* GetCallableRelic();
    void CallWeapon(SmartRelicWeapon* a_weaponToCall = nullptr);

protected:
    float alterationLevel = 0.f;

private:
    RelicWeapon* _weaponToCall = nullptr;
};
