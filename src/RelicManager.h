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
    using BaseActorPtr = std::unique_ptr<BaseActor>;
    using RelicWeaponPtr = std::unique_ptr<RelicWeapon>;

    static constexpr std::uint32_t kRecordRelics = 'RELC';
    static constexpr std::uint32_t kRecordActors = 'ACTR';
    static constexpr std::uint32_t kSerializationVersion = 1;

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
        RelicType type = RelicType::kNone;
    };
    struct SavedActor
    {
        RE::FormID formID  = 0;

        std::uint32_t titles = 0;

        float level = 0.f;
        float meleeSkill = 0.f;
        float damageMult = 1.f;

        bool skipEquipAnim = false;
        bool unequipWhenAnimEnds = false;
        bool isBarehanded = false;

        std::vector<SavedRelicKey> knownRelics;

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

    std::optional<RelicIdentity> GetOrCreateEquippedRelicIdentity(RE::Actor* a_actor)
    {
        std::optional<RelicIdentity> result;
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

        if (formID == 0u || uniqueID == 0u) result = std::nullopt;
        else result = std::make_optional<RelicIdentity>(MakeRelicIdentity(formID, uniqueID));

        return result;
    }
    SavedRelicKey MakeSavedRelicKey(const RelicWeapon& a_relic)
    {
        return {GetRelicBaseFormID(a_relic), GetRelicUniqueID(a_relic)};
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
        for (auto& relic : activeRelics) {
            if (relic.second)
                relic.second->ResetCharge(relic.second->enchMag, relic.second->defaultEnchMag, false, true);
        }

        if (!a_intfc) {
            return;
        }

        std::uint32_t type = 0;
        std::uint32_t version = 0;
        std::uint32_t length = 0;

        ClearRuntimeState();
        ClearSavedState();

        while (a_intfc->GetNextRecordInfo(type, version, length)) {

            if (version != kSerializationVersion) {
                continue;
            }

            //
            // RELICS
            //

            if (type == kRecordRelics) {

                std::uint32_t count = 0;

                if (!a_intfc->ReadRecordData(count) || count > 65536) {
                    ClearSavedState();
                    return;
                }

                savedRelics.clear();
                savedRelics.reserve(count);

                for (std::uint32_t i = 0; i < count; i++) {

                    SavedRelic saved;

                    if (!ReadRelicKey(a_intfc, saved.key) || !a_intfc->ReadRecordData(saved.type)) {
                        ClearSavedState();
                        return;
                    }

                    if (saved.key.baseFormID == 0 || saved.key.uniqueID == 0) {
                        continue;
                    }

                    savedRelics.push_back(std::move(saved));
                }
            }


            //
            // ACTORS
            //

            else if (type == kRecordActors) {

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
                        !a_intfc->ReadRecordData(saved.damageMult) ||
                        !a_intfc->ReadRecordData(saved.skipEquipAnim) ||
                        !a_intfc->ReadRecordData(saved.unequipWhenAnimEnds) ||
                        !a_intfc->ReadRecordData(saved.isBarehanded))
                    {
                        ClearSavedState();
                        return;
                    }


                    //
                    // Known relics
                    //

                    std::uint32_t knownCount = 0;

                    if (!a_intfc->ReadRecordData(knownCount) || knownCount > 65536) {
                        ClearSavedState();
                        return;
                    }

                    saved.knownRelics.reserve(knownCount);

                    for (std::uint32_t j = 0; j < knownCount; j++) {
                        SavedRelicKey key;

                        if (!ReadRelicKey(a_intfc, key)) {
                            ClearSavedState();
                            return;
                        }

                        saved.knownRelics.push_back(key);
                    }


                    //
                    // Equipped relics
                    //

                    auto readOptionalRelic = [&](SavedRelicKey& a_key) -> bool
                    {
                        bool valid = false;

                        if (!a_intfc->ReadRecordData(valid)) {
                            return false;
                        }

                        if (!valid) {
                            a_key = {};
                            return true;
                        }

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

                    if (saved.formID != 0) {
                        savedActors.push_back(std::move(saved));
                    }
                }
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

        if (!a_intfc) {
            return;
        }

        //
        // RELICS
        //

        if (!a_intfc->OpenRecord(kRecordRelics, kSerializationVersion)) {
            return;
        }

        std::uint32_t relicCount = 0;

        for (const auto& [identity, relic] : activeRelics) {
            if (relic) {
                relicCount++;
            }
        }

        if (!a_intfc->WriteRecordData(relicCount)) {
            return;
        }

        for (const auto& [identity, relic] : activeRelics) {

            if (!relic) {
                continue;
            }

            const auto key = MakeSavedRelicKey(*relic);

            if (!a_intfc->WriteRecordData(key.baseFormID) ||
                !a_intfc->WriteRecordData(key.uniqueID) ||
                !a_intfc->WriteRecordData(relic->GetType()))
            {
                return;
            }
        }


        //
        // ACTORS
        //

        if (!a_intfc->OpenRecord(kRecordActors, kSerializationVersion))
        {
            return;
        }

        std::uint32_t actorCount = 0;

        for (const auto& [handle, actor] : activeActors) {
            if (actor && actor->GetActor()) {
                actorCount++;
            }
        }

        if (!a_intfc->WriteRecordData(actorCount)) {
            return;
        }

        for (const auto& [handle, actor] : activeActors) {

            if (!actor || !actor->IsValid()) {
                continue;
            }

            auto gameActor = actor->GetActor();

            const RE::FormID formID = gameActor->formID;

            if (!a_intfc->WriteRecordData(formID)) {
                return;
            }


            //
            // Actor state
            //

            const auto titles = actor->GetTitles().underlying();

            if (!a_intfc->WriteRecordData(titles) ||
                !a_intfc->WriteRecordData(actor->level) ||
                !a_intfc->WriteRecordData(actor->meleeSkill) ||
                !a_intfc->WriteRecordData(actor->damageMult))
            {
                return;
            }

            const bool skipEquipAnim = actor->GetSkipEquipAnim();

            const bool unequipWhenAnimEnds = actor->GetUnequipWhenAnimEnds();

            const bool isBarehanded = actor->IsBarehanded();

            if (!a_intfc->WriteRecordData(skipEquipAnim) ||
                !a_intfc->WriteRecordData(unequipWhenAnimEnds) ||
                !a_intfc->WriteRecordData(isBarehanded))
            {
                return;
            }


            //
            // Known relics
            //

            std::vector<SavedRelicKey> knownRelics;

            for (auto* relic : actor->GetKnownRelics()) {

                if (!relic) {
                    continue;
                }

                knownRelics.push_back(MakeSavedRelicKey(*relic));
            }

            const std::uint32_t knownCount = static_cast<std::uint32_t>(knownRelics.size());

            if (!a_intfc->WriteRecordData(knownCount)) {
                return;
            }

            for (const auto& key : knownRelics) {

                if (!a_intfc->WriteRecordData(key.baseFormID) ||
                    !a_intfc->WriteRecordData(key.uniqueID))
                {
                    return;
                }
            }


            //
            // Equipped / last relics
            //

            auto writeRelicKey = [&](RelicWeapon* a_relic) -> bool
            {
                const bool valid = a_relic != nullptr;

                if (!a_intfc->WriteRecordData(valid)) {
                    return false;
                }

                if (!valid) {
                    return true;
                }

                const auto key = MakeSavedRelicKey(*a_relic);

                return
                    a_intfc->WriteRecordData(key.baseFormID) &&
                    a_intfc->WriteRecordData(key.uniqueID);
            };

            if (!writeRelicKey(actor->GetRightHandRelic()) ||
                !writeRelicKey(actor->GetLeftHandRelic()) ||
                !writeRelicKey(actor->GetLastRightHandRelic()) ||
                !writeRelicKey(actor->GetLastLeftHandRelic()))
            {
                return;
            }
        }
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

                auto relicID = GetOrCreateEquippedRelicIdentity(a_actor).value_or(0x0);
                if (relicID == 0x0) return;

                auto& activeRelic = activeRelics[relicID];
                if (activeRelic) {
                    activeRelic->OnEquip(activeActor);
                    spdlog::debug("relic equipped, {}", typeid(type).name());
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

    BaseActor* GetPlayer() const {return player;}
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
    BaseActorPtr playerPtr;
    BaseActor* player = nullptr;
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

            for (const auto& key : saved.knownRelics) {
                if (auto relic = FindRuntimeRelic(key, a_serialization)) {
                    knownRelics.push_back(relic);
                } else {
                    unresolved = true;
                }
            }

            auto resolveOptional = [&](const SavedRelicKey& key) -> RelicWeapon* {
                if (key.baseFormID == 0 || key.uniqueID == 0) {
                    return nullptr;
                }

                auto* relic = FindRuntimeRelic(key, a_serialization);

                if (!relic) {
                    unresolved = true;
                }

                return relic;
            };

            baseActor->SetRightHandRelic(resolveOptional(saved.rightHandRelic));
            baseActor->SetLeftHandRelic(resolveOptional(saved.leftHandRelic));
            baseActor->SetLastRightHandRelic(resolveOptional(saved.lastRightHandRelic));
            baseActor->SetLastLeftHandRelic(resolveOptional(saved.lastLeftHandRelic));
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

                if (relic && relic->GetType() != saved.type) {
                    spdlog::error(
                        "Relic type mismatch for identity {:X}: runtime: {}, saved: {}",
                        identity,
                        static_cast<int>(relic->GetType()),
                        static_cast<int>(saved.type));

                    unresolved = true;
                }
                continue;
            }

            auto relic = RelicFactory::Create(saved.type, weapon);

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

            if (!a_serialization->ResolveFormID(saved.formID, resolvedFormID))
            {
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
            baseActor->SetSkipEquipAnim(saved.skipEquipAnim);
            baseActor->SetUnequipWhenAnimEnds(saved.unequipWhenAnimEnds);
            baseActor->SetBarehanded(saved.isBarehanded);
            baseActor->SetTitles(saved.titles);
        }
        return !unresolved;
    }
    void RestoreSavedState(SKSE::SerializationInterface* a_serialization)
    {
        if (!restorePending || !a_serialization) {
            return;
        }

        bool pending = false;

        pending |= !RestoreRelics(a_serialization);
        pending |= !RestoreActors(a_serialization);
        pending |= !RelinkActorsToRelics(a_serialization);

        restorePending = pending;
    }
    void ClearRuntimeState()
    {
        activeRelics.clear();
        activeActors.clear();

        playerPtr.reset();
        player = nullptr;

        thrownRelics.clear();
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

    RelicType GetRelicType(const RE::TESObjectWEAP* a_weap) const
    {
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
/*
void Initialize(RE::Actor* a_actor)
{
    lastEquippedRelic = Kratos::Relic::kNone;
    lastThrownRelic = Kratos::Relic::kNone;
    if (a_actor) {
        auto kratos = Kratos::GetSingleton();
        auto invChanges = a_actor->GetInventoryChanges();
        auto entries = invChanges->entryList;
        spdlog::info("Kratos's special weapons initializing...");
        for (auto entry : *entries) {
            if (entry && entry->object && entry->object->GetFile()) {
                if (entry->object->IsWeapon()) {
#ifdef NEW_WEAPON_REGISTER_METHOD
#else
                    if (kratos->gLeviathanAxeFormID && entry->object->GetLocalFormID() == kratos->gLeviathanAxeFormID->value && entry->object->GetFile()->fileName == Config::registeredLeviathanFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::LeviathanAxeKWD);
                            LeviathanAxe = weap;
                            spdlog::info("{} is your Leviathan Axe", weap->GetName());
                        }
                    } else if (kratos->gBladeOfChaosFormID && entry->object->GetLocalFormID() == kratos->gBladeOfChaosFormID->value && entry->object->GetFile()->fileName == Config::registeredBladeOfChaosFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            BladeOfChaos = weap;
                            spdlog::info("{} is your Blade of Chaos", weap->GetName());
                        }
                    } else if (kratos->gBladeOfChaosFormID && entry->object->GetLocalFormID() == kratos->gBladeOfChaosFormID->value+1 && entry->object->GetFile()->fileName == Config::registeredLBladeOfChaosFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfChaosKWD);
                            BladeOfChaosL = weap;
                            spdlog::info("{} is your left Blade of Chaos", weap->GetName());
                        }
                    } else if (kratos->gDraupnirSpearFormID && entry->object->GetLocalFormID() == kratos->gDraupnirSpearFormID->value && entry->object->GetFile()->fileName == Config::registeredDraupnirFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::DraupnirSpearKWD);
                            DraupnirSpear = weap;
                            spdlog::info("{} is your Draupnir Spear", weap->GetName());
                        }
                    } else if (kratos->gBladeOfOlympusFormID && entry->object->GetLocalFormID() == kratos->gBladeOfOlympusFormID->value && entry->object->GetFile()->fileName == Config::registeredBladeOfOlympusFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::BladeOfOlympusKWD);
                            BladeOfOlympus = weap;
                            spdlog::info("{} is your Blade of Olympus", weap->GetName());
                        }
                    } else if (kratos->gMjolnirFormID && entry->object->GetLocalFormID() == kratos->gMjolnirFormID->value && entry->object->GetFile()->fileName == Config::registeredMjolnirFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::MjolnirKWD);
                            Mjolnir = weap;
                            spdlog::info("{} is your Mjolnir", weap->GetName());
                        }
                    }
#ifdef TRIDENT
                    else if (kratos->gTridentFormID && entry->object->GetLocalFormID() == kratos->gTridentFormID->value && entry->object->GetFile()->fileName == Config::registeredTridentFileName) {
                        if (auto weap = entry->object->As<RE::TESObjectWEAP>(); weap) {
                            weap->RemoveKeywords(Config::SpecialKWDs);
                            weap->AddKeyword(Config::TridentKWD);
                            Trident = weap;
                            spdlog::info("{} is your Trident", weap->GetName());
                        }
                    }
#endif
#endif
                } else if (entry->object->IsArmor()) {
#ifdef NEW_WEAPON_REGISTER_METHOD
#else
                    if (entry->object->GetLocalFormID() == kratos->gGuardianShieldFormID->value && entry->object->GetFile()->fileName == Config::registeredGuardianShieldFileName) {
                        if (auto shield = entry->object->As<RE::TESObjectARMO>(); shield) {
                            shield->RemoveKeywords(Config::SpecialKWDs);
                            shield->AddKeyword(Config::GuardianShieldKWD);
                            GuardianShield = shield;
                            spdlog::info("{} is your Guardian Shield", shield->GetName());
                        }
                    }
#endif
                }
            } else spdlog::info("entry is not an ordinary object");
        } spdlog::info("Kratos's special weapons initialized.");
    }
}
void ResetRegistrations()
{
    Config::registeredLeviathanID      = 105;
    Config::registeredBladeOfChaosID   = 105;
    Config::registeredLBladeOfChaosID  = 105;
    Config::registeredDraupnirID       = 105;
    Config::registeredBladeOfOlympusID = 105;
    Config::registeredMjolnirID        = 105;
    Config::registeredTridentID        = 105;
    Config::registeredGuardianShieldID = 105;
    Config::registeredLeviathanFileName      = "Not Registered";
    Config::registeredBladeOfChaosFileName   = "Not Registered";
    Config::registeredLBladeOfChaosFileName  = "Not Registered";
    Config::registeredDraupnirFileName       = "Not Registered";
    Config::registeredBladeOfOlympusFileName = "Not Registered";
    Config::registeredMjolnirFileName        = "Not Registered";
    Config::registeredTridentFileName        = "Not Registered";
    Config::registeredGuardianShieldFileName = "Not Registered";
}
void WeaponIdentifier(RE::Actor* a_actor, RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectWEAP* a_LHandWeapon, RE::TESObjectARMO* a_shield)
{
    Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
    Config::SpecialShield->value = (uint8_t)Kratos::Shield::kNone;
    auto RelicName = "not a Relic";
//  auto address = reinterpret_cast<std::uintptr_t>(a_RHandWeapon);
    auto kratos = Kratos::GetSingleton();
    if (a_RHandWeapon && a_RHandWeapon->GetFile()) {
        const auto equippedWeaponFile = a_RHandWeapon->GetFile()->fileName;
        const FormID equippedWeaponID = a_RHandWeapon->GetLocalFormID();
        spdlog::debug("registering the {:8x} from {}", equippedWeaponID, equippedWeaponFile);
        if (kratos->IsInRage(a_actor) && !kratos->IsWantFinishRage())
            kratos->EndRage(kratos->GetLastTriggeredRageType(), false, false, false, a_actor);
        if (Config::LeviathanAxeKWD && a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {
            isLeviathanAxe = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Leviathan Axe";
            LeviathanAxe = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kLeviathanAxe;
            Config::registeredLeviathanFileName = equippedWeaponFile;
            Config::registeredLeviathanID = equippedWeaponID;
            kratos->gLeviathanAxeFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kLeviathanAxe;

            auto Levi = LeviathanAxe::GetSingleton();
            Levi->data.weap     = LeviathanAxe;
            Levi->data.ench     = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            Levi->data.poison   = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            Levi->data.damage   = static_cast<float>(LeviathanAxe->attackDamage);
            if (WeaponIdentify::LeviathanAxe->HasWorldModel()) {
                spdlog::debug("Levi is throwable");
                Levi->SetThrowState(tState::kThrowable);
                Levi->ResetCharge(Levi->data.enchMag, Levi->data.defaultEnchMag, true);
                kratos->SetIsCanCallAxe(a_actor, false);
            } else spdlog::warn("Levi is not equipped for real");

            if (Levi->SpellCatchLevi && a_actor->AsMagicTarget()->HasMagicEffect(Levi->EffCatchLevi)) {a_actor->RemoveSpell(Levi->SpellCatchLevi);}

            if (Levi->LeviathanAxeProjectileA 
            && (Levi->GetThrowState() == tState::kArriving
             || Levi->GetThrowState() == tState::kArrived)) 
                Levi->Catch(true);
        }
        else if (Config::BladeOfChaosKWD && a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {
            isBladeOfChaos = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Chaos";
            BladeOfChaos = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kBladeOfChaos;
            Config::registeredBladeOfChaosFileName = equippedWeaponFile;
            if (a_LHandWeapon && (a_LHandWeapon->HasKeyword(Config::BladeOfChaosKWD) || a_LHandWeapon->GetFile() == a_RHandWeapon->GetFile())) {
                Config::registeredLBladeOfChaosFileName = a_LHandWeapon->GetFile()->fileName;
                Config::registeredLBladeOfChaosID = a_LHandWeapon->GetLocalFormID();
                BladeOfChaosL = a_LHandWeapon;
            }
            Config::registeredBladeOfChaosID = equippedWeaponID;
            kratos->gBladeOfChaosFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kBladeOfChaos;

            auto BoC = BladeOfChaos::GetSingleton();
            BoC->data.weap  = BladeOfChaos;
            BoC->data.weaponModel   = WeaponBone;
            BoC->data.weaponModelL  = ShieldBone;
        }
        else if (Config::DraupnirSpearKWD && a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {
            isDraupnirSpear = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Draupnir Spear";
            DraupnirSpear = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kDraupnirSpear;
            Config::registeredDraupnirFileName = equippedWeaponFile;
            Config::registeredDraupnirID = equippedWeaponID;
            kratos->gDraupnirSpearFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kDraupnirSpear;

            Draupnir::data.weap     = DraupnirSpear;
            Draupnir::data.ench     = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            Draupnir::data.poison   = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            Draupnir::data.damage   = static_cast<float>(DraupnirSpear->attackDamage);
        }
        else if (Config::BladeOfOlympusKWD && a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {
            isBladeOfOlympus = true;
            isRelic = true;
            isKratos = true;
            RelicName = "the Blade of Olympus";
            BladeOfOlympus = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kBladeOfOlympus;
            Config::registeredBladeOfOlympusFileName = equippedWeaponFile;
            Config::registeredBladeOfOlympusID = equippedWeaponID;
            kratos->gBladeOfOlympusFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kBladeOfOlympus;
        }
        else if (Config::MjolnirKWD && a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {
            isMjolnir = true;
            isRelic = true;
            isThor = true;
            RelicName = "the Mjolnir";
            Mjolnir = a_RHandWeapon;
            Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kMjolnir;
            Config::registeredMjolnirFileName = equippedWeaponFile;
            Config::registeredMjolnirID = equippedWeaponID;
            kratos->gMjolnirFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kMjolnir;

            auto mjolnir = Mjolnir::GetSingleton();
            mjolnir->data.weap      = Mjolnir;
            mjolnir->data.ench      = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            mjolnir->data.poison    = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            mjolnir->data.damage    = static_cast<float>(Mjolnir->attackDamage);
        //    mjolnir->MjolnirProjBaseT->model = Mjolnir->model;
        //    mjolnir->MjolnirProjBaseA->model = Mjolnir->model;
            if (WeaponIdentify::Mjolnir->HasWorldModel()) {
                spdlog::debug("Mjolnir is throwable");
                mjolnir->SetThrowState(tStateM::kThrowable);
                mjolnir->ResetCharge(mjolnir->data.enchMag, mjolnir->data.defaultEnchMag, true);
                kratos->SetIsCanCallMjolnir(a_actor, false);
            } else spdlog::warn("Mjolnir is not equipped for real");

            if (mjolnir->SpellCatchMjolnir && a_actor->AsMagicTarget()->HasMagicEffect(mjolnir->EffCatchMjolnir)) {a_actor->RemoveSpell(mjolnir->SpellCatchMjolnir);}

            if (mjolnir->MjolnirProjectileA 
            && (mjolnir->GetThrowState() == tStateM::kArriving
             || mjolnir->GetThrowState() == tStateM::kArrived)) 
                mjolnir->Catch(true);
        }
#ifdef TRIDENT
        else if (Config::TridentKWD && a_RHandWeapon->HasKeyword(Config::TridentKWD)) {
            isTrident = true;
            isRelic = true;
            isPoseidon = true;
            RelicName = "the Trident";
            Trident = a_RHandWeapon;
            Config::SpecialWeapon->value = 3U;//(uint8_t)Kratos::Relic::kTrident;
            Config::registeredTridentFileName = equippedWeaponFile;
            Config::registeredTridentID = equippedWeaponID;
            kratos->gTridentFormID->value = equippedWeaponID;
            lastEquippedRelic = Kratos::Relic::kTrident;

            auto trident = Trident::GetSingleton();
            trident->data.weap      = Trident;
            trident->data.ench      = ObjectUtil::Enchantment::GetEquippedWeaponEnchantment(a_actor, false);
#ifdef EXPERIMENTAL_THROWPOISON
            trident->data.poison    = ObjectUtil::Poison::GetEquippedObjPoison(a_actor, false);
#endif
            trident->data.damage    = static_cast<float>(Trident->attackDamage);
            trident->TridentProjBaseL->model = Trident->model;
            if (WeaponIdentify::Trident->HasWorldModel()) {
                trident->isTridentThrowable = true;
                spdlog::debug("Trident is throwable");
            } else spdlog::warn("Trident is not equipped for real");
        }
#endif
        spdlog::info("{} is {}", a_RHandWeapon->GetName(), RelicName);
    } if (a_shield && a_shield->GetFile()) {
        const auto equippedShieldFile = a_shield->GetFile()->fileName;
        RelicName = "not a Relic";
        if (Config::GuardianShieldKWD && a_shield->HasKeyword(Config::GuardianShieldKWD)) {
            isGuardianShield = true;
            isKratos = true;
            Config::SpecialShield->value = (uint8_t)Kratos::Shield::kGuardianShield;
            Config::registeredGuardianShieldFileName = equippedShieldFile;
            RelicName = "the Guardian Shield";
            GuardianShield = a_shield;
            kratos->gGuardianShieldFormID->value = a_shield->formID;
        }
        spdlog::info("{} is {}", a_shield->GetName(), RelicName);
    }
    a_actor->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
}
void SpecialityCheck(RE::TESObjectWEAP* a_RHandWeapon, RE::TESObjectWEAP* a_LHandWeapon, RE::TESObjectARMO* a_shield, const Kratos::Relic a_relic, const Kratos::Shield a_specialShield)
{
    if (Config::SpecialKWDs.size() < 6) {spdlog::error("check the special equipment keyword list!"); return;}
    if (a_RHandWeapon) {
        switch (a_relic)
        {
        case Kratos::Relic::kNone:
            if (a_RHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
            }
            break;
        case Kratos::Relic::kLeviathanAxe:
            if (!isLeviathanAxe) {
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::LeviathanAxeKWD);
            } break;
        case Kratos::Relic::kBladeOfChaos:
            if (!isBladeOfChaos) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::BladeOfChaosKWD);
                if (a_LHandWeapon && !a_LHandWeapon->HasKeywordInArray(Config::SpecialKWDs, false)) {
                    a_LHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                    a_LHandWeapon->AddKeyword(Config::BladeOfChaosKWD);
                }
            } break;
        case Kratos::Relic::kDraupnirSpear:
            if (!isDraupnirSpear) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::DraupnirSpearKWD);
            } break;
        case Kratos::Relic::kBladeOfOlympus:
            if (!isBladeOfOlympus) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::BladeOfOlympusKWD);
            } break;
        case Kratos::Relic::kMjolnir:
            if (!isMjolnir) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::TridentKWD)) {Config::registeredTridentID = 105; Config::registeredTridentFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::MjolnirKWD);
            } break;
#ifdef TRIDENT
        case Kratos::Relic::kTrident:
            if (!isTrident) {
                if (a_RHandWeapon->HasKeyword(Config::LeviathanAxeKWD)) {Config::registeredLeviathanID = 105; Config::registeredLeviathanFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfChaosKWD)) {Config::registeredBladeOfChaosID = 105; Config::registeredBladeOfChaosFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::DraupnirSpearKWD)) {Config::registeredDraupnirID = 105; Config::registeredDraupnirFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::BladeOfOlympusKWD)) {Config::registeredBladeOfOlympusID = 105; Config::registeredBladeOfOlympusFileName = "Not Registered";}
                if (a_RHandWeapon->HasKeyword(Config::MjolnirKWD)) {Config::registeredMjolnirID = 105; Config::registeredMjolnirFileName = "Not Registered";}
                a_RHandWeapon->RemoveKeywords(Config::SpecialKWDs);
                a_RHandWeapon->AddKeyword(Config::TridentKWD);
            } break;
#endif
        default:
            break;
        }
    }
    if (a_shield) {
        switch (a_specialShield)
        {
        case Kratos::Shield::kNone:
            if (isGuardianShield) {
                if (a_RHandWeapon->HasKeyword(Config::GuardianShieldKWD)) {Config::registeredGuardianShieldID = 105; Config::registeredGuardianShieldFileName = "Not Registered";}
                a_shield->RemoveKeywords(Config::SpecialKWDs);
            } break;
        case Kratos::Shield::kGuardianShield:
            if (!isGuardianShield) {
                a_shield->RemoveKeywords(Config::SpecialKWDs);
                a_shield->AddKeyword(Config::GuardianShieldKWD);
            } break;
        case Kratos::Shield::kDauntlessShield:
            break;
        case Kratos::Shield::kStoneWallShield:
            break;
        case Kratos::Shield::kShatterStarShield:
            break;
        case Kratos::Shield::kOnslaughtShield:
            break;

        default:
            break;
        }
    }
}
void WeaponCheck(const bool a_specialityCheck)
{
    isLeviathanAxe  = false;
    isBladeOfChaos  = false;
    isDraupnirSpear = false;
    isBladeOfOlympus = false;
    isMjolnir = false;
    isTrident = false;
    isRelic = false;
    isKratos = false;
    isThor = false;

    auto AnArchos = PlayerCharacter::GetSingleton();

    RHandBone = GetRhandBone(AnArchos);
    LHandBone = GetLhandBone(AnArchos);
    WeaponBone = GetWeaponBone(AnArchos);
    ShieldBone = GetShieldBone(AnArchos);
    AnimObjectRBone = GetAnimObjectRBone(AnArchos);

    auto pcSkillArchery = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kArchery);
    auto pcSkill1Handed = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kOneHanded);
    auto pcDamageMult   = AnArchos->AsActorValueOwner()->GetActorValue(RE::ActorValue::kAttackDamageMult);
    DamageMult  = 1.f + (pcSkill1Handed / 120) + (pcSkillArchery / 80);
    DamageMult  *= pcDamageMult;

    RE::TESObjectWEAP* RHandWeapon = nullptr;
    RE::TESObjectWEAP* LHandWeapon = nullptr;
    RE::TESObjectARMO* shield = nullptr;
    auto rObj = AnArchos->GetEquippedObject(false);
    auto lObj = AnArchos->GetEquippedObject(true);
    if (rObj) {
        EquippedObjR = rObj->As<TESBoundObject>();
        if (rObj->IsWeapon()) {
            RHandWeapon = rObj->As<TESObjectWEAP>();
        }
    } if (lObj) {
        EquippedObjL = lObj->As<TESBoundObject>();
        if (lObj->IsArmor()) {
            shield = lObj->As<TESObjectARMO>();
        } else if (lObj->IsWeapon()) {
            LHandWeapon = lObj->As<TESObjectWEAP>();
        }
    } if (!lObj && !rObj) {
        isBarehanded = true;
    } else isBarehanded = false;

    if (RHandWeapon || LHandWeapon || shield) {
        if (!RHandWeapon && !LHandWeapon) isBarehanded = true;
        else isBarehanded = false;
        if (a_specialityCheck) {
            SpecialityCheck(RHandWeapon, LHandWeapon, shield, static_cast<Kratos::Relic>((uint8_t)Config::SpecialWeapon->value), static_cast<Kratos::Shield>((uint8_t)Config::SpecialShield->value));
        }
        WeaponIdentifier(AnArchos, RHandWeapon, LHandWeapon, shield);
    } else {
        Config::SpecialWeapon->value = (uint8_t)Kratos::Relic::kNone;
        Config::SpecialShield->value = (uint8_t)Kratos::Shield::kNone;
        AnArchos->SetGraphVariableInt("iRelicWeapon", (uint8_t)Config::SpecialWeapon->value);
    }
}
*/
