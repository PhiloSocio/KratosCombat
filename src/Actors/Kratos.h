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

    void Update(float a_delta) override;
    void DoAction(const ActionType a_action) override
    {
        if (IsValid())
            switch (a_action)
            {
            case ActionType::kRage:
                if (!IsInRage()) InitiateRage();
                else EndRage();
                break;
            case ActionType::kWeaponCharge:
                if (!IsInRage() && !GetRightHandRelic()->IsCharged()) {
                    GetActor()->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kWeaponCharge);
                    GetActor()->NotifyAnimationGraph("DoKratosAction");
                }
                break;
            case ActionType::kSpecialIdle:
                if (!IsInRage()) {
                    GetActor()->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kSpecialIdle);
                    GetActor()->NotifyAnimationGraph("DoKratosAction");
                }
                break;
            case ActionType::kWeaponCall:
                if (!IsInRage() && !GetRightHandRelic() && GetLastRightHandRelic()->GetOwner()) {
                    GetActor()->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kWeaponCharge);   //  intentionally
                    GetActor()->NotifyAnimationGraph("DoKratosAction");
                }
                break;

            default:
                break;
            }
    }

protected:

};
