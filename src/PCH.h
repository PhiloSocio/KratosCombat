#pragma once

#undef ENABLE_SKYRIM_VR

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "REL/Relocation.h"
#include <vector>

//#include <glm.hpp>

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

using namespace std::literals;

	using ProjectileRot = RE::Projectile::ProjectileRot;
	using EventChecker = RE::BSEventNotifyControl;
	using rFlag = RE::TESForm::RecordFlags;
	using pFlag = RE::Projectile::Flags;


namespace std
{
	template <class T>
	struct hash<RE::BSPointerHandle<T>>
	{
		uint32_t operator()(const RE::BSPointerHandle<T>& a_handle) const
		{
			uint32_t nativeHandle = const_cast<RE::BSPointerHandle<T>*>(&a_handle)->native_handle();  // ugh
			return nativeHandle;
		}
	};
}

#define DEBUG_MODE
//#undef DEBUG_MODE

#define EXPERIMENTAL_CHARGE_LEVI
//#define EXPERIMENTAL_EXTRAARROW
#define EXPERIMENTAL_SHIELD
#define EXPERIMENTAL_THROWPOISON
#define NEW_DRAUPNIR_EXPLOSION_METHOD
#define EXPERIMENTAL_UNEQUIP_AFTER_THROW
//#define NEW_WEAPON_REGISTER_METHOD
//#define TRIDENT
//#define OVERPOWERED
//#define EXPERIMENTAL_PROJECTILE_HIT_EXPLOSION