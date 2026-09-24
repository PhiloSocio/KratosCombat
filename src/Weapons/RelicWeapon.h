#pragma once

#include "Types.h"
#include "Actors/BaseActor.h"

using RelicIdentity = std::uint64_t;

class BaseActor;

class RelicWeapon
{
public:
    virtual ~RelicWeapon() = default;
    explicit RelicWeapon(RE::TESBoundObject* a_object);

    RelicIdentity        relicIdentity  = 0u;
    RelicType            type           = RelicType::kNone;
    RE::TESObjectWEAP*   weap           = nullptr;
    RE::EnchantmentItem* ench           = nullptr;
    RE::AlchemyItem*     poison         = nullptr;
    float*               enchMag        = nullptr;
    float                defaultEnchMag = 0.f;
    float                damage         = 0.f;
    float                length         = 0.f;

    struct RuntimeData {
        BaseActor* owner = nullptr;
        BaseActor* wielder = nullptr;
        BaseActor* lastWielder = nullptr;

        RE::TESObjectREFR* currentReference = nullptr;
        bool isEquipped = false;
    };

    virtual bool Initialize() = 0;
    virtual void Update() = 0;
    virtual bool IsCharged() const {return _isCharged;}
    virtual void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u);
    virtual void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false);
    virtual void SetWielder(BaseActor* a_actor);

    virtual void OnEquip(BaseActor* a_actor);
    virtual bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) = 0;
    virtual void OnImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) = 0;
    virtual void OnMenuOpenCloseEvent(const bool a_opening) = 0;

    [[nodiscard]] virtual RelicType GetType() const = 0;
    [[nodiscard]] RE::TESObjectWEAP* GetWeapon() const {return weap;}
    [[nodiscard]] RuntimeData& GetRuntimeData() {return runtimeData;}
    [[nodiscard]] BaseActor* GetOwner() const {return runtimeData.owner;}
    [[nodiscard]] BaseActor* GetWielder() const {return runtimeData.wielder;}
    [[nodiscard]] BaseActor* GetLastWielder() const {return runtimeData.lastWielder;}
    [[nodiscard]] bool IsEquipped() const {return runtimeData.isEquipped;}

    RE::SpellItem* SpellChargeCD    = nullptr;
    RE::EnchantmentItem* EnchCharge = nullptr;

    bool _isCharged = false;
    uint8_t chargeHitCount = 0;
protected:
    RuntimeData runtimeData;
};
