#pragma once
#include <cstdint>

// ---------------------------------------------------------------------------
// Offsets for eqgame.exe (x64, 2026-07-09 Live)
// All values from openvanilla/src/eqlib/include/eqlib/offsets/eqgame.h
// ---------------------------------------------------------------------------

constexpr uintptr_t kEqGamePreferredBase = 0x140000000;

// Version info
#define __ClientDate                20260709u
#define __ExpectedVersionDate       "Jul  9 2026"
#define __ExpectedVersionTime       "12:42:52"
#define __ActualVersionDate_x       0x140990BE8
#define __ActualVersionTime_x       0x140990BD8
#define __ActualVersionBuild_x      0x140855870

// Misc Globals
#define pinstCEverQuest_x           0x140F33268
#define pinstCXWndManager_x         0x140F59658

// Heap allocators (game's operator new/delete)
#define __eq_new_x                  0x1406E31A0
#define __eq_delete_x               0x1406E2F48

// CXStr globals
#define CXStr__gFreeLists_x         0x140D84060

// CEditWnd
#define CEditWnd__SetWindowText_x   0x1406080D0

// Function Offsets
#define CCharacterListWnd__SelectCharacter_x    0x1400D8DD0
#define CCharacterListWnd__EnterWorld_x         0x1400D7BD0
#define CXWnd__IsType_x                         0x1405CEA00
#define CXWnd__GetChildItem_x                   0x1405CC220

extern uintptr_t g_realEQGameBase;

inline void InitEQGameBase()
{
    g_realEQGameBase = (uintptr_t)GetModuleHandleW(L"eqgame.exe");
    if (g_realEQGameBase == 0)
        g_realEQGameBase = (uintptr_t)GetModuleHandleW(NULL);
}

inline uintptr_t FixEQGameOffset(uintptr_t x)
{
    return FixOffset(x, kEqGamePreferredBase, g_realEQGameBase);
}