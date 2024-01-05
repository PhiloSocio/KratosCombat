#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "REL/Relocation.h"

#include <glm.hpp>

#define RELOCATION_OFFSET(SE, AE) REL::VariantOffset(SE, AE, 0).offset()

using namespace std::literals;

	using rFlag = RE::TESForm::RecordFlags;
	using pFlag = RE::Projectile::Flags;
