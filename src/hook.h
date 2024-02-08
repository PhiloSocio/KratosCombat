#pragma once

// huge thanks to Ersh & fenix

static float* g_deltaTime = (float*)RELOCATION_ID(523660, 410199).address();			//	sensitive to slow time spell
static float* g_deltaTimeRealTime = (float*)RELOCATION_ID(523661, 410200).address();	//	const

class ProjectileHook
{
public:
//	static inline REL::Version runtimeVer;
	static void Hook()
	{
		spdlog::info("Hooking Projectiles...");
	//	REL::Relocation<std::uintptr_t> ProjectileVtbl{ RE::VTABLE_Projectile[0] };
	//	REL::Relocation<std::uintptr_t> MissileProjectileVtbl{ RE::VTABLE_MissileProjectile[0] };
		REL::Relocation<std::uintptr_t> ArrowProjectileVtbl{ RE::VTABLE_ArrowProjectile[0] };
	//	REL::Relocation<std::uintptr_t> FlameProjectileVtbl{ RE::VTABLE_FlameProjectile[0] };
	//	REL::Relocation<std::uintptr_t> ConeProjectileVtbl{ RE::VTABLE_ConeProjectile[0] };
	//	REL::Relocation<std::uintptr_t> BeamProjectileVtbl{ RE::VTABLE_BeamProjectile[0] };

	//	_GetLinearVelocityProj		= ProjectileVtbl.write_vfunc(0x86, GetLinearVelocityProj);
	//	_GetLinearVelocityMissile	= MissileProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
		_GetLinearVelocityArrow		= ArrowProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
	//	_GetLinearVelocityCone		= ConeProjectileVtbl.write_vfunc(0x86, GetLinearVelocityCone);

	//	_GetCollisionProjectile		= ProjectileVtbl.write_vfunc(0xBE, GetCollisionProjectile);
	//	_GetCollisionMissile		= MissileProjectileVtbl.write_vfunc(0xBE, GetCollisionMissile);
		_GetCollisionArrow			= ArrowProjectileVtbl.write_vfunc(0xBE, GetCollisionArrow);
	//	_GetCollisionFlame			= FlameProjectileVtbl.write_vfunc(0xBE, GetCollisionFlame);
	//	_GetCollisionCone			= ConeProjectileVtbl.write_vfunc(0xBE, GetCollisionCone);
	//	_GetCollisionBeam			= BeamProjectileVtbl.write_vfunc(0xBE, GetCollisionBeam);

//		_UpdateMovingCone			= ConeProjectileVtbl.write_vfunc(0xAB, UpdateMovingCone);

//		_GetKillOnCollisionMissile	= MissileProjectileVtbl.write_vfunc(0xB8, GetKillOnCollisionMissile);
//		_GetKillOnCollisionArrow	= ArrowProjectileVtbl.write_vfunc(0xB8, GetKillOnCollisionArrow);

		_GetArrowImpactData			= ArrowProjectileVtbl.write_vfunc(0xBD, GetArrowImpactData);
//		_GetConeImpactData			= ConeProjectileVtbl.write_vfunc(0xBD, GetConeImpactData);

//		auto& longLiveFenix = SKSE::GetTrampoline();
//		spdlog::info("runtime version is {}", runtimeVer);
//		auto launchAddress			= (runtimeVer <= SKSE::RUNTIME_SSE_1_5_97) ? (REL::ID(33672).address() + 0x377) : (REL::ID(34452).address() + 0x337);	//-40	//0x550A37
//		auto updateMovingAddress	= (runtimeVer <= SKSE::RUNTIME_SSE_1_5_97) ? (REL::ID(42624).address() + 0x155) : (REL::ID(42624).address() + 0x155);
//		_Launch						= longLiveFenix.write_call<5>(launchAddress, Launch);					//	credits to github.com/fenix31415	33672 for se,	34452 for ae
//		_UpdateMovingCone			= longLiveFenix.write_call<5>(updateMovingAddress, UpdateMovingCone);	//	credits to github.com/fenix31415	42624 for se,	43789 for ae

		spdlog::info("ProjectileHook done, long live Ersh and Fenix!");
	}
private:
	static void GetLinearVelocityProj(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityMissile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityCone(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);

	static void GetCollisionProjectile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionMissile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionFlame(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionCone(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);// {_GetCollisionCone(a_this, a_AllCdPointCollector); }
	static void GetCollisionBeam(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)		{_GetCollisionBeam(a_this, a_AllCdPointCollector);}

	static bool GetKillOnCollisionMissile(RE::MissileProjectile* a_this);
	static bool GetKillOnCollisionArrow(RE::ArrowProjectile* a_this);

//	static uint32_t* Launch(uint32_t* handle, RE::Projectile::LaunchData* ldata);
//	static uint32_t* _Launch(uint32_t* handle, RE::Projectile::LaunchData* ldata);
	static void UpdateMovingCone(RE::ConeProjectile* proj, float dtime);
//	static void _UpdateMovingCone(RE::ConeProjectile* proj, float dtime);
	static RE::Projectile::ImpactData* GetArrowImpactData(RE::ArrowProjectile *proj, RE::TESObjectREFR *a2, RE::NiPoint3 *a3, RE::NiPoint3 *a_velocity, RE::hkpCollidable *a_collidable, uint32_t a6, char a7);

	static void LeviAndDraupnir(RE::Projectile* a_this);
	static inline bool LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);

	static inline REL::Relocation<decltype(GetLinearVelocityProj)> _GetLinearVelocityProj;
	static inline REL::Relocation<decltype(GetLinearVelocityMissile)> _GetLinearVelocityMissile;
	static inline REL::Relocation<decltype(GetLinearVelocityArrow)> _GetLinearVelocityArrow;
	static inline REL::Relocation<decltype(GetLinearVelocityCone)> _GetLinearVelocityCone;

	static inline REL::Relocation<decltype(GetCollisionProjectile)> _GetCollisionProjectile;
	static inline REL::Relocation<decltype(GetCollisionMissile)> _GetCollisionMissile;
	static inline REL::Relocation<decltype(GetCollisionArrow)> _GetCollisionArrow;
	static inline REL::Relocation<decltype(GetCollisionFlame)> _GetCollisionFlame;
	static inline REL::Relocation<decltype(GetCollisionCone)> _GetCollisionCone;
	static inline REL::Relocation<decltype(GetCollisionBeam)> _GetCollisionBeam;

	static inline REL::Relocation<decltype(GetKillOnCollisionMissile)> _GetKillOnCollisionMissile;
	static inline REL::Relocation<decltype(GetKillOnCollisionArrow)> _GetKillOnCollisionArrow;

	/*	long live fenix!	*/
//	static inline REL::Relocation<decltype(Launch)> _Launch;
	static inline REL::Relocation<decltype(UpdateMovingCone)> _UpdateMovingCone;
	static inline REL::Relocation<decltype(GetArrowImpactData)> _GetArrowImpactData;
//	static inline REL::Relocation<decltype(GetConeImpactData)> _GetConeImpactData;
};
