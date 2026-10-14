#pragma once

// huge thanks to Ersh & fenix & LokiWasTaken

class ProjectileHook
{
public:
    static void Hook()
    {
        spdlog::info("Hooking Projectiles...");
        REL::Relocation<std::uintptr_t> MissileProjectileVtbl{ RE::VTABLE_MissileProjectile[0] };
        REL::Relocation<std::uintptr_t> ArrowProjectileVtbl{ RE::VTABLE_ArrowProjectile[0] };
        REL::Relocation<std::uintptr_t> ConeProjectileVtbl{ RE::VTABLE_BeamProjectile[0] };

        _GetCollisionMissile        = MissileProjectileVtbl.write_vfunc(0xBE, GetCollisionMissile);
        _GetMissileImpactData       = MissileProjectileVtbl.write_vfunc(0xBD, GetMissileImpactData);
        _UpdateArrow                = ArrowProjectileVtbl.write_vfunc(0xAB, UpdateArrow);
        _GetLinearVelocityArrow     = ArrowProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
        _GetCollisionArrow          = ArrowProjectileVtbl.write_vfunc(0xBE, GetCollisionArrow);
    //    _GetKillOnCollisionArrow    = ArrowProjectileVtbl.write_vfunc(0xB8, GetKillOnCollisionArrow);
        _GetArrowImpactData         = ArrowProjectileVtbl.write_vfunc(0xBD, GetArrowImpactData);
        _OnArrowKill                = ArrowProjectileVtbl.write_vfunc(0xA8, OnArrowKill);

        spdlog::info("ProjectileHook done, long live Ersh and Fenix!");
    }
private:
    static void UpdateArrow(RE::ArrowProjectile* a_this, float a_delta);
    static void OnArrowKill(RE::ArrowProjectile* a_this);
    static void GetLinearVelocityArrow(RE::ArrowProjectile* a_this, RE::NiPoint3& a_outVelocity);
    static void GetCollisionArrow(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
    static void GetCollisionMissile(RE::MissileProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
    static bool GetKillOnCollisionArrow(RE::ArrowProjectile* a_this);
    static RE::Projectile::ImpactData* GetMissileImpactData(RE::MissileProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, uint32_t a7);
    static RE::Projectile::ImpactData* GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, uint32_t a7);

    static void LeviAndDraupnir(RE::Projectile* a_this);
    static inline bool LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
    static inline void LeviAndDraupnirImpactData(RE::Projectile::ImpactData* impactData, RE::MissileProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable);

    static inline REL::Relocation<decltype(UpdateArrow)>                _UpdateArrow;
    static inline REL::Relocation<decltype(OnArrowKill)>                _OnArrowKill;
    static inline REL::Relocation<decltype(GetLinearVelocityArrow)>     _GetLinearVelocityArrow;
    static inline REL::Relocation<decltype(GetCollisionArrow)>          _GetCollisionArrow;
    static inline REL::Relocation<decltype(GetCollisionMissile)>        _GetCollisionMissile;
    static inline REL::Relocation<decltype(GetKillOnCollisionArrow)>    _GetKillOnCollisionArrow;
    static inline REL::Relocation<decltype(GetMissileImpactData)>       _GetMissileImpactData;
    static inline REL::Relocation<decltype(GetArrowImpactData)>         _GetArrowImpactData;    /*  long live fenix!    */
};

class PlayerHook
{
public:
    static void Hook()
    {
        REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

        _Update             = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
//      _ProcessEventPC     = PlayerCharacterVtbl.write_vfunc(0x01, ProcessEventPC);
//      _UnequipItem        = PlayerCharacterVtbl.write_vfunc(0xA1, UnequipItem);
        _OnEquipItem        = PlayerCharacterVtbl.write_vfunc(0xB2, OnEquipItem);
    }
private:
    static void Update(RE::PlayerCharacter* a_this, const float a_delta);
    static EventChecker ProcessEventPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_dispatcher);
//  static void UnequipItem(RE::PlayerCharacter* a_this, std::uint64_t a_arg1, RE::TESBoundObject* a_object);
    static void OnEquipItem(RE::PlayerCharacter* a_this, bool a_playAnim);

    static bool ModEvent(RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_dispatcher);
    static bool SkipAnim(RE::PlayerCharacter* a_this, bool a_playAnim);

    static inline REL::Relocation<decltype(Update)>         _Update;
    static inline REL::Relocation<decltype(ProcessEventPC)> _ProcessEventPC;
//  static inline REL::Relocation<decltype(UnequipItem)>    _UnequipItem;
    static inline REL::Relocation<decltype(OnEquipItem)>    _OnEquipItem;
};

