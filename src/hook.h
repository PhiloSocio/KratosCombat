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

        _GetCollisionMissile        = MissileProjectileVtbl.write_vfunc(0xBE, GetCollisionMissile);
        _GetMissileImpactData       = MissileProjectileVtbl.write_vfunc(0xBD, GetMissileImpactData);
        _GetLinearVelocityArrow     = ArrowProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
        _GetCollisionArrow          = ArrowProjectileVtbl.write_vfunc(0xBE, GetCollisionArrow);
//      _GetKillOnCollisionArrow    = ArrowProjectileVtbl.write_vfunc(0xB8, GetKillOnCollisionArrow);
        _GetArrowImpactData         = ArrowProjectileVtbl.write_vfunc(0xBD, GetArrowImpactData);

        spdlog::info("ProjectileHook done, long live Ersh and Fenix!");
    }
private:
    static void GetLinearVelocityArrow(RE::ArrowProjectile* a_this, RE::NiPoint3& a_outVelocity);
    static void GetCollisionArrow(RE::ArrowProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
    static void GetCollisionMissile(RE::MissileProjectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
    static bool GetKillOnCollisionArrow(RE::ArrowProjectile* a_this);
    static RE::Projectile::ImpactData* GetMissileImpactData(RE::MissileProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, uint32_t a7);
    static RE::Projectile::ImpactData* GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, uint32_t a7);

    static void LeviAndDraupnir(RE::Projectile* a_this);
    static inline bool LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
    static inline void LeviAndDraupnirImpactData(RE::Projectile::ImpactData* impactData, RE::MissileProjectile *proj, RE::TESObjectREFR *a_target, RE::NiPoint3 *a_targetLoc, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable);

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
#ifdef EXPERIMENTAL_SKIP_EQUIP_ANIM
        _OnEquipItem        = PlayerCharacterVtbl.write_vfunc(0xB2, OnEquipItem);
#endif
    }
private:
    using EventChecker = RE::BSEventNotifyControl;
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