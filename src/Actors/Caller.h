#pragma once

#include "BaseActor.h"

class Caller : virtual public BaseActor
{
public:
    explicit Caller(RE::ActorHandle a_actorHandle) :
        BaseActor(a_actorHandle)
    {titles.set(ActorType::kCaller);}

    virtual ~Caller() = default;

    void CallWeapon() {};
    virtual RelicWeapon* GetCallableRelic();
protected:
    float alterationLevel = 0.f;
};
