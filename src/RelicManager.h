#pragma once

#include "RelicFactory.h"
#include "Weapons/RelicWeapon.h"
#include "Actors/Kratos.h"

class RelicManager {
public:
    static RelicManager* GetSingleton() {
        static RelicManager singleton;
        return &singleton;
    }

    using RelicIdentity = std::uint64_t;
    using BaseActorPtr = std::unique_ptr<BaseActor>;
    using KratosPtr = std::unique_ptr<Kratos>;
    using RelicWeaponPtr = std::unique_ptr<RelicWeapon>;

    static constexpr std::uint32_t kRecordRelics = 'RELC';
    static constexpr std::uint32_t kRecordActors = 'ACTR';

    static constexpr std::uint32_t kRelicsVersion = 1;
    static constexpr std::uint32_t kActorsVersion = 1;

    static constexpr std::uint32_t kMinRelicsVersion = 1;
    static constexpr std::uint32_t kMinActorsVersion = 1;

    struct SavedRelicKey
    {
        RE::FormID baseFormID = 0;
        std::uint16_t uniqueID = 0;

        bool operator==(const SavedRelicKey& a_rhs) const
        {
            return baseFormID == a_rhs.baseFormID && uniqueID == a_rhs.uniqueID;
        }
    };
    struct SavedRelic
    {
        SavedRelicKey key;
        std::uint8_t type = (std::uint8_t)RelicType::kNone;
    };
    struct SavedActor
    {
        RE::FormID formID  = 0x0;

        std::uint32_t titles = 0u;

        float level = 0.f;
        float meleeSkill = 0.f;
        float damageMult = 1.f;

        std::vector<SavedRelicKey> knownRelicKeys;

        SavedRelicKey rightHandRelic;
        SavedRelicKey leftHandRelic;
        SavedRelicKey lastRightHandRelic;
        SavedRelicKey lastLeftHandRelic;
    };

    static constexpr RelicIdentity MakeRelicIdentity(RE::FormID a_baseFormID, std::uint16_t a_uniqueID)
    {
        return (static_cast<RelicIdentity>(a_baseFormID) << 16) | a_uniqueID;
    }
    static constexpr std::uint16_t GetRelicUniqueID(const RelicWeapon& a_relic)
    {
        return static_cast<std::uint16_t>(a_relic.relicIdentity & 0xFFFFu);
    }
    static constexpr RE::FormID GetRelicBaseFormID(const RelicWeapon& a_relic)
    {
        return static_cast<RE::FormID>(a_relic.relicIdentity >> 16);
    }

    std::optional<RelicIdentity> GetOrCreateEquippedRelicIdentity(RE::Actor* a_actor, const bool a_isLeft = false)
    {
        if (!a_actor) return std::nullopt;

        std::optional<RelicIdentity> result;
        RE::FormID formID = 0x0;
        uint16_t uniqueID = 0x0;
        auto ieData = a_actor->GetEquippedEntryData(a_isLeft);

        if (ieData && ieData->object) {
            formID = ieData->object->formID;
        }

        auto xLists = ieData ? ieData->extraLists : nullptr;
        auto xData = xLists && !xLists->empty() ? xLists->front() : nullptr;
        auto xUniqueID = xData ? xData->GetByType<RE::ExtraUniqueID>() : nullptr;
        if (xUniqueID) {
            uniqueID = xUniqueID->uniqueID;
            formID = formID != 0x0 ? formID : xUniqueID->baseID;
        } else if (xData) {
            auto xUniqueIDnew = RE::BSExtraData::Create<RE::ExtraUniqueID>();
            auto inventoryChanges = a_actor->GetInventoryChanges();
            uint16_t nextUniqueID = inventoryChanges ? inventoryChanges->GetNextUniqueID() : 0x0;
            if (xUniqueIDnew && nextUniqueID) {
                xUniqueIDnew->baseID = formID;
                xUniqueIDnew->uniqueID = nextUniqueID;
                xData->Add(xUniqueIDnew);
                uniqueID = nextUniqueID;
            } else {
                delete xUniqueIDnew;
            }
        }

        if (formID == 0x0 || uniqueID == 0x0) result = std::nullopt;
        else result = std::make_optional<RelicIdentity>(MakeRelicIdentity(formID, uniqueID));

        return result;
    }
    SavedRelicKey MakeSavedRelicKey(const RelicWeapon& a_relic)
    {
        return {GetRelicBaseFormID(a_relic), GetRelicUniqueID(a_relic)};
    }

