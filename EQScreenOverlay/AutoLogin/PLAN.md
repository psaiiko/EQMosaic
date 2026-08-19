# AutoLogin Plan

## Source Repos
- `C:\Projects\openvanilla\src\eqlib\include\eqlib\` — CXWnd, CXStr, EWndRuntimeType, offsets
- `C:\Projects\openvanilla\src\plugins\autologin\MQ2AutoLogin.cpp` + `StateMachine.cpp` — state machine pattern
- `C:\Projects\openvanilla\src\eqlib\include\eqlib\game\LoginFrontend.h` — eqmain vtable offsets

## Project Files
- `EQTypes.h` — CXStr, CStrRep, ArrayClass, EWndRuntimeType, UIType, CXWndOffsets, CXWndVtable
- `EQGameOffsets.h` — eqgame/eqmain pointer/function offsets, FixEQGameOffset/FixEQMainOffset
- `EQGameInterface.cpp` — ReadCXStr, IsWndType, DumpWindows, FindEditField, ClickButton, SetEditText, etc.
- `EQGameInterface.h` — public API
- `AutoLogin.cpp` — pulse loop, state machine, credential loading

## Key Data Structures

### CXStr (8 bytes) → CStrRep (24 bytes + utf8)
CXStr: `CStrRep* m_data` at +0x00.
CStrRep: `refCount(4), alloc(4), length(4), encoding(4), freeList(8), utf8[1]`.

### CXWnd Layout (0x268 bytes)
| +0x000 vfptr | +0x008 pPrev | +0x010 pNext | +0x018 m_pFirstNode | +0x020 m_pLastNode | +0x028 m_count |
|---|---|---|---|---|---|
| +0x030 members start | **+0x081 dShow** | **+0x120 Enabled** | **+0x124 Location** (4x int) | **+0x1A0 RuntimeTypes** (ArrayClass2<u32>) | **+0x1D8 WindowText** (CXStr) |

### CXWndVtable (eqmain, from LoginFrontend.h)
```
WndNotification = 0x110  // int __fastcall(void*,void*,uint32_t,void*)
Show            = 0x1B0
SetWindowText   = 0x250
GetWindowName   = 0x268
```
IsType/GetType are **NOT virtual**. Call at fixed address `CXWnd__IsType_x = 0x1405C9150`.
Signature: `bool __fastcall IsType(CXWnd* this, int EWndRuntimeType)`

### EWndRuntimeType (sequential, from CXWnd.h)
WRT_WND=0, WRT_LISTWND=1, **WRT_EDITWND=2**, WRT_TREEWND=3, WRT_PAGEWND=4,
WRT_TABWND=5, WRT_HOTKEYWND=6, WRT_EDITHOTKEYWND=7, WRT_RANGESLIDERWND=8,
WRT_STMLWND=9, WRT_BROWSERWND=10, WRT_MODALMESSAGEWND=11, WRT_CHECKBOXWND=12,
**WRT_SIDLSCREENWND=13**, WRT_SLIDERWND=14, **WRT_LABEL=15**, **WRT_BUTTON=16**,
WRT_GAUGE=17, WRT_COMBOBOX=18, WRT_CHATWND=19, WRT_HELPWND=20.

### CSidlScreenWnd (+0x2D0)
- +0x268 bControlsCreated | **+0x270 SidlText** (CXStr) | +0x278 SidlPiece

### CEditBaseWnd (eqmain)
- +0x268 eAlign..MaxBytesUTF8 | **+0x280 InputText** (CXStr)

### CEditWnd extends CEditBaseWnd (+0x408)
- +0x3D0 bAnchorAtStart..bEditable | +0x3D8 FilterChars (CXStr) | +0x3E0 EditMode | +0x3E4 PasswordChar (wchar_t) | +0x3E8 LineIndices

### CListWnd (extends CXWnd, eqmain)
- +0x270 ItemsArray (ArrayClass<SListWndLine>) | +0x288 Columns (ArrayClass<SListWndColumn>) | +0x2A0 CurSel (int) | +0x2A4 CurCol (int)

### LoginClient (eqmain, offset from `pinstCLoginViewManager - 8`)
| Offset | Field | Type |
|--------|-------|------|
| 0x130 | LoginName | CXStr |
| 0x138 | Password | CXStr |
| 0x140 | LoginNameCopy | CXStr |
| 0x148 | PasswordCopy | CXStr |
| 0x150 | AccountKey | CXStr |
| 0x158 | accountId | int |
| 0x160 | selectedServer | EQClientServerData* |
| 0x168 | displayDeviceIndex | int |
| 0x16C | isLoggingIn | bool |
| 0x178 | ServerList | DoublyLinkedList<EQClientServerData*> |

## Key Technical Decisions

### `AllocCStrRep` uses `__eq_new_x` (game's operator new)
Instead of `VirtualAlloc`, we now allocate `CStrRep` via the game's own heap allocator at `0x1406DF7C0`. The `freeList` field is set to `CXStr__gFreeLists_x` (game's free list array) so `FreeRep` can properly return or free the block. `refCount` starts at 1.

### `SetEditText` calls `CEditWnd__SetWindowText_x`
Instead of writing to `InputText` directly, we call the game's function at `0x140602430` via a stack-allocated temp CXStr. This properly updates the edit control's display, unlike raw memory writes.

### `ClickButton` uses `XWM_LCLICK` via vtable at `0x110`
Matches MQ's `SendWndNotification(pBtn, pBtn, XWM_LCLICK)`.

### Button found as child of connect screen
Login button found via `GetChildWindow(pConnect, "LOGIN")` (child of "connect" screen by WindowText), not as a top-level window. Avoids finding stale buttons from other UI states.

## Login Flow (current implementation)

### PulseSplashScreen (phase: Ready/SplashScreen)
1. Wait 500ms
2. Fill account name via `SetEditText(FindEditField(0), ...)` if different from current
3. Fill password via `SetEditText(FindEditField(1), ...)` unconditionally
4. Wait 2000ms total
5. Click LOGIN via `GetChildWindow(pConnect, "LOGIN")` → `ClickButton`
6. Transition to `Connecting`

### PulseConnecting (phase: Connecting)
1. Check for game state changes (InGame, CharSelect)
2. Check for dialogs (okdialog → ConnectConfirm, serverselect → ServerSelect)
3. Timeout after 45s → Failed

### PulseConnectConfirm (phase: ConnectConfirm/WaitConnect)
Handles connection result dialogs (success, error messages, server full).

### PulseServerSelect (phase: ServerSelect)
Finds server by name from `LoginClient.ServerList` → calls `JoinServer(serverID)`.

### PulseCharSelect (phase: CharSelect)
Selects character from `CLW_CharactersScreen` → `Character_List` → `EnterWorld()`.

## Important Caveats
1. **`__eq_new_x` allocator works** — game's own `operator new` at `0x1406DF7C0`. CStrRep allocated on game's heap with `freeList = gFreeLists`. No more crashes.
2. **`pinstEqLogin_x` is WRONG** — LoginClient lives in eqmain at `pinstCLoginViewManager - 8`. Our `GetLoginClient()` reads from eqgame offset which is stale. `SetLoginCredentials` was removed because of this.
3. **pEverQuest null at early init** — CEverQuest object hasn't been constructed yet. `GetGameState()` returns UNKNOWN until game advances.
4. **MainWindow null at early init** — CXWndManager::MainWindow not populated until UI constructed. Use pWindows array instead.
5. **CEditWnd__SetWindowText_x** at 0x140602430 is essential for proper edit field display update.
6. **ScreenMode = 3 breaks things** — MQ sets it but our attempt caused issues. Not using it.

## Status (2026-05-26)

### ✅ Working
- **ReadCXStr** with VirtualQuery validation — no crashes
- **IsWndType** via CXWnd__IsType_x — 136/136 correct classifications
- **DumpWindows** — type, vtable, visible, enabled, location, WindowText, SidlText
- **FindEditField** via IsType(WRT_EDITWND) — finds 4 edit fields
- **GetTopLevelWindow** — searches pWindows by SidlText then WindowText
- **GetChildWindow** — recursive tree walk by name
- **ClickButton** via vtable WndNotification at offset 0x110 (XWM_LCLICK)
- **SetEditText** via CEditWnd__SetWindowText_x — properly updates display
- **AllocCStrRep** via __eq_new_x — game heap allocation, no crashes
- **Login credential fill** — account + password both set correctly
- **LOGIN button click** — found as child of connect screen, click works, phase transitions properly
- **Build** — 0 errors, 0 warnings, Release x64

### ❌ Known Issues (all addressed)
- ~~WriteCXStrAt via VirtualAlloc crashes~~ → FIXED: uses __eq_new_x
- ~~SetEditText didn't update display~~ → FIXED: uses CEditWnd__SetWindowText_x
- ~~SetLoginCredentials crashed~~ → FIXED: removed (obsolete)  
- ~~LOGOUT button spam~~ → FIXED: single click with phase transition
- ~~Password disappeared on click~~ → FIXED: single click + proper button lookup

### 📋 TODO (next: Server Selection)
1. **Server Select screen** — detect when server select is visible, iterate `LoginClient.ServerList`, find target server by name, click Quick Connect / SELECT button
2. **Character Select screen** — detect when character select is visible, click character in list, click Enter World
3. **Dialog handling** — connect errors, server full, already logged in kick
4. **Credential storage** — config file format, account/password/server/character
5. **EULA/Splash handling** — handle EulaWindow, seizurewarning, OrderWindow, news, soesplash/dbgsplash
