#pragma once

#include "RelicFactory.h"
#include "Weapons/RelicWeapon.h"
#include "Actors/Thrower.h"
#include "Actors/Kratos.h"

class RelicManager {
public:
    static RelicManager* GetSingleton() {
        static RelicManager singleton;
        return &singleton;
    }

    using RelicIdentity = std::uint64_t;

    static constexpr RelicIdentity MakeRelicIdentity(RE::FormID a_baseFormID, std::uint16_t a_uniqueID)
    {
        return (static_cast<RelicIdentity>(a_baseFormID) << 16) | a_uniqueID;
    }
    RelicIdentity GetOrCreateEquippedRelicIdentity(RE::Actor* a_actor)
    {
        RE::FormID formID = 0u;
        uint16_t uniqueID = 0u;
        auto ieData = a_actor->GetEquippedEntryData(false);

         if (ieData && ieData->object) {
            formID = ieData->object->formID;
        }

        auto xLists = ieData ? ieData->extraLists : nullptr;
        auto xData = xLists && !xLists->empty() ? xLists->front() : nullptr;
        auto xUniqueID = xData ? xData->GetByType<RE::ExtraUniqueID>() : nullptr;
        if (xUniqueID) {
            uniqueID = xUniqueID->uniqueID;
            formID = formID ? formID : xUniqueID->baseID;
        } else if (xData) {
            auto xUniqueIDnew = RE::BSExtraData::Create<RE::ExtraUniqueID>();
            auto inventoryChanges = a_actor->GetInventoryChanges();
            uint16_t nextUniqueID = inventoryChanges ? inventoryChanges->GetNextUniqueID() : 0u;
            if (xUniqueIDnew && nextUniqueID) {
                xUniqueIDnew->baseID = formID;
                xUniqueIDnew->uniqueID = nextUniqueID;
                xData->Add(xUniqueIDnew);
                uniqueID = xUniqueIDnew->uniqueID;
            }
        }
        return MakeRelicIdentity(formID, uniqueID);
    }

    void OnEquip(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->GetWeaponState() != RE::WEAPON_STATE::kDrawn) return;
        auto rObject = a_actor->GetEquippedObject(false);
        auto lObject = a_actor->GetEquippedObject(true);
        auto rWeapon = rObject ? rObject->As<RE::TESObjectWEAP>() : nullptr;
        auto lWeapon = lObject ? lObject->As<RE::TESObjectWEAP>() : nullptr;

        if (rWeapon) {
            if (auto type = GetRelicType(rWeapon); type != RelicType::kNone) {

                BaseActor* activeActor = a_actor->IsPlayerRef() ? GetOrInitializePlayer() : GetOrCreateActor(a_actor->GetHandle());
                if (!activeActor) return;

                auto relicID = GetOrCreateEquippedRelicIdentity(a_actor);
                auto& activeRelic = activeRelics[relicID];
                if (activeRelic) {
                    activeRelic->OnEquip(activeActor);
                } else {
                    activeRelic = RelicFactory::Create(type, rWeapon);
                    if (activeRelic) {
                        activeRelic->relicIdentity = relicID;
                        activeRelic->OnEquip(activeActor);
                    } else{
                        activeRelics.erase(relicID);
                    }
                }
            }
        }
        if (lWeapon) {
            // sadece blades of chaos için
        }
    }
    bool OnHit(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
    {
        bool skip = false;
        if (auto it = throwableRelics.find(a_this); it != throwableRelics.end() && it->second) {
            skip = it->second->OnHit(a_AllCdPointCollector);
        }
        return skip;
    }
    void OnImpact(RE::Projectile::ImpactData* a_impactData, RE::ArrowProjectile* a_this, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
    {
        if (auto it = throwableRelics.find(a_this); it != throwableRelics.end() && it->second) {
            it->second->OnImpact(a_impactData, a_target, a_targetLoc, a_velocity, a_collidable);
        }
    }
    void OnMenuOpenCloseEvent(const bool a_opening) {
        for (const auto& [id, relic] : activeRelics) {
            if (relic)
                relic->OnMenuOpenCloseEvent(a_opening);
        }
    }

    void UpdateRelic(RE::Projectile* a_this)
    {
        if (a_this) {
            if (auto it = throwableRelics.find(a_this); it != throwableRelics.end() && it->second) {
                it->second->Update();
            }
        }
    }

    void UpdatePlayer(RE::PlayerCharacter* a_player, float a_delta) {
        if (GetOrInitializePlayer()) {
            player->Update(a_delta);
        }
    }

    void UpdateNPC(RE::Actor* a_this, float a_delta) {
        if (a_this && !a_this->IsPlayerRef()) {
            if (auto it = activeActors.find(a_this->GetHandle()); it != activeActors.end() && it->second) {
                it->second->Update(a_delta);
            }
        }
    }

private:
    std::unique_ptr<BaseActor> playerPtr;
    BaseActor* player = nullptr;    //  performans için, her seferinde get() yapmamak için
    std::unordered_map<RE::ActorHandle, std::unique_ptr<BaseActor>> activeActors;

    std::unordered_map<RelicIdentity, std::unique_ptr<RelicWeapon>> activeRelics;
    std::unordered_map<RE::Projectile*, RelicWeapon*> throwableRelics;

    RelicType GetRelicType(const RE::TESObjectWEAP* a_weap) const { /* keywordlardan anlaşılacak */ return RelicType::kNone; }

    BaseActor* GetOrInitializePlayer() {
        if (!player) {
            playerPtr = std::make_unique<Kratos>(RE::PlayerCharacter::GetSingleton()->GetHandle());
            if (playerPtr->IsValid()) {
                player = playerPtr.get();
            } else {
                playerPtr.reset();
            }
        }
        return player;
    }
    BaseActor* GetOrCreateActor(RE::ActorHandle a_actorHandle) {
        auto it = activeActors.find(a_actorHandle);
        if (it != activeActors.end()) return it->second.get();

        // İstersen burada aktörün skill seviyesine göre (Örn: Rager, Caller) polimorfik olarak aktör yaratabilirsin.
        activeActors[a_actorHandle] = std::make_unique<Kratos>(a_actorHandle);
        return activeActors[a_actorHandle].get();
    }
};
