#pragma once

// Ersh
class ProjectileHook
{
public:
	static void Hook()
	{
		spdlog::info("ProjectileHook hooking...");
		REL::Relocation<std::uintptr_t> MissileProjectileVtbl{ RE::VTABLE_MissileProjectile[0] };
		REL::Relocation<std::uintptr_t> ArrowProjectileVtbl{ RE::VTABLE_ArrowProjectile[0] };
		REL::Relocation<std::uintptr_t> FlameProjectileVtbl{ RE::VTABLE_FlameProjectile[0] };
		REL::Relocation<std::uintptr_t> ConeProjectileVtbl{ RE::VTABLE_ConeProjectile[0] };
		REL::Relocation<std::uintptr_t> BeamProjectileVtbl{ RE::VTABLE_BeamProjectile[0] };
	
		_GetLinearVelocityMissile	= MissileProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
		_GetLinearVelocityArrow		= ArrowProjectileVtbl.write_vfunc(0x86, GetLinearVelocityArrow);
		_GetLinearVelocityCone		= ConeProjectileVtbl.write_vfunc(0x86, GetLinearVelocityCone);

		_GetCollisionMissile		= MissileProjectileVtbl.write_vfunc(0xBE, GetCollisionMissile);
		_GetCollisionArrow			= ArrowProjectileVtbl.write_vfunc(0xBE, GetCollisionArrow);							//	hkpCollidable* a_collidable
		_GetCollisionFlame			= FlameProjectileVtbl.write_vfunc(0xBE, GetCollisionFlame);
		_GetCollisionCone			= ConeProjectileVtbl.write_vfunc(0xBE, GetCollisionFlame);
		_GetCollisionBeam			= BeamProjectileVtbl.write_vfunc(0xBE, GetCollisionBeam);

		_Launch 					= SKSE::GetTrampoline().write_call<5>(REL::ID(33672).address() + 0x377, Launch);	//	credits to github.com/fenix31415

		spdlog::info("ProjectileHook done, long live Ersh and Fenix!");
	}
private:
	static void GetLinearVelocityMissile(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityArrow(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);
	static void GetLinearVelocityCone(RE::Projectile* a_this, RE::NiPoint3& a_outVelocity);

	static void GetCollisionMissile(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionArrow(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionFlame(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);
	static void GetCollisionCone(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)		{_GetCollisionCone(a_this, a_AllCdPointCollector);}
	static void GetCollisionBeam(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector)		{_GetCollisionBeam(a_this, a_AllCdPointCollector);}

	static uint32_t* Launch(uint32_t* handle, RE::Projectile::LaunchData* ldata);

	static void LeviAndDraupnir(RE::Projectile* a_this);
	static bool LeviAndDraupnirHit(RE::Projectile* a_this, RE::hkpAllCdPointCollector* a_AllCdPointCollector);

	static inline REL::Relocation<decltype(GetLinearVelocityMissile)> _GetLinearVelocityMissile;
	static inline REL::Relocation<decltype(GetLinearVelocityArrow)> _GetLinearVelocityArrow;
	static inline REL::Relocation<decltype(GetLinearVelocityCone)> _GetLinearVelocityCone;

	static inline REL::Relocation<decltype(GetCollisionMissile)> _GetCollisionMissile;
	static inline REL::Relocation<decltype(GetCollisionArrow)> _GetCollisionArrow;
	static inline REL::Relocation<decltype(GetCollisionFlame)> _GetCollisionFlame;
	static inline REL::Relocation<decltype(GetCollisionCone)> _GetCollisionCone;
	static inline REL::Relocation<decltype(GetCollisionBeam)> _GetCollisionBeam;

	static inline REL::Relocation<decltype(Launch)> _Launch;
};