#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "REL/Relocation.h"

//#include <glm.hpp>

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

using namespace std::literals;

	using rFlag = RE::TESForm::RecordFlags;
	using pFlag = RE::Projectile::Flags;

#define DEBUG_MODE
//#undef DEBUG_MODE

// for experimental draupnir spear detonating features:
//#define EXPERIMENTAL
//#undef EXPERIMENTAL
#define EXPERIMENTAL_CHARGE_LEVI
#define EXPERIMENTAL_SKIP_EQUIP_ANIM
//#define EXPERIMENTAL_EXTRAARROW
//#define EXPERIMENTAL_SHIELD