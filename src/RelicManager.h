#pragma once

#include "Weapons/RelicWeapon.h"
#include "Actors/Thrower.h"
#include "Actors/Kratos.h"

class RelicManager {
public:
    static RelicManager* GetSingleton() {
        static RelicManager singleton;
        return &singleton;
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

                RelicWeapon* relic = GetOrCreateRelic(type, rWeapon);
                BaseActor* activeActor = nullptr;
                if (a_actor->IsPlayerRef()) {
                    activeActor = GetOrInitializePlayer();
                } else {
                    activeActor = GetOrCreateActor(a_actor);
                }

                relic->OnEquip(activeActor);

                // Aktör sınıfının içine de silahı kaydet (BaseActor.h'ye set metodları eklenmeli)[cite: 7]
                // baseActor->SetRightHandRelic(relic);
            }
        }
        if (lWeapon) {
            // sadece blades of chaos için
        }
    }
    bool OnHit(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
    {
        bool skip = false;
        if (auto it = activeRelics.find(a_this); it != activeRelics.end() && it->second.get()) {
            skip = it->second->OnHit(a_AllCdPointCollector);
        }
        return skip;
    }
    void OnImpact(RE::Projectile::ImpactData* a_impactData, RE::ArrowProjectile* a_this, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
    {
        if (auto it = activeRelics.find(a_this); it != activeRelics.end() && it->second.get()) {
            it->second->OnImpact(a_impactData, a_target, a_targetLoc, a_velocity, a_collidable);
        }
    }
    void OnMenuOpenCloseEvent(const bool a_opening) {
        if (a_opening) {
            for (const auto& relic : activeRelics) {
                relic.second->OnMenuOpenCloseEvent(a_opening);
            }
        }
    }

    void UpdateRelic(RE::Projectile* a_this)
    {
        if (a_this) {
            if (auto it = activeRelics.find(a_this); it != activeRelics.end() && it->second) {
                it->second->Update();
            }
        }
    }

    void UpdatePlayer(RE::PlayerCharacter* a_player, float a_delta) {
        if (player) {
            player->Update(a_delta);
        }
    }

    void UpdateNPC(RE::Actor* a_this, float a_delta) {
        if (a_this && !a_this->IsPlayerRef()) {
            if (auto it = activeActors.find(a_this); it != activeActors.end() && it->second) {
                it->second->Update(a_delta);
            }
        }
    }

private:
    std::unique_ptr<BaseActor> player;
    std::unordered_map<RE::Actor*, std::unique_ptr<BaseActor>> activeActors;
    std::unordered_map<RE::Projectile*, std::unique_ptr<RelicWeapon>> activeRelics;

    RelicType GetRelicType(RE::TESObjectWEAP* a_weap) { /* keywordlardan anlaşılacak */ return RelicType::kNone; }

    RelicWeapon* GetOrCreateRelic(RelicType type, RE::TESObjectWEAP* weap) {
        // Eğer bu formID'ye sahip silah daha önce yaratılmadıysa yeni factory metoduyla oluştur.
        // Factory pattern'ı ile LeviathanAxe, BladeOfChaos vb. oluşturulup activeRelics'e atılır.
        return nullptr; 
    }

    BaseActor* GetOrInitializePlayer() {
        if (!player) {
            player = std::make_unique<Kratos>(RE::PlayerCharacter::GetSingleton()->GetHandle());
            return player && player->IsValid() ? player.get() : nullptr;
        } else return player.get();
    }
    BaseActor* GetOrCreateActor(RE::Actor* a_actor) {
        auto it = activeActors.find(a_actor);
        if (it != activeActors.end()) return it->second.get();

        // İstersen burada aktörün skill seviyesine göre (Örn: Rager, Caller) polimorfik olarak aktör yaratabilirsin.
        activeActors[a_actor] = std::make_unique<Kratos>(a_actor->GetHandle());
        return activeActors[a_actor].get();
    }
};