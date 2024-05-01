#pragma once

// huge thanks to Ersh & fenix

static float* g_deltaTime = (float*)RELOCATION_ID(523660, 410199).address();			//	sensitive to slow time spell
static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address();	//	const

class ProjectileHook
{
public:
	static void Hook()
	{
		spdlog::info("Hooking Projectiles...");
		REL::Relocation<std::uintptr_t> ArrowProjectileVtbl{ RE::VTABLE_ArrowProjectile[0] };

		_GetLinearVelocityArrow		= ArrowProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
		_GetCollisionArrow			= ArrowProjectileVtbl.write_vfunc(0xBE, GetCollisionArrow);
//		_GetKillOnCollisionArrow	= ArrowProjectileVtbl.write_vfunc(0xB8, GetKillOnCollisionArrow);
		_GetArrowImpactData			= ArrowProjectileVtbl.write_vfunc(0xBD, GetArrowImpactData);

		spdlog::info("ProjectileHook done, long live Ersh and Fenix!");
	}
private:
	static void GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static bool GetKillOnCollisionArrow(RE::ArrowProjectile* a_this);
	static RE::Projectile::ImpactData* GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a2, RE::NiPoint3 *a3, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, char a7);

	static void LeviAndDraupnir(RE::Projectile* a_this);
	static inline bool LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);

	static inline REL::Relocation<decltype(GetLinearVelocityArrow)>		_GetLinearVelocityArrow;
	static inline REL::Relocation<decltype(GetCollisionArrow)>			_GetCollisionArrow;
	static inline REL::Relocation<decltype(GetKillOnCollisionArrow)>	_GetKillOnCollisionArrow;
	static inline REL::Relocation<decltype(GetArrowImpactData)>			_GetArrowImpactData;	/*	long live fenix!	*/
};

class PlayerHook
{
public:
	static void Hook()
	{
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

//		_UnequipItem		= PlayerCharacterVtbl.write_vfunc(0xA1, UnequipItem);
		_OnEquipItem		= PlayerCharacterVtbl.write_vfunc(0xB2, OnEquipItem);
	}
private:
//	static void UnequipItem(RE::PlayerCharacter* a_this, std::uint64_t a_arg1, RE::TESBoundObject* a_object);
	static void OnEquipItem(RE::PlayerCharacter* a_this, bool a_playAnim);

	static bool SkipAnim(RE::PlayerCharacter* a_this, bool a_playAnim);

//	static inline REL::Relocation<decltype(UnequipItem)>		_UnequipItem;
	static inline REL::Relocation<decltype(OnEquipItem)>		_OnEquipItem;
};