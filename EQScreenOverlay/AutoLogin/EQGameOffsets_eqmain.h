#pragma once
#include <cstdint>

// ---------------------------------------------------------------------------
// Offsets for eqmain.dll (x64, 2026-07-09 build)
// From openvanilla/src/eqlib/include/eqlib/offsets/eqmain.h
// ---------------------------------------------------------------------------

constexpr uintptr_t kEqMainPreferredBase = 0x180000000;

#define EQMain__LoginServerAPI__JoinServer_x            0x180018060
#define EQMain__pinstLoginServerAPI_x                   0x1801804E0
#define EQMain__pinstCXWndManager_x                     0x1803834C8
#define EQMain__CLoginViewManager__HandleLButtonUp_x    0x18001B0F0
#define EQMain__pinstCLoginViewManager_x                0x1801804F8
#define EQMain__pinstLoginClient_x                      (EQMain__pinstCLoginViewManager_x- sizeof(uintptr_t))

// ---------------------------------------------------------------------------
// Absolute virtual addresses (within the process)
// We resolve these by subtracting preferred base and adding real base.
// ---------------------------------------------------------------------------

extern uintptr_t g_realEQMainBase;

inline void InitEQMainBase()
{
    g_realEQMainBase = (uintptr_t)GetModuleHandleW(L"eqmain.dll");
}

inline uintptr_t FixEQMainOffset(uintptr_t x)
{
    return FixOffset(x, kEqMainPreferredBase, g_realEQMainBase);
}