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
        if (!a_actor) return;

        Config::SpecialWeapon->value = (uint8_t)RelicType::kNone;

        auto rObject = a_actor->GetEquippedObject(false);
        auto lObject = a_actor->GetEquippedObject(true);
        auto rWeapon = rObject ? rObject->As<RE::TESObjectWEAP>() : nullptr;
        auto lWeapon = lObject ? lObject->As<RE::TESObjectWEAP>() : nullptr;

        if (rWeapon) {
            if (auto type = GetRelicType(rWeapon); type != RelicType::kNone) {

                BaseActor* activeActor = a_actor->IsPlayerRef() ? GetOrInitializePlayer() : GetOrCreateActor(a_actor->GetHandle());
                if (!activeActor) {
                    spdlog::error("failed to get or initialize player");
                    return;
                }

                auto relicID = GetOrCreateEquippedRelicIdentity(a_actor);
                auto& activeRelic = activeRelics[relicID];
                if (activeRelic) {
                    activeRelic->OnEquip(activeActor);
                    spdlog::error("relic equipped, {}", typeid(type).name());
                } else {
                    activeRelic = RelicFactory::Create(type, rWeapon);
                    if (activeRelic) {
                        activeRelic->relicIdentity = relicID;
                        activeRelic->OnEquip(activeActor);
                        spdlog::debug("relic created, {}", typeid(type).name());
                    } else{
                        activeRelics.erase(relicID);
                        spdlog::error("failed to initialize relic weapon");
                    }
                }

                activeActor->SetRightHandRelic(activeRelic.get());

                if (activeRelic) {
                    Config::SpecialWeapon->value = (uint8_t)activeRelic->GetType();
                    auto& knownRelics = activeActor->GetKnownRelics();
                    if (std::find(knownRelics.rbegin(), knownRelics.rend(), activeRelic.get()) == knownRelics.rend()) {
                        knownRelics.emplace_back(activeRelic.get());
                    }
                }
            }
        }
        if (lWeapon) {
            // sadece blades of chaos için
        }
        a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
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
            it->second->PostImpact(a_impactData, a_target, a_targetLoc, a_velocity, a_collidable);
        }
    }
    void OnMenuOpenCloseEvent(const bool a_opening)
    {
        for (const auto& [id, relic] : activeRelics) {
            if (relic)
                relic->OnMenuOpenCloseEvent(a_opening);
        }
    }

    void OnRelicThrow(RE::Projectile* a_projectile, ThrowableRelicWeapon* a_relic)
    {
        throwableRelics.emplace(a_projectile, a_relic);
    }

    BaseActor* GetPlayer() const {return player;}
    void UpdateRelic(RE::Projectile* a_this)
    {
        if (a_this) {
            if (auto it = throwableRelics.find(a_this); it != throwableRelics.end() && it->second) {
                it->second->UpdateProjectile(a_this);
            }
        }
    }
    void UpdatePlayer(RE::PlayerCharacter* a_player, float a_delta) {
        if (GetOrInitializePlayer()) {
            player->Update(a_delta);
        }

        for (auto& relic : activeRelics) {
            if (relic.second)
                relic.second->Update();
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
    BaseActor* player = nullptr;
    std::unordered_map<RE::ActorHandle, std::unique_ptr<BaseActor>> activeActors;

    std::unordered_map<RelicIdentity, std::unique_ptr<RelicWeapon>> activeRelics;
    std::unordered_map<RE::Projectile*, ThrowableRelicWeapon*> throwableRelics;

    RelicType GetRelicType(const RE::TESObjectWEAP* a_weap) const {
        RelicType type = RelicType::kNone;
        if (a_weap->HasKeyword(Config::LeviathanAxeKWD)) {
            type = RelicType::kLeviathanAxe;
        } else if (a_weap->HasKeyword(Config::BladeOfChaosKWD)) {
            type = RelicType::kBladesOfChaos;
        } else if (a_weap->HasKeyword(Config::DraupnirSpearKWD)) {
            type = RelicType::kDraupnir;
        } else if (a_weap->HasKeyword(Config::MjolnirKWD)) {
            type = RelicType::kMjolnir;
        } else if (a_weap->HasKeyword(Config::BladeOfOlympusKWD)) {
            type = RelicType::kBladeOfOlympus;
        } else if (a_weap->HasKeyword(Config::TridentKWD)) {
            type = RelicType::kTrident;
        }
        return type;
    }

    BaseActor* GetOrInitializePlayer() {
        if (!player) {
            auto playerCharacter = RE::PlayerCharacter::GetSingleton();
            if (playerCharacter && playerCharacter->IsHandleValid())
                playerPtr = std::make_unique<Kratos>(playerCharacter->GetHandle());
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
