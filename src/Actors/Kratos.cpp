#include "Kratos.h"
#include "Weapons/RelicWeapon.h"
#include "Capabilities/Thrower.h"
#include "Capabilities/Charger.h"
#include "Capabilities/Caller.h"
#include "Capabilities/Rager.h"

Kratos::Kratos(RE::ActorHandle a_actorHandle) :
    BaseActor(a_actorHandle)
{
    AddCapability(std::make_unique<Thrower>());
    AddCapability(std::make_unique<Charger>());
    AddCapability(std::make_unique<Caller>());
    AddCapability(std::make_unique<Rager>());
}

void Kratos::Update(float a_delta)
{
    for (const auto& capabilityPtr : actorCapabilities) {
        auto capability = capabilityPtr.get();
        if (capability && capability->IsActive()) {
            capability->Update(a_delta);
        }
    }
}
void Kratos::DoAction(const ActionType a_action)
{
    if (IsValid())
        switch (a_action)
        {
        case ActionType::kRage:
            if (auto rageCap = GetCapabilityAs<Rager>()) {
                rageCap->HandleAction(ActionType::kRage);
            }
            break;
        case ActionType::kWeaponCharge:
            if (auto chargerCap = GetCapabilityAs<Charger>()) {
                chargerCap->HandleAction(ActionType::kWeaponCharge);
            }
            break;
        case ActionType::kSpecialIdle:
            if (!IsInRage()) {
                GetActor()->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kSpecialIdle);
                GetActor()->NotifyAnimationGraph("DoKratosAction");
            }
            break;
        case ActionType::kWeaponCall:
            if (!IsInRage() && !GetRightHandRelic()) {
                if (auto callerCap = GetCapabilityAs<Caller>()) {
                    callerCap->HandleAction(ActionType::kWeaponCall);
                }
            } else if (IsInRage()) {
                //  todo
            }
            break;

        default:
            break;
        }
}
