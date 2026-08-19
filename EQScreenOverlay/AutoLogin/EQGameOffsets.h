#pragma once
#include <cstdint>

inline uintptr_t FixOffset(uintptr_t absOffset, uintptr_t preferredBase, uintptr_t realBase)
{
    return absOffset - preferredBase + realBase;
}

#include "EQGameOffsets_eqgame.h"
#include "EQGameOffsets_eqmain.h"

