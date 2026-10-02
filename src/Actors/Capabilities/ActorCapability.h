#pragma once

#include "Actors/BaseActor.h"
#include "Types.h"

class BaseActor;

class ActorCapability
{
public:
    virtual ~ActorCapability() = default;

    virtual void OnAttach(BaseActor* a_actor) { parent = a_actor; }
    virtual void OnDetach(BaseActor* a_actor) { parent = nullptr; }

    virtual void Update(float a_delta) {}
    virtual void HandleAction(const ActionType a_action) {}

    [[nodiscard]] virtual ActorType GetCapabilityType() const = 0;
    [[nodiscard]] virtual bool IsActive() const { return true; }

    [[nodiscard]] BaseActor* GetParent() const { return parent; }

protected:
    BaseActor* parent = nullptr;
};

using ActorCapabilityPtr = std::unique_ptr<ActorCapability>;