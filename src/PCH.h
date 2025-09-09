#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "REL/Relocation.h"
#include <vector>

//#include <glm.hpp>

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

using namespace std::literals;

	using rFlag = RE::TESForm::RecordFlags;
	using pFlag = RE::Projectile::Flags;

#define DEBUG_MODE
//#undef DEBUG_MODE

// for experimental draupnir spear detonating features:
//#define EXPERIMENTAL
#define EXPERIMENTAL_CHARGE_LEVI
#define EXPERIMENTAL_SKIP_EQUIP_ANIM
//#define EXPERIMENTAL_EXTRAARROW
#define EXPERIMENTAL_SHIELD
#define EXPERIMENTAL_THROWPOISON
#define NEW_DRAUPNIR_EXPLOSION_METHOD
#define EXPERIMENTAL_UNEQUIP_AFTER_THROW
//#define NEW_WEAPON_REGISTER_METHOD
//#define TRIDENT
//#define PRECISION
#define NEW_ARRIVING_METHOD
#define SMART_ARRIVING_METHOD
//#define OVERPOWERED
//#define EXPERIMENTAL_LEVIATHAN_MODEL
//#define EXPERIMENTAL_PROJECTILE_HIT_EXPLOSION