    [[nodiscard]] RelicWeapon* GetActiveRelic(RelicIdentity a_identity)
    {
        auto it = activeRelics.find(a_identity);
        return it != activeRelics.end() ? it->second.get() : nullptr;
    }

    void OnRevert()
    {
        ClearRuntimeState();
        ClearSavedState();
        spdlog::debug("states reverted");
    }
    void OnPreLoadGame()
    {
        if (GetOrInitializePlayer()) {
            player->GetActor()->SetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);
            player->GetActor()->SetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);
            player->GetActor()->SetGraphVariableBool("Skip3DLoading", _skipLoad3D);
        }

        for (auto& relic : activeRelics) {
            if (relic.second) {
                relic.second->ResetCharge(relic.second->enchMag, relic.second->defaultEnchMag, false, true);
            }
        }

        for (auto& activeActor : activeActors) {
            if (auto actor = activeActor.first.get().get()) {
                actor->SetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);
                actor->SetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);
                actor->SetGraphVariableBool("Skip3DLoading", _skipLoad3D);
            }
        }
    }
    void OnPostLoadGame(SKSE::SerializationInterface* a_intfc = nullptr)
    {
        if (!a_intfc) return;

        ClearRuntimeState();
        ClearSavedState();

        std::uint32_t type = 0;
        std::uint32_t version = 0;
        std::uint32_t length = 0;

        while (a_intfc->GetNextRecordInfo(type, version, length)) {
            if (type == kRecordRelics) {
                ReadRelicsRecord(a_intfc, version);
            } else if (type == kRecordActors) {
                ReadActorsRecord(a_intfc, version);
            }
        }

        restorePending = !savedRelics.empty() || !savedActors.empty();
        RestoreSavedState(a_intfc);
    }
    void OnSaveGame(SKSE::SerializationInterface* a_intfc = nullptr)
    {
        for (auto& relic : activeRelics) {
            if (relic.second)
                relic.second->ResetCharge(relic.second->enchMag, relic.second->defaultEnchMag, false, true);
        }

        if (GetOrInitializePlayer()) {
            player->GetActor()->SetGraphVariableBool("SkipEquipAnimation", _skipEquipAnim);
            player->GetActor()->SetGraphVariableInt("LoadBoundObjectDelay", _load3Ddelay);
            player->GetActor()->SetGraphVariableBool("Skip3DLoading", _skipLoad3D);
        }

        if (!a_intfc) return;

        spdlog::debug("saving (relics v{}, actors v{})...",
                      kRelicsVersion, kActorsVersion);

        //  ── RELICS ───────────────────────────────────────────────
        if (!a_intfc->OpenRecord(kRecordRelics, kRelicsVersion)) {
            spdlog::error("couldn't open record for relics.");
            return;
        }
        switch (kRelicsVersion) {
        case 1: WriteRelicsV1(a_intfc); break;
        //  case 2: WriteRelicsV2(a_intfc); break;
        default: break;
        }

        //  ── ACTORS ───────────────────────────────────────────────
        if (!a_intfc->OpenRecord(kRecordActors, kActorsVersion)) {
            spdlog::error("couldn't open record for actors.");
            return;
        }
        switch (kActorsVersion) {
        case 1: WriteActorsV1(a_intfc); break;
        //  case 2: WriteActorsV2(a_intfc); break;
        default: break;
        }

        spdlog::debug("save success!");
    }
    void OnConfigClose()
    {
        if (auto player = RE::PlayerCharacter::GetSingleton(); player && GetOrInitializePlayer() && Config::SpecialWeapon) {
            const auto mcmSelectedRelic = (RelicType)(uint8_t)Config::SpecialWeapon->value;
            const auto rObject = player->GetEquippedObject(false);
            const auto rWeapon = rObject ? rObject->As<RE::TESObjectWEAP>() : nullptr;
            RelicWeapon* relic = nullptr;
            if (rWeapon) {
                const auto identity = GetOrCreateEquippedRelicIdentity(player).value_or(0x0);
                if (identity == 0x0) return;

                auto it = activeRelics.find(identity);

                if (mcmSelectedRelic == RelicType::kNone) {
                    if (it != activeRelics.end()) {
                        if (auto rRelic = it->second ? it->second.get() : nullptr) {
                            if (auto thrownWeapon = rRelic ? dynamic_cast<ThrowableRelicWeapon*>(rRelic) : nullptr)
                                thrownRelics.erase(thrownWeapon->projectile);

                            if (rRelic && rRelic->GetWielder())
                                rRelic->GetWielder()->SetRightHandRelic(nullptr);
                        }

                        activeRelics.erase(it);
                    }
                    return;
                }

                if (it != activeRelics.end()) {
                    if (it->second->GetType() == mcmSelectedRelic) {
                        return;
                    }
                    if (auto thrownWeapon = it->second ? dynamic_cast<ThrowableRelicWeapon*>(it->second.get()) : nullptr)
                        thrownRelics.erase(thrownWeapon->projectile);

                    activeRelics.erase(it);
                }

                auto relicPtr = RelicFactory::Create(mcmSelectedRelic, rWeapon);
                if (relicPtr) {
                    relicPtr->relicIdentity = identity;
                    relicPtr->OnEquip(GetPlayer());
                    relic = relicPtr.get();
                }

                activeRelics.emplace(identity, std::move(relicPtr));
            }
            GetPlayer()->OnEquip(relic);
        }
    }

    void OnEquip(RE::Actor* a_actor)
    {
        if (!a_actor || !Config::SpecialWeapon) return;

        uint8_t relicType = (uint8_t)RelicType::kNone;

        if (a_actor->IsPlayer())
            Config::SpecialWeapon->value = relicType;

        auto rObject = a_actor->GetEquippedObject(false);
        auto lObject = a_actor->GetEquippedObject(true);
        auto rWeapon = rObject ? rObject->As<RE::TESObjectWEAP>() : nullptr;
        auto lWeapon = lObject ? lObject->As<RE::TESObjectWEAP>() : nullptr;

        BaseActor* activeActor = a_actor->IsPlayerRef() ? GetOrInitializePlayer() : GetOrCreateActor(a_actor->GetHandle());
        if (!activeActor) {
            spdlog::error("failed to get or initialize {}", a_actor->GetName());
            return;
        }

        RelicWeapon* activeRelic = nullptr;

        if (rWeapon) {
            auto relicID = GetOrCreateEquippedRelicIdentity(a_actor).value_or(0x0);
            if (relicID == 0x0) return;

            auto it = activeRelics.find(relicID);

            activeRelic = it != activeRelics.end() && it->second ? it->second.get() : nullptr;

            activeActor->SetRightHandRelic(activeRelic);

            if (activeRelic) {
                activeRelic->OnEquip(activeActor);
                spdlog::debug("relic equipped");

                relicType = (uint8_t)activeRelic->GetType();

                auto& knownRelics = activeActor->GetKnownRelics();
                if (std::find(knownRelics.rbegin(), knownRelics.rend(), relicID) == knownRelics.rend()) {
                    knownRelics.emplace_back(relicID);
                }
                if (auto throwableRelic = dynamic_cast<ThrowableRelicWeapon*>(activeRelic)) {
                    if (throwableRelic->projectile) {
                        thrownRelics.erase(throwableRelic->projectile);
                    }
                }
            }
        }
        if (lWeapon) {
            // sadece blades of chaos için
        }

        if (a_actor->IsPlayer())
            Config::SpecialWeapon->value = relicType;

        activeActor->OnEquip(activeRelic);
    }
    bool OnHit(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)
    {
        bool skip = false;
        if (auto it = thrownRelics.find(a_this); it != thrownRelics.end() && it->second) {
            skip = it->second->OnHit(a_AllCdPointCollector);
        }
        return skip;
    }
    void OnImpact(RE::Projectile::ImpactData* a_impactData, RE::ArrowProjectile* a_this, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable)
    {
        if (auto it = thrownRelics.find(a_this); it != thrownRelics.end() && it->second) {
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
        thrownRelics.emplace(a_projectile, a_relic);
    }

    Kratos* GetPlayer() const {return player;}
    void UpdateRelic(RE::Projectile* a_this)
    {
        if (a_this) {
            if (auto it = thrownRelics.find(a_this); it != thrownRelics.end() && it->second) {
                it->second->UpdateProjectile(a_this);
            }
        }
    }
    void UpdatePlayer(RE::PlayerCharacter* a_player, float a_delta)
    {

        if (GetOrInitializePlayer()) {
            player->Update(a_delta);
        }

        for (auto& relic : activeRelics) {
            if (relic.second)
                relic.second->Update();
        }
    }
    void UpdateNPC(RE::Actor* a_this, float a_delta)
    {
        if (a_this && !a_this->IsPlayerRef()) {
            if (auto it = activeActors.find(a_this->GetHandle()); it != activeActors.end() && it->second) {
                it->second->Update(a_delta);
            }
        }
    }

private:
    KratosPtr playerPtr;
    Kratos* player = nullptr;
    std::unordered_map<RE::ActorHandle, BaseActorPtr> activeActors;

    std::unordered_map<RelicIdentity, RelicWeaponPtr> activeRelics;
    std::unordered_map<RE::Projectile*, ThrowableRelicWeapon*> thrownRelics;

    std::vector<SavedRelic> savedRelics;
    std::vector<SavedActor> savedActors;

    bool restorePending = false;

    RelicWeapon* FindRuntimeRelic(const SavedRelicKey& a_key, SKSE::SerializationInterface* a_serialization)
    {
        if (!a_serialization || a_key.baseFormID == 0 || a_key.uniqueID == 0) {
            return nullptr;
        }

        RE::FormID resolvedFormID = 0;

        if (!a_serialization->ResolveFormID(a_key.baseFormID, resolvedFormID)) {
            return nullptr;
        }

        const auto identity = MakeRelicIdentity(resolvedFormID, a_key.uniqueID);

        auto it = activeRelics.find(identity);

        return it != activeRelics.end() && it->second ? it->second.get() : nullptr;
    }
    bool RelinkActorsToRelics(SKSE::SerializationInterface* a_serialization)
    {
        if (!a_serialization) {
            return false;
        }

        bool unresolved = false;

        for (const auto& saved : savedActors) {

            RE::FormID resolvedActorFormID = 0;
            if (saved.formID == 0x0) {
                unresolved = true;
                continue;
            }

            if (!a_serialization->ResolveFormID(saved.formID, resolvedActorFormID)) {
                unresolved = true;
                continue;
            }

            auto form = RE::TESForm::LookupByID(resolvedActorFormID);
            auto actor = form ? form->As<RE::Actor>() : nullptr;

            if (!actor) {
                unresolved = true;
                continue;
            }

            BaseActor* baseActor = actor->IsPlayerRef()
                ? GetOrInitializePlayer()
                : GetOrCreateActor(actor->GetHandle());

            if (!baseActor) {
                unresolved = true;
                continue;
            }

            auto& knownRelics = baseActor->GetKnownRelics();
            knownRelics.clear();

            for (const auto& key : saved.knownRelicKeys) {
                if (key.baseFormID == 0 || key.uniqueID == 0) {
                    unresolved = true;
                    continue;
                }

                RE::FormID resolvedFormID = 0;

                if (!a_serialization->ResolveFormID(key.baseFormID, resolvedFormID)) {
                    unresolved = true;
                    continue;
                }

                const auto identity = MakeRelicIdentity(resolvedFormID, key.uniqueID);

                if (activeRelics.contains(identity)) {
                    knownRelics.push_back(identity);
                } else {
                    unresolved = true;
                }
            }

            auto resolveOptional = [&](const SavedRelicKey& key) -> RelicWeapon* {
                if (key.baseFormID == 0 || key.uniqueID == 0) {
                    return nullptr;
                }

                auto relic = FindRuntimeRelic(key, a_serialization);

                if (!relic) {
                    unresolved = true;
                }

                return relic;
            };

            baseActor->SetRightHandRelic(resolveOptional(saved.rightHandRelic));
            baseActor->SetLeftHandRelic(resolveOptional(saved.leftHandRelic));
            baseActor->SetLastRightHandRelic(resolveOptional(saved.lastRightHandRelic));
            baseActor->SetLastLeftHandRelic(resolveOptional(saved.lastLeftHandRelic));

            baseActor->OnEquip(baseActor->GetRightHandRelic());
            if (baseActor->GetRightHandRelic())
                baseActor->GetRightHandRelic()->OnEquip(baseActor);
        }
        return !unresolved;
    }
    bool RestoreRelics(SKSE::SerializationInterface* a_serialization)
    {
        if (!a_serialization) {
            return false;
        }

        bool unresolved = false;

        for (const auto& saved : savedRelics) {

            if (saved.key.baseFormID == 0 || saved.key.uniqueID == 0) {
                continue;
            }

            RE::FormID resolvedFormID = 0;

            if (!a_serialization->ResolveFormID(saved.key.baseFormID, resolvedFormID))
            {
                unresolved = true;
                continue;
            }

            auto* form = RE::TESForm::LookupByID(resolvedFormID);
            auto* weapon = form ? form->As<RE::TESObjectWEAP>() : nullptr;

            if (!weapon) {
                unresolved = true;
                continue;
            }

            const auto identity = MakeRelicIdentity(resolvedFormID, saved.key.uniqueID);

            if (activeRelics.contains(identity)) {
                auto& relic = activeRelics.at(identity);

                if (relic && (std::uint8_t)relic->GetType() != saved.type) {
                    spdlog::error(
                        "Relic type mismatch for identity {:X}: runtime: {}, saved: {}",
                        identity,
                        static_cast<int>(relic->GetType()),
                        static_cast<int>(saved.type));

                    unresolved = true;
                }
                continue;
            }

            auto relic = RelicFactory::Create((RelicType)saved.type, weapon);

            if (!relic) {
                unresolved = true;
                continue;
            }

            relic->relicIdentity = identity;
            relic->currentReference = nullptr;  //  actually not necessary, maybe I will delete it from the class

            activeRelics.emplace(identity, std::move(relic));
        }

        return !unresolved;
    }
    bool RestoreActors(SKSE::SerializationInterface* a_serialization)
    {
        if (!a_serialization) {
            return false;
        }

        bool unresolved = false;

        for (const auto& saved : savedActors) {

            RE::FormID resolvedFormID = 0;

            if (!a_serialization->ResolveFormID(saved.formID, resolvedFormID)) {
                unresolved = true;
                continue;
            }

            auto form = RE::TESForm::LookupByID(resolvedFormID);
            auto actor = form ? form->As<RE::Actor>() : nullptr;

            if (!actor) {
                unresolved = true;
                continue;
            }

            BaseActor* baseActor = actor->IsPlayerRef()
                ? GetOrInitializePlayer()
                : GetOrCreateActor(actor->GetHandle());

            if (!baseActor) {
                unresolved = true;
                continue;
            }

            baseActor->level = saved.level;
            baseActor->meleeSkill = saved.meleeSkill;
            baseActor->damageMult = saved.damageMult;
            baseActor->SetTitles(saved.titles);
        }
        return !unresolved;
    }
    void RestoreSavedState(SKSE::SerializationInterface* a_serialization)
    {
        if (!restorePending || !a_serialization) {
            return;
        }
        spdlog::info("loading saved state...");

        bool pending = false;

        pending |= !RestoreRelics(a_serialization);
        pending |= !RestoreActors(a_serialization);
        pending |= !RelinkActorsToRelics(a_serialization);

        restorePending = pending;
        if (restorePending)
            spdlog::warn("save is not loaded successfully!");
        else 
            spdlog::info("save loaded successfully!");
    }

    void ClearRuntimeState()
    {
        activeRelics.clear();
        activeActors.clear();

        playerPtr.reset();
        player = nullptr;

        thrownRelics.clear();

        if (Config::SpecialWeapon) Config::SpecialWeapon->value = (uint8_t)RelicType::kNone;
    }
    void ClearSavedState()
    {
        savedRelics.clear();
        savedActors.clear();
        restorePending = false;
    }
    static bool ReadRelicKey(SKSE::SerializationInterface* a_intfc, SavedRelicKey& a_key)
    {
        if (!a_intfc) {
            return false;
        }
        return
            a_intfc->ReadRecordData(a_key.baseFormID) &&
            a_intfc->ReadRecordData(a_key.uniqueID);
    }

    void ReadRelicsV1(SKSE::SerializationInterface* a_intfc)
    {
        std::uint32_t count = 0;
        if (!a_intfc->ReadRecordData(count) || count > 65536) {
            ClearSavedState();
            return;
        }

        savedRelics.clear();
        savedRelics.reserve(count);

        for (std::uint32_t i = 0; i < count; i++) {
            SavedRelic saved;
            if (!ReadRelicKey(a_intfc, saved.key) ||
                !a_intfc->ReadRecordData(saved.type)) {
                ClearSavedState();
                return;
            }
            if (saved.key.baseFormID == 0 || saved.key.uniqueID == 0) continue;
            savedRelics.push_back(std::move(saved));
        }
    }
    void ReadActorsV1(SKSE::SerializationInterface* a_intfc)
    {
        std::uint32_t count = 0;
        if (!a_intfc->ReadRecordData(count) || count > 65536) {
            ClearSavedState();
            return;
        }

        savedActors.reserve(count);

        for (std::uint32_t i = 0; i < count; i++) {
            SavedActor saved;

            if (!a_intfc->ReadRecordData(saved.formID) ||
                !a_intfc->ReadRecordData(saved.titles) ||
                !a_intfc->ReadRecordData(saved.level) ||
                !a_intfc->ReadRecordData(saved.meleeSkill) ||
                !a_intfc->ReadRecordData(saved.damageMult))
            {
                ClearSavedState();
                return;
            }

            std::uint32_t knownCount = 0;
            if (!a_intfc->ReadRecordData(knownCount) || knownCount > 65536) {
                ClearSavedState();
                return;
            }
            saved.knownRelicKeys.reserve(knownCount);

            for (std::uint32_t j = 0; j < knownCount; j++) {
                SavedRelicKey key;
                if (!ReadRelicKey(a_intfc, key)) {
                    ClearSavedState();
                    return;
                }
                saved.knownRelicKeys.push_back(key);
            }

            auto readOptionalRelic = [&](SavedRelicKey& a_key) -> bool {
                bool valid = false;
                if (!a_intfc->ReadRecordData(valid)) return false;
                if (!valid) { a_key = {}; return true; }
                return ReadRelicKey(a_intfc, a_key);
            };

            if (!readOptionalRelic(saved.rightHandRelic) ||
                !readOptionalRelic(saved.leftHandRelic) ||
                !readOptionalRelic(saved.lastRightHandRelic) ||
                !readOptionalRelic(saved.lastLeftHandRelic))
            {
                ClearSavedState();
                return;
            }

            if (saved.formID != 0x0) {
                savedActors.push_back(std::move(saved));
            }
        }
    }

    void WriteRelicsV1(SKSE::SerializationInterface* a_intfc)
    {
        std::uint32_t relicCount = 0;
        for (const auto& [identity, relic] : activeRelics) {
            if (relic) relicCount++;
        }

        if (!a_intfc->WriteRecordData(relicCount)) return;

        for (const auto& [identity, relic] : activeRelics) {
            if (!relic) continue;

            const auto key = MakeSavedRelicKey(*relic);
            if (!a_intfc->WriteRecordData(key.baseFormID) ||
                !a_intfc->WriteRecordData(key.uniqueID) ||
                !a_intfc->WriteRecordData((std::uint8_t)relic->GetType()))
            {
                return;
            }
        }
    }
    void WriteActorsV1(SKSE::SerializationInterface* a_intfc)
    {
        auto isValid = [](BaseActor* a) {
            return a && a->IsValid();
        };

        BaseActor* playerActor = GetOrInitializePlayer();
        const bool hasPlayer = isValid(playerActor);

        std::uint32_t actorCount = playerActor ? 1u : 0u;
        for (const auto& [handle, actor] : activeActors) {
            if (isValid(actor.get())) actorCount++;
        }

        if (!a_intfc->WriteRecordData(actorCount)) return;

        auto writeActor = [&](BaseActor* actor) -> bool {
            if (!isValid(actor)) return false;

            auto gameActor = actor->GetActor();
            const RE::FormID formID = gameActor->formID;

            if (!a_intfc->WriteRecordData(formID)) return false;

            const auto titles = actor->GetTitles().underlying();
            if (!a_intfc->WriteRecordData(titles) ||
                !a_intfc->WriteRecordData(actor->level) ||
                !a_intfc->WriteRecordData(actor->meleeSkill) ||
                !a_intfc->WriteRecordData(actor->damageMult))
            {
                return false;
            }

            const auto& knownRelics = actor->GetKnownRelics();
            std::vector<SavedRelicKey> knownRelicKeys;
            knownRelicKeys.reserve(knownRelics.size());

            for (const RelicIdentity identity : knownRelics) {
                if (identity == 0) continue;
                SavedRelicKey key;
                key.baseFormID = static_cast<RE::FormID>(identity >> 16);
                key.uniqueID   = static_cast<std::uint16_t>(identity & 0xFFFFu);
                if (key.baseFormID == 0 || key.uniqueID == 0) continue;
                knownRelicKeys.push_back(key);
            }

            const std::uint32_t knownCount =
                static_cast<std::uint32_t>(knownRelicKeys.size());
            if (!a_intfc->WriteRecordData(knownCount)) return false;

            for (const auto& key : knownRelicKeys) {
                if (!a_intfc->WriteRecordData(key.baseFormID) ||
                    !a_intfc->WriteRecordData(key.uniqueID))
                {
                    return false;
                }
            }

            auto writeRelicKey = [&](RelicWeapon* a_relic) -> bool {
                const bool valid = a_relic != nullptr;
                if (!a_intfc->WriteRecordData(valid)) return false;
                if (!valid) return true;
                const auto key = MakeSavedRelicKey(*a_relic);
                return a_intfc->WriteRecordData(key.baseFormID) &&
                       a_intfc->WriteRecordData(key.uniqueID);
            };

            if (!writeRelicKey(actor->GetRightHandRelic()) ||
                !writeRelicKey(actor->GetLeftHandRelic()) ||
                !writeRelicKey(actor->GetLastRightHandRelic()) ||
                !writeRelicKey(actor->GetLastLeftHandRelic()))
            {
                return false;
            }
            return true;
        };
            if (hasPlayer && !writeActor(playerActor)) return;

            for (const auto& [handle, actor] : activeActors) {
                if (!isValid(actor.get())) continue;
                if (!writeActor(actor.get())) return;
            }
    }

    void ReadRelicsRecord(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version)
    {
        if (a_version < kMinRelicsVersion) {
            spdlog::warn("relics record v{} < min v{}, skipping",
                         a_version, kMinRelicsVersion);
            return;   //  SKSE zaten kaydın sonuna atlar
        }
        if (a_version > kRelicsVersion) {
            spdlog::warn("relics record v{} is newer than this build (v{}), skipping",
                         a_version, kRelicsVersion);
            return;
        }

        switch (a_version) {
        case 1: ReadRelicsV1(a_intfc); break;

        default:
            spdlog::warn("unknown relics version {}, skipping", a_version);
            break;
        }
    }
    void ReadActorsRecord(SKSE::SerializationInterface* a_intfc, std::uint32_t a_version)
    {
        if (a_version < kMinActorsVersion) {
            spdlog::warn("actors record v{} < min v{}, skipping",
                         a_version, kMinActorsVersion);
            return;
        }
        if (a_version > kActorsVersion) {
            spdlog::warn("actors record v{} is newer than this build (v{}), skipping",
                         a_version, kActorsVersion);
            return;
        }

        switch (a_version) {
        case 1: ReadActorsV1(a_intfc); break;

        default:
            spdlog::warn("unknown actors version {}, skipping", a_version);
            break;
        }
    }

    BaseActor* GetOrInitializePlayer()
    {
        if (!player) {
            auto playerCharacter = RE::PlayerCharacter::GetSingleton();
            if (playerCharacter && playerCharacter->IsHandleValid())
                playerPtr = std::make_unique<Kratos>(playerCharacter->GetHandle());
            else 
                player = nullptr;

            if (playerPtr && playerPtr->IsValid()) {
                player = playerPtr.get();
            } else {
                playerPtr.reset();
            }
        }
        return player;
    }
    BaseActor* GetOrCreateActor(RE::ActorHandle a_actorHandle)
    {
        auto [it, inserted] = activeActors.try_emplace(a_actorHandle, nullptr);
        if (inserted) {
            it->second = std::make_unique<Kratos>(a_actorHandle);
        }
        return it->second.get();
    }

    RelicManager()
    {
        activeActors.reserve(64);
        activeRelics.reserve(256);

        savedActors.reserve(64);
        savedRelics.reserve(256);

        thrownRelics.reserve(128);
    }
};
