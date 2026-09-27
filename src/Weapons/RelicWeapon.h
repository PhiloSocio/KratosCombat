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

    BaseActor* owner = nullptr;
    BaseActor* wielder = nullptr;
    BaseActor* lastWielder = nullptr;

    RE::TESObjectREFR* currentReference = nullptr;
    bool isEquipped = false;

    virtual bool Initialize() = 0;
    virtual void Update() = 0;
    virtual bool IsCharged() const {return _isCharged;}
    virtual void Charge(const uint8_t a_chargeHitCount = 1u, const float a_magnitude = 1.5f, const uint8_t a_stage = 3u, const uint8_t a_coolDown = 15u) = 0;
    virtual void ResetCharge(float* a_magnitude, const float a_defMagnitude, const bool a_justCheck = false, const bool a_justReset = false) = 0;
    void SetWielder(BaseActor* a_actor);
    void SetOwner(BaseActor* a_actor);

    void OnEquip(BaseActor* a_actor);
    virtual bool OnHit(RE::hkpAllCdPointCollector* a_AllCdPointCollector) = 0;
    virtual void PreImpact(RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) = 0;
    virtual void PostImpact(RE::Projectile::ImpactData* a_impactData, RE::TESObjectREFR* a_target, RE::NiPoint3* a_targetLoc, RE::NiPoint3* a_velocity, RE::hkpCollidable* a_collidable) = 0;
    virtual void OnMenuOpenCloseEvent(const bool a_opening) = 0;

    [[nodiscard]] virtual RelicType GetType() const = 0;
    [[nodiscard]] bool HasAbility(const RelicAbility a_ability) const { return abilities.all(a_ability); }
    [[nodiscard]] REX::EnumSet<RelicAbility, std::uint32_t> GetAbility() const { return abilities; }
    [[nodiscard]] RE::TESObjectWEAP* GetWeapon() const {return weap;}
    [[nodiscard]] BaseActor* GetOwner() const {return owner;}
    [[nodiscard]] BaseActor* GetWielder() const {return wielder;}
    [[nodiscard]] BaseActor* GetLastWielder() const {return lastWielder;}
    [[nodiscard]] bool IsEquipped() const {return isEquipped;}

    RE::SpellItem* SpellChargeCD    = nullptr;
    RE::EnchantmentItem* EnchCharge = nullptr;

    bool _isCharged = false;
    uint8_t chargeHitCount = 0;
protected:
    REX::EnumSet<RelicAbility, std::uint32_t> abilities;

};
