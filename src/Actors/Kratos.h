#pragma once

#include "Thrower.h"
#include "Charger.h"
#include "Caller.h"
#include "Rager.h"

class Kratos : public Thrower, public Caller, public Charger, public Rager
{
public:
    explicit Kratos(RE::ActorHandle a_actorHandle);
    ~Kratos() override = default;

    void DoAction(const ActionType a_action) override;

protected:

};
