#include "Caller.h"
#include "Weapons/SmartRelicWeapon.h"
#include "RelicManager.h"

Caller::Caller()
{
}

void Caller::CallWeapon(SmartRelicWeapon* a_weaponToCall)
{
    if (parent && parent->IsValid()) {
        if (a_weaponToCall && a_weaponToCall->Call(this, false)) {
            RelicManager::GetSingleton()->OnRelicThrow(a_weaponToCall->projectile, a_weaponToCall);
        } else if (auto weaponToCall = _weaponToCall ? _weaponToCall->As<SmartRelicWeapon>() : nullptr) {
            if (auto wielder = weaponToCall->GetWielder(); wielder && wielder->GetActor() != parent->GetActor()) {
            //    if (weaponToCall->Call(this, false, 1.618f)) {
            //        weaponToCall->currentReference = wielder->GetActor()->DropObject(weaponToCall->GetWeapon(), nullptr, 1, nullptr, nullptr).get().get();
                    spdlog::info("weapon forcefully called from {}", wielder->GetActor()->GetName());
            //    }
            } else if (weaponToCall->Call(this, false)) {
                RelicManager::GetSingleton()->OnRelicThrow(weaponToCall->projectile, weaponToCall);
                spdlog::info("{} has successfuly called {}", parent->GetActor()->GetName(), weaponToCall->weap->fullName);
            } else {
                spdlog::info("{} has failed to call {}", parent->GetActor()->GetName(), weaponToCall->weap->fullName);
            }
        } else {
            spdlog::info("no callable weapon found!");
        }
    }
}
RelicWeapon* Caller::GetCallableRelic()
{
    _weaponToCall = nullptr;
    if (!parent) return nullptr;
    if (!parent->GetRightHandRelic()) {
        if (auto lastRelic = parent->GetLastRightHandRelic();
            lastRelic && !lastRelic->IsEquipped() &&
            lastRelic->GetOwner() &&
            lastRelic->GetOwner() == parent &&
            lastRelic->HasAbility(RelicAbility::kCallable))
        {
            _weaponToCall = lastRelic;
            spdlog::debug("{} is the callable weapon of {}.", lastRelic->weap->GetFullName(), parent->GetActor()->GetName());
        } else if (const auto& knownRelics = parent->GetKnownRelics(); !knownRelics.empty()) {
            for (const auto& relicID : knownRelics) {
                auto relic = RelicManager::GetSingleton()->GetActiveRelic(relicID);
                spdlog::debug("0");
                if (!relic || !relic->HasAbility(RelicAbility::kCallable)) continue;
                spdlog::debug("1");
                auto owner = relic->GetOwner();
                if (owner && owner == parent) {
                    if (const auto thrownRelic = relic->As<ThrowableRelicWeapon>(); !relic->GetCarrier() && thrownRelic->projectile) {
                        spdlog::debug("2");
                        _weaponToCall = relic;
                        break;
                    } else if (auto wielder = relic->GetWielder()) {
                        if (wielder == parent) {
                            spdlog::error("WEIRD! Caller {} already equipping {}. This should never happen.", parent->GetActor()->GetName(), relic->weap->GetFullName());
                            continue;
                        } else if (alterationLevel > wielder->meleeSkill * 2.f) {
                            spdlog::debug("3");
                            _weaponToCall = relic;
                            break;
                        }
                    } else {
                        spdlog::error("WEIRD! {} is not owned by {}, but it's equipped. This should never happen.", relic->weap->GetFullName(), parent->GetActor()->GetName());
                    }
                } else if (auto wielder = relic->GetWielder()) {
                    if (wielder == parent) {
                        spdlog::error("WEIRD! Caller {} already equipping {}. This should never happen.", parent->GetActor()->GetName(), relic->weap->GetFullName());
                        continue;
                    } else if (alterationLevel > wielder->meleeSkill * 3.f) {
                        spdlog::debug("4");
                        _weaponToCall = relic;
                        break;
                    }
                } else if (const auto thrownRelic = relic->As<ThrowableRelicWeapon>(); !relic->GetCarrier() && thrownRelic->projectile) {
                    spdlog::debug("5");
                    _weaponToCall = relic;
                    break;
                } else {
                    spdlog::debug("6");
                    _weaponToCall = relic;
                    continue;   //  try to find better options
                }
            }
            spdlog::error("{} is the callable weapon of {}.", _weaponToCall ? _weaponToCall->weap->GetFullName() : "null", parent->GetActor()->GetName());
        } else {
            spdlog::info("you don't have any relic weapon to call.");
        }
    } else {
        spdlog::info("you already have a relic weapon equipped.");
    }
    return _weaponToCall;
}

void Caller::OnAttach(BaseActor* a_actor)
{
    ActorCapability::OnAttach(a_actor);
    if (parent && parent->IsValid()) {
        if (auto callerAVO = parent->GetActor() ? parent->GetActor()->AsActorValueOwner() : nullptr) {
            alterationLevel = callerAVO->GetActorValue(RE::ActorValue::kAlteration);
        }
    }
}

void Caller::HandleAction(const ActionType a_action)
{
    switch (a_action)
    {
    case ActionType::kRage:
        break;
    case ActionType::kWeaponCharge:
        break;
    case ActionType::kSpecialIdle:
        break;
    case ActionType::kWeaponCall:
        if (!parent->IsInRage() && !parent->GetRightHandRelic()) {
            if (GetCallableRelic()) {
                parent->GetActor()->SetGraphVariableBool("bLeviInCatchRange", false);
                parent->GetActor()->SetGraphVariableInt("iKratosActionType", (uint8_t)ActionType::kWeaponCharge);   //  intentional
                parent->GetActor()->NotifyAnimationGraph("DoKratosAction");
            }
        } else if (parent->IsInRage()) {
            //  todo
        }
        break;

    default:
        break;
    }
}
