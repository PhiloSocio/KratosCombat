#pragma once

#include "BaseActor.h"

class Kratos : public BaseActor
{
public:
    explicit Kratos(RE::ActorHandle a_actorHandle);
    ~Kratos() override = default;

    void Update(float a_delta) override;
    void DoAction(const ActionType a_action) override;

protected:
};
