#pragma once

#include "BaseActor.h"
#include "Weapons/RelicWeapon.h"

class Charger : virtual public BaseActor
{
public:
    explicit Charger(RE::ActorHandle a_actorHandle) :
        BaseActor(a_actorHandle)
    {}
    virtual ~Charger() = default;

    bool IsCanCharge(const RelicType a_relic) const
    {
        const auto index = static_cast<std::size_t>(a_relic);

        if (index >= canCharge.size()) {
            return false;
        }

        return canCharge[index];
    }

protected:
    std::array<bool, static_cast<std::size_t>(RelicType::kCount)> canCharge{};
};
