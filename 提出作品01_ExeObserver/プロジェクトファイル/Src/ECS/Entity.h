#pragma once
//=============================================================================
//=============================================================================
#include <cstdint>
#include <limits>

using EntityID = uint32_t;

// Windows の max マクロと衝突しないよう () で囲む
static constexpr EntityID INVALID_ENTITY = (std::numeric_limits<EntityID>::max)();