class NPCHook
{
public:
    static void Hook()
    {
        REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Actor[0] };
        REL::Relocation<std::uintptr_t> CharacterVtbl{ RE::VTABLE_Character[0] };

        _Update                 = ActorVtbl.write_vfunc(0xAD, Update);
//      _ProcessEventPC         = PlayerActorVtbl.write_vfunc(0x01, ProcessEventPC);
//      _UnequipItem            = PlayerActorVtbl.write_vfunc(0xA1, UnequipItem);
        _OnEquipItem            = ActorVtbl.write_vfunc(0xB2, OnEquipItem);
        _AddObjectToContainer   = CharacterVtbl.write_vfunc(0x5A, AddObjectToContainer);
    }
private:
    static void Update(RE::Actor* a_this, const float a_delta);
//  static void UnequipItem(RE::Actor* a_this, std::uint64_t a_arg1, RE::TESBoundObject* a_object);
    static void OnEquipItem(RE::Actor* a_this, bool a_playAnim);
    static void AddObjectToContainer(RE::Actor* a_this, RE::TESBoundObject* a_object, RE::ExtraDataList* a_extraList, std::int32_t a_count, RE::TESObjectREFR* a_fromRefr);

    static inline REL::Relocation<decltype(Update)>         _Update;
//  static inline REL::Relocation<decltype(UnequipItem)>    _UnequipItem;
    static inline REL::Relocation<decltype(OnEquipItem)>    _OnEquipItem;
    static inline REL::Relocation<decltype(AddObjectToContainer)>   _AddObjectToContainer;
};

class AttackHook
{
public:
    static void Hook()
    {
        REL::Relocation<std::uintptr_t> AttackBlockHandlerVtbl{ RE::VTABLE_AttackBlockHandler[0] };

    //    _ProcessButton             = AttackBlockHandlerVtbl.write_vfunc(0x04, ProcessButton);

        auto& trampoline = SKSE::GetTrampoline();
        REL::Relocation<uintptr_t> onHit{ RELOCATION_ID(37673, 38627) }; //  credits to https://github.com/LokiWasTaken
        _OnMeleeHit = trampoline.write_call<5>(onHit.address() + RELOCATION_OFFSET(0x3C0, 0x4A8), OnMeleeHit);
    }
    static void BeforeDamage(RE::Actor* a_target, RE::HitData& a_this);
    static void BeforeDamage(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
private:
    static bool	ProcessButton(RE::AttackBlockHandler* a_handler, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data);
    static void OnMeleeHit(RE::Actor* a_target, RE::HitData& a_this);

    static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
    static inline REL::Relocation<decltype(OnMeleeHit)> _OnMeleeHit;
};

struct ActorEquipManagerHook
{
public:
    using EquipObject_t = decltype(&RE::ActorEquipManager::EquipObject);
    using UnequipObject_t = decltype(&RE::ActorEquipManager::UnequipObject);

    static void Hook()
    {
        REL::Relocation<std::uintptr_t> target1{RELOCATION_ID(37938, 38894), REL::VariantOffset(0xE5, 0x170, 0xE5)};
        REL::Relocation<std::uintptr_t> target2{RELOCATION_ID(37945, 38901), REL::VariantOffset(0x138, 0x1B9, 0x0)};

        auto& trampoline = SKSE::GetTrampoline();
        _EquipObject    = trampoline.write_call<5>(target1.address(), EquipObject);
        _UnequipObject  = trampoline.write_call<5>(target2.address(), UnequipObject);
    }

private:
    static void EquipObject(
        RE::ActorEquipManager* a_manager,
        RE::Actor* a_actor,
        RE::TESBoundObject* a_object,
        RE::ExtraDataList* a_extraData,
        std::uint32_t a_count,
        const RE::BGSEquipSlot* a_slot,
        bool a_queueEquip,
        bool a_forceEquip,
        bool a_playSounds,
        bool a_applyNow);

    static void UnequipObject(
        RE::ActorEquipManager* a_manager,
        RE::Actor* a_actor,
        RE::TESBoundObject* a_object,
        RE::ExtraDataList* a_extraData,
        std::uint32_t a_count,
        const RE::BGSEquipSlot* a_slot,
        bool a_queueEquip,
        bool a_forceEquip,
        bool a_playSounds,
        bool a_applyNow,
        const RE::BGSEquipSlot* a_slotToReplace);

    static inline REL::Relocation<EquipObject_t> _EquipObject;
    static inline REL::Relocation<UnequipObject_t> _UnequipObject;
};
