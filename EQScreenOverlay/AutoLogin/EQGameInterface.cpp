#include <windows.h>
#include "EQGameInterface.h"
#include "EQGameOffsets.h"
#include "EQTypes.h"
#include <cstdio>
#include <string>

extern void ConsolePrintf(const char* fmt, ...);

uintptr_t g_realEQGameBase = 0;
uintptr_t g_realEQMainBase = 0;
static bool s_initialized = false;

// ---------------------------------------------------------------------------
// CXStr helpers
// ---------------------------------------------------------------------------

// Check if the memory at ptr is readable (committed, not guarded).
static bool IsReadablePtr(const void* ptr, size_t size)
{
    if (!ptr) return false;
    MEMORY_BASIC_INFORMATION mbi;
    SIZE_T ret = VirtualQuery(ptr, &mbi, sizeof(mbi));
    if (ret == 0) return false;
    DWORD protect = mbi.Protect;
    return (mbi.State == MEM_COMMIT) &&
        (protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) != 0 &&
        !(protect & (PAGE_GUARD | PAGE_NOACCESS));
}

static std::string ReadCXStr(uintptr_t addr)
{
    CStrRep* pRep = *(CStrRep**)addr;
    if (!pRep) return {};
    if (!IsReadablePtr(pRep, sizeof(CStrRep))) return {};
    int len = pRep->length;
    if (len <= 0 || len > 65536) return {};
    if (!IsReadablePtr(pRep->utf8, (size_t)len)) return {};

    if (pRep->encoding == StringEncodingUtf16)
    {
        // UTF-16: length is byte count (2 bytes per wchar_t)
        int wlen = len / 2;
        if (wlen <= 0) return {};
        // Convert to narrow string
        int needed = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)pRep->utf8, wlen, nullptr, 0, nullptr, nullptr);
        if (needed <= 0) return {};
        std::string result((size_t)needed, '\0');
        WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)pRep->utf8, wlen, &result[0], needed, nullptr, nullptr);
        return result;
    }

    return std::string(pRep->utf8, len);
}

typedef void* (__fastcall* EqNewFn)(size_t size);

static CStrRep* AllocCStrRep(const char* text)
{
    if (!text || !*text) return nullptr;
    size_t len = strlen(text);
    size_t allocSize = offsetof(CStrRep, utf8) + len + 1;

    auto eqNew = (EqNewFn)FixEQGameOffset(__eq_new_x);
    if (!eqNew) return nullptr;

    CStrRep* rep = (CStrRep*)eqNew(allocSize);
    if (!rep) return nullptr;

    rep->refCount.store(1);
    rep->alloc = (uint32_t)allocSize;
    rep->length = (uint32_t)len;
    rep->encoding = 0;
    rep->freeList = (void*)FixEQGameOffset(CXStr__gFreeLists_x);
    memcpy(rep->utf8, text, len);
    rep->utf8[len] = '\0';
    return rep;
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void InitEQInterface()
{
    if (s_initialized) return;
    InitEQGameBase();
    InitEQMainBase();

    char buf[1024];
    uintptr_t eqGameBase = (uintptr_t)GetModuleHandleW(L"eqgame.exe");
    uintptr_t eqMainBase = (uintptr_t)GetModuleHandleW(L"eqmain.dll");
    uintptr_t pWndMgrAddr = (eqMainBase ? FixEQMainOffset(EQMain__pinstCXWndManager_x) : 0);
    uintptr_t pWndMgrVal = pWndMgrAddr ? *(uintptr_t*)pWndMgrAddr : 0;

    // Dump tree walk from MainWindow (CXWndManager+0x088)
    if (pWndMgrVal)
    {
        void* mainWnd = *(void**)(pWndMgrVal + CXWndManagerOffsets::MainWindow);
        snprintf(buf, sizeof(buf),
            "[Overlay][AutoLogin] MainWindow=0x%llX\n",
            (unsigned long long)(uintptr_t)mainWnd);
        ConsolePrintf("%s", buf);
    }

    s_initialized = true;
}

bool IsEQReady()
{
    return s_initialized && g_realEQGameBase != 0;
}

bool CheckEQVersion()
{
    if (!g_realEQGameBase)
    {
        ConsolePrintf("[Overlay][AutoLogin] VersionCheck: eqgame base not resolved\n");
        return false;
    }

    const char* actualDate = (const char*)FixEQGameOffset(__ActualVersionDate_x);
    const char* actualTime = (const char*)FixEQGameOffset(__ActualVersionTime_x);

    if (!actualDate || !actualTime)
    {
        ConsolePrintf("[Overlay][AutoLogin] VersionCheck: failed to read version strings\n");
        return false;
    }

    bool match = (strncmp(__ExpectedVersionDate, actualDate, strlen(__ExpectedVersionDate)) == 0 &&
        strncmp(__ExpectedVersionTime, actualTime, strlen(__ExpectedVersionTime)) == 0);

    if (!match)
    {
        ConsolePrintf("[Overlay][AutoLogin] VersionCheck: MISMATCH — expected %s %s, got %s %s\n",
            __ExpectedVersionDate, __ExpectedVersionTime, actualDate, actualTime);

        char msg[512];
        snprintf(msg, sizeof(msg),
            "EQ client version mismatch.\n\n"
            "Expected: %s %s\n"
            "Actual:   %s %s\n\n"
            "Auto-login will be disabled.\n"
            "Update offsets in EQGameOffsets.h for the new client.",
            __ExpectedVersionDate, __ExpectedVersionTime,
            actualDate ? actualDate : "?", actualTime ? actualTime : "?");

        MessageBoxA(nullptr, msg, "EQScreenOverlay — Version Mismatch",
            MB_OK | MB_ICONWARNING | MB_TASKMODAL);
        return false;
    }

    ConsolePrintf("[Overlay][AutoLogin] VersionCheck: OK (%s %s)\n", actualDate, actualTime);
    return true;
}

uintptr_t GetEQGameBase() { return g_realEQGameBase; }
uintptr_t GetEQMainBase() { return g_realEQMainBase; }

// ---------------------------------------------------------------------------
// Game State
// ---------------------------------------------------------------------------

EQGameState GetGameState()
{
    if (!g_realEQGameBase) return GAMESTATE_UNKNOWN;
    void* pEverQuest = *(void**)FixEQGameOffset(pinstCEverQuest_x);
    if (!pEverQuest) return GAMESTATE_UNKNOWN;
    return (EQGameState) * (int*)((uintptr_t)pEverQuest + CEverQuestOffsets::GameState);
}

const char* GetGameStateName(EQGameState state)
{
    switch (state)
    {
    case GAMESTATE_PRECHARSELECT:  return "PRECHARSELECT";
    case GAMESTATE_CHARSELECT:     return "CHARSELECT";
    case GAMESTATE_CHARCREATE:     return "CHARCREATE";
    case GAMESTATE_POSTCHARSELECT: return "POSTCHARSELECT";
    case GAMESTATE_INGAME:         return "INGAME";
    case GAMESTATE_LOADING_WORLD:  return "LOADING_WORLD";
    case GAMESTATE_LOGGINGIN:      return "LOGGINGIN";
    case GAMESTATE_UNLOADING:      return "UNLOADING";
    default:                       return "UNKNOWN";
    }
}

// ---------------------------------------------------------------------------
// CXWnd name reading & window lookup
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Type detection — call CXWnd::IsType directly (NOT virtual, fixed address)
// Reference: eqlib/offsets/eqgame.h : CXWnd__IsType_x
// ---------------------------------------------------------------------------

typedef bool(__fastcall* IsTypeFn)(void* wnd, int type);
static bool IsWndType(void* wnd, EWndRuntimeType type)
{
    if (!wnd) return false;
    if (!g_realEQGameBase) return false;
    IsTypeFn fn = (IsTypeFn)FixEQGameOffset(CXWnd__IsType_x);
    if (!fn) return false;
    return fn(wnd, (int)type);
}

static std::string GetWindowScreenName(void* wnd)
{
    if (!wnd) return {};
    // Try eqmain SidlText offset first (login screens created by eqmain.dll)
    std::string name = ReadCXStr((uintptr_t)wnd + CXWndOffsets::SidlTextEqmain);
    if (!name.empty()) return name;
    // Try eqgame SidlText offset (screens created by eqgame.exe's CSidlManager)
    name = ReadCXStr((uintptr_t)wnd + CXWndOffsets::SidlTextEqgame);
    if (!name.empty()) return name;
    // DataStr holds the SIDL piece name for some windows.
    name = ReadCXStr((uintptr_t)wnd + CXWndOffsets::DataStr);
    if (!name.empty()) return name;
    name = ReadCXStr((uintptr_t)wnd + CXWndOffsets::WindowText);
    return name;
}

// Recursive child tree walk (siblings via TListNode::m_pNext, children via TList::m_pFirstNode)
static void* FindChildInTree(void* wnd, const char* name)
{
    if (!wnd || !name) return nullptr;

    std::string wndName = GetWindowScreenName(wnd);
    if (!wndName.empty() && _stricmp(wndName.c_str(), name) == 0)
        return wnd;

    void* child = *(void**)((uintptr_t)wnd + CXWndOffsets::FirstChild);
    while (child)
    {
        void* found = FindChildInTree(child, name);
        if (found) return found;
        child = *(void**)((uintptr_t)child + CXWndOffsets::NextSibling);
    }
    return nullptr;
}

// Get the CXWndManager pointer (eqmain preferred, fallback to eqgame)
static void* GetWndManager()
{
    void* mgr = nullptr;
    if (g_realEQMainBase)
    {
        uintptr_t addr = FixEQMainOffset(EQMain__pinstCXWndManager_x);
        if (IsReadablePtr((void*)addr, sizeof(void*)))
        {
            mgr = *(void**)addr;
            // Sanity check: mgr must point to readable memory
            if (mgr && !IsReadablePtr(mgr, 0x20))
                mgr = nullptr;
        }
    }
    if (!mgr && g_realEQGameBase)
    {
        uintptr_t addr = FixEQGameOffset(pinstCXWndManager_x);
        if (IsReadablePtr((void*)addr, sizeof(void*)))
        {
            mgr = *(void**)addr;
            if (mgr && !IsReadablePtr(mgr, 0x20))
                mgr = nullptr;
        }
    }
    return mgr;
}

static const char* RuntimeTypeName(void* wnd)
{
    if (!wnd) return "null";
    if (IsWndType(wnd, WRT_EDITWND))            return "CEditBaseWnd";
    if (IsWndType(wnd, WRT_BUTTON))             return "CButtonWnd";
    if (IsWndType(wnd, WRT_LISTWND))            return "CListWnd";
    if (IsWndType(wnd, WRT_STMLWND))            return "CStmlWnd";
    if (IsWndType(wnd, WRT_CHECKBOXWND))        return "CCheckBoxWnd";
    if (IsWndType(wnd, WRT_SLIDERWND))          return "CSliderWnd";
    if (IsWndType(wnd, WRT_LABEL))              return "CLabel";
    if (IsWndType(wnd, WRT_SIDLSCREENWND))      return "CSidlScreenWnd";
    if (IsWndType(wnd, WRT_TABWND))             return "CTabWnd";
    if (IsWndType(wnd, WRT_COMBOBOX))           return "CComboBoxWnd";
    if (IsWndType(wnd, WRT_GAUGE))              return "CGaugeWnd";
    if (IsWndType(wnd, WRT_CHATWND))            return "CChatWnd";
    if (IsWndType(wnd, WRT_TREEWND))            return "CTreeWnd";
    if (IsWndType(wnd, WRT_PAGEWND))            return "CPageWnd";
    if (IsWndType(wnd, WRT_BROWSERWND))         return "CBrowserWnd";
    if (IsWndType(wnd, WRT_MODALMESSAGEWND))    return "CModalMessageWnd";
    if (IsWndType(wnd, WRT_HELPWND))            return "CHelpWnd";
    if (IsWndType(wnd, WRT_WND))                return "CXWnd";
    return "?";
}

void* FindEditField(int index)
{
    void* mgr = GetWndManager();
    if (!mgr) return nullptr;

    int count = *(int*)((uintptr_t)mgr + CXWndManagerOffsets::WindowsArray + 0x00);
    void** data = *(void***)((uintptr_t)mgr + CXWndManagerOffsets::WindowsArray + 0x08);
    if (!data || count <= 0) return nullptr;

    int found = 0;
    for (int i = 0; i < count; i++)
    {
        uintptr_t wndAddr = (uintptr_t)data[i];
        if (wndAddr == 0 || wndAddr == (uintptr_t)-1) continue;

        if (IsWndType((void*)wndAddr, WRT_EDITWND))
        {
            if (found == index) return (void*)wndAddr;
            found++;
        }
    }
    return nullptr;
}

// Recursive helper: walks children and siblings, returns the index-th CEditBaseWnd
static void* FindChildEditFieldRecursive(void* wnd, int* index)
{
    if (!wnd || !index) return nullptr;

    if (IsWndType(wnd, WRT_EDITWND))
    {
        if (*index == 0) return wnd;
        (*index)--;
    }

    void* child = *(void**)((uintptr_t)wnd + CXWndOffsets::FirstChild);
    while (child)
    {
        void* found = FindChildEditFieldRecursive(child, index);
        if (found) return found;
        child = *(void**)((uintptr_t)child + CXWndOffsets::NextSibling);
    }
    return nullptr;
}

void* FindChildEditField(void* parentWnd, int index)
{
    if (!parentWnd || index < 0) return nullptr;
    return FindChildEditFieldRecursive(parentWnd, &index);
}

void SetEditTextDirect(void* editWnd, const char* text)
{
    if (!editWnd || !text || !*text) return;

    CStrRep* rep = AllocCStrRep(text);
    if (!rep) return;

    // Release old CStrRep if any, then assign new one at InputText offset
    CStrRep* old = *(CStrRep**)((uintptr_t)editWnd + CEditBaseWndOffsets::InputText);
    if (old) {
        if (old->refCount.fetch_sub(1) <= 1) {
            auto del = (void(__fastcall*)(void*))FixEQGameOffset(__eq_delete_x);
            if (del) del(old);
        }
    }
    *(CStrRep**)((uintptr_t)editWnd + CEditBaseWndOffsets::InputText) = rep;
}

void DumpWindows()
{
    void* mgr = GetWndManager();
    if (!mgr) { ConsolePrintf("[Overlay][AutoLogin] DumpWindows: no wndmgr\n"); return; }

    int count = *(int*)((uintptr_t)mgr + CXWndManagerOffsets::WindowsArray + 0x00);
    void** data = *(void***)((uintptr_t)mgr + CXWndManagerOffsets::WindowsArray + 0x08);
    if (!data || count <= 0) return;

    char buf[1024];
    snprintf(buf, sizeof(buf), "[Overlay][AutoLogin] DumpWindows count=%d\n", count);
    ConsolePrintf("%s", buf);

    for (int i = 0; i < count; i++)
    {
        uintptr_t wndAddr = (uintptr_t)data[i];
        if (wndAddr == 0 || wndAddr == (uintptr_t)-1) continue;

        uintptr_t vtab = *(uintptr_t*)wndAddr;
        bool visible = *(bool*)(wndAddr + CXWndOffsets::dShow);
        bool enabled = *(bool*)(wndAddr + CXWndOffsets::Enabled);
        const char* typeName = RuntimeTypeName((void*)wndAddr);

        int l = *(int*)(wndAddr + CXWndOffsets::RectLeft);
        int t = *(int*)(wndAddr + CXWndOffsets::RectTop);
        int r = *(int*)(wndAddr + CXWndOffsets::RectRight);
        int b = *(int*)(wndAddr + CXWndOffsets::RectBottom);

        std::string wndText = ReadCXStr(wndAddr + CXWndOffsets::WindowText);
        std::string sidlText = ReadCXStr(wndAddr + CXWndOffsets::SidlTextEqgame);

        snprintf(buf, sizeof(buf),
            "[Overlay][AutoLogin] [%d] wnd=0x%llX vt=0x%llX v=%d e=%d type=%s rect=(%d,%d,%d,%d) txt='%s' sidl='%s'\n",
            i, (unsigned long long)wndAddr, (unsigned long long)vtab,
            (int)visible, (int)enabled, typeName,
            l, t, r, b,
            wndText.c_str(), sidlText.c_str());
        ConsolePrintf("%s", buf);
    }
}

// Find a top-level window by name, scanning pWndMgr->pWindows.
void* GetTopLevelWindow(const char* screenName)
{
    if (!screenName) return nullptr;
    void* mgr = GetWndManager();
    if (!mgr) return nullptr;

    int count = *(int*)((uintptr_t)mgr + CXWndManagerOffsets::WindowsArray + 0x00);
    void** data = *(void***)((uintptr_t)mgr + CXWndManagerOffsets::WindowsArray + 0x08);
    if (!data || count <= 0) return nullptr;

    for (int i = 0; i < count; i++)
    {
        uintptr_t wndAddr = (uintptr_t)data[i];
        if (wndAddr == 0 || wndAddr == (uintptr_t)-1) continue;

        // SidlText: try eqmain offset first, then eqgame offset
        std::string name = ReadCXStr(wndAddr + CXWndOffsets::SidlTextEqmain);
        if (!name.empty() && _stricmp(name.c_str(), screenName) == 0)
            return (void*)wndAddr;
        name = ReadCXStr(wndAddr + CXWndOffsets::SidlTextEqgame);
        if (!name.empty() && _stricmp(name.c_str(), screenName) == 0)
            return (void*)wndAddr;

        name = ReadCXStr(wndAddr + CXWndOffsets::WindowText);
        if (!name.empty() && _stricmp(name.c_str(), screenName) == 0)
            return (void*)wndAddr;
    }
    return nullptr;
}

// Find a child (any descendant) by name via recursive tree walk.
// Compares against SidlText, WindowText, and DataStr.
// and the window's CXMLData piece name if accessible.
void* GetChildWindow(void* parentWnd, const char* childName)
{
    if (!parentWnd || !childName) return nullptr;
    void* first = *(void**)((uintptr_t)parentWnd + CXWndOffsets::FirstChild);
    if (!first) return nullptr;
    return FindChildInTree(first, childName);
}

// GetChildWindow using CXWnd::GetItem - simpler approach
void* GetChildWindowTyped(void* parentWnd, const char* childName)
{
    if (!parentWnd || !childName) return nullptr;

    uintptr_t func = FixEQGameOffset(CXWnd__GetChildItem_x);
    if (!func)
        return nullptr;

    typedef CXWnd* (__fastcall* GetChildItemFn)(void* pThis, const CXStr& childName);
    GetChildItemFn fn = (GetChildItemFn)func;

    CXStr strChild{};
    CStrRep* rep = AllocCStrRep(childName);
    if (!rep)
        return nullptr;
    strChild.m_data = rep;

    CXWnd* result = fn(parentWnd, strChild);

    if (rep) {
        auto deleteFn = (void(__fastcall*)(void*))FixEQGameOffset(__eq_delete_x);
        if (deleteFn)
            deleteFn(rep);
    }

    return (void*)result;
}

bool IsWindowVisible(void* wnd)
{
    if (!wnd) return false;
    return *(bool*)((uintptr_t)wnd + CXWndOffsets::dShow);
}

bool IsWindowEnabled(void* wnd)
{
    if (!wnd) return false;
    return *(bool*)((uintptr_t)wnd + CXWndOffsets::Enabled);
}

void* GetActiveWindow(const char* screenName)
{
    // In MQ2AutoLogin, FindActiveWindow checks IsWindowVisible + IsEnabled
    // Not strict - just returns first visible enabled window with this name
    void* wnd = GetTopLevelWindow(screenName);
    if (wnd && IsWindowVisible(wnd) && IsWindowEnabled(wnd))
        return wnd;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Edit window helpers
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Button helpers - simulate click via WndNotification virtual
// WndNotification signature: int __fastcall(CXWnd* pThis, CXWnd* sender, uint32_t msg, void* data)
// ---------------------------------------------------------------------------

void ClickButton(void* buttonWnd)
{
    if (!buttonWnd) return;
    auto vtable = *(uintptr_t**)buttonWnd;
    if (!vtable) return;

    typedef int(__fastcall* WndNotificationFn)(void*, void*, uint32_t, void*);
    WndNotificationFn wndNotify = (WndNotificationFn)vtable[CXWndVtable::WndNotification / 8];
    if (wndNotify)
        wndNotify(buttonWnd, buttonWnd, XWM_LCLICK, nullptr);
}

// For buttons that need WM_LMOUSEUP instead
void ClickButtonUp(void* buttonWnd)
{
    if (!buttonWnd) return;
    auto vtable = *(uintptr_t**)buttonWnd;
    if (!vtable) return;

    typedef int(__fastcall* WndNotificationFn)(void*, void*, uint32_t, void*);
    WndNotificationFn wndNotify = (WndNotificationFn)vtable[CXWndVtable::WndNotification / 8];
    if (wndNotify)
        wndNotify(buttonWnd, buttonWnd, XWM_LMOUSEUP, nullptr);
}

// ---------------------------------------------------------------------------
// Dialog text reading
// ---------------------------------------------------------------------------

// Find the first CStmlWnd child with non-empty STMLText under a parent.
// Used as fallback when name-based child lookup fails.
static void* FindDisplayChildRecursive(void* wnd)
{
    if (!wnd) return nullptr;
    void* child = *(void**)((uintptr_t)wnd + CXWndOffsets::FirstChild);
    while (child)
    {
        if (IsWndType(child, WRT_STMLWND))
        {
            std::string text = ReadCXStr((uintptr_t)child + CStmlWndOffsets::STMLTextEqgame);
            if (!text.empty()) return child;
            text = ReadCXStr((uintptr_t)child + CStmlWndOffsets::STMLTextEqmain);
            if (!text.empty()) return child;
        }
        void* found = FindDisplayChildRecursive(child);
        if (found) return found;
        child = *(void**)((uintptr_t)child + CXWndOffsets::NextSibling);
    }
    return nullptr;
}

// Find the first visible/enabled CButtonWnd child under a parent (fallback).
static void* FindFirstButtonChild(void* wnd)
{
    if (!wnd) return nullptr;
    void* child = *(void**)((uintptr_t)wnd + CXWndOffsets::FirstChild);
    while (child)
    {
        if (IsWndType(child, WRT_BUTTON))
            return child;
        child = *(void**)((uintptr_t)child + CXWndOffsets::NextSibling);
    }
    return nullptr;
}

// Gets the text from the OK_Display STML window inside okdialog
std::string GetDialogDisplayText(const char* dialogName, const char* displayChild)
{
    void* pDialog = GetTopLevelWindow(dialogName);
    if (!pDialog) return {};

    void* pDisplay = GetChildWindow(pDialog, displayChild);
    // Fallback: scan children for any CStmlWnd with non-empty text
    if (!pDisplay)
        pDisplay = FindDisplayChildRecursive(pDialog);
    if (!pDisplay) return {};

    // Try both eqgame and eqmain STMLText offsets
    std::string text = ReadCXStr((uintptr_t)pDisplay + CStmlWndOffsets::STMLTextEqgame);
    if (!text.empty()) return text;
    text = ReadCXStr((uintptr_t)pDisplay + CStmlWndOffsets::STMLTextEqmain);
    if (!text.empty()) return text;
    text = ReadCXStr((uintptr_t)pDisplay + CXWndOffsets::WindowText);
    return text;
}

std::string GetOkDialogText()
{
    return GetDialogDisplayText("okdialog", "OK_Display");
}

std::string GetYesNoDialogText()
{
    return GetDialogDisplayText("yesnodialog", "YESNO_Display");
}

bool IsOkDialogVisible()
{
    void* pDlg = GetTopLevelWindow("okdialog");
    return pDlg && IsWindowVisible(pDlg);
}

bool IsYesNoDialogVisible()
{
    void* pDlg = GetTopLevelWindow("yesnodialog");
    return pDlg && IsWindowVisible(pDlg);
}

void ClickOkButton()
{
    void* pDlg = GetTopLevelWindow("okdialog");
    if (!pDlg) return;
    void* pBtn = GetChildWindow(pDlg, "OK_OKButton");
    if (!pBtn) pBtn = GetChildWindow(pDlg, "OK_OkButton");
    if (!pBtn) pBtn = FindFirstButtonChild(pDlg);
    if (pBtn) ClickButton(pBtn);
}

void ClickYesButton()
{
    void* pDlg = GetTopLevelWindow("yesnodialog");
    if (!pDlg) return;
    void* pBtn = GetChildWindow(pDlg, "YESNO_YesButton");
    if (!pBtn) pBtn = FindFirstButtonChild(pDlg);
    if (pBtn) ClickButton(pBtn);
}

void ClickNoButton()
{
    void* pDlg = GetTopLevelWindow("yesnodialog");
    if (!pDlg) return;
    void* pBtn = GetChildWindow(pDlg, "YESNO_NoButton");
    if (!pBtn)
    {
        // Try to find any button other than the first one
        void* child = *(void**)((uintptr_t)pDlg + CXWndOffsets::FirstChild);
        int btnIdx = 0;
        while (child)
        {
            if (IsWndType(child, WRT_BUTTON))
            {
                if (btnIdx == 1) { pBtn = child; break; }
                btnIdx++;
            }
            child = *(void**)((uintptr_t)child + CXWndOffsets::NextSibling);
        }
    }
    if (pBtn) ClickButton(pBtn);
}

// ---------------------------------------------------------------------------
// List window helpers
// ---------------------------------------------------------------------------

int GetListCurSel(void* listWnd)
{
    if (!listWnd) return -1;
    return *(int*)((uintptr_t)listWnd + CListWndOffsets::CurSel);
}

void SetListCurSel(void* listWnd, int sel)
{
    if (!listWnd) return;
    *(int*)((uintptr_t)listWnd + CListWndOffsets::CurSel) = sel;
}

int GetListItemCount(void* listWnd)
{
    if (!listWnd) return 0;
    ArrayClass<SListWndLine>* items = (ArrayClass<SListWndLine>*)((uintptr_t)listWnd + CListWndOffsets::ItemsArray);
    return items->Count;
}

std::string GetListItemText(void* listWnd, int row, int col)
{
    if (!listWnd) return {};
    ArrayClass<SListWndLine>* items = (ArrayClass<SListWndLine>*)((uintptr_t)listWnd + CListWndOffsets::ItemsArray);
    if (row < 0 || row >= items->Count || !items->Data)
        return {};

    SListWndLine& line = items->Data[row];
    if (col < 0 || col >= line.Cells.Count || !line.Cells.Data)
        return {};

    return ReadCXStr((uintptr_t)&line.Cells.Data[col].Text);
}

// ---------------------------------------------------------------------------
// Server list traversal (LoginClient.ServerList)
// ---------------------------------------------------------------------------

void* GetLoginClient()
{
    if (!g_realEQMainBase) return nullptr;
    uintptr_t pAddr = FixEQMainOffset(EQMain__pinstLoginClient_x);
    if (!IsReadablePtr((void*)pAddr, sizeof(void*)))
        return nullptr;
    void* pClient = *(void**)pAddr;
    if (!pClient) return nullptr;

    // Sanity check: the vtable at +0x00 should be a readable code pointer
    if (!IsReadablePtr(pClient, sizeof(void*)))
        return nullptr;
    // Check that ServerList DoublyLinkedList vtable at +0x178 looks valid
    void* srvListVtab = *(void**)((uintptr_t)pClient + LoginClientOffsets::ServerList);
    if (!IsReadablePtr(srvListVtab, sizeof(void*)))
        return nullptr;

    return pClient;
}

static void* GetLoginServerAPI()
{
    if (!g_realEQMainBase) return nullptr;
    return *(void**)FixEQMainOffset(EQMain__pinstLoginServerAPI_x);
}

// Walk LoginClient.ServerList (DoublyLinkedList<EQClientServerData*>)
// and find a server by name (exact match only).
// Returns a pointer to the EQClientServerData, or nullptr.
void* FindServerByName(const char* serverName)
{
    if (!serverName || !*serverName) return nullptr;

    void* pClient = GetLoginClient();
    if (!pClient) return nullptr;

    DoublyLinkedList<EQClientServerData*>* pList =
        (DoublyLinkedList<EQClientServerData*>*)((uintptr_t)pClient + LoginClientOffsets::ServerList);

    for (auto pNode = pList->m_pHead; pNode; pNode = pNode->m_pNext)
    {
        EQClientServerData* pServer = pNode->m_object;
        if (!pServer) continue;

        std::string name = ReadCXStr((uintptr_t)&pServer->ServerName);
        if (name.empty()) continue;

        if (_stricmp(name.c_str(), serverName) == 0)
            return pServer;
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// UI-based server finder (does not require LoginClient)
// ---------------------------------------------------------------------------

// Recursive child scan by type, returns first child matching the given type.
static void* FindChildByType(void* wnd, EWndRuntimeType type)
{
    if (!wnd) return nullptr;

    if (IsWndType(wnd, type))
        return wnd;

    void* child = *(void**)((uintptr_t)wnd + CXWndOffsets::FirstChild);
    while (child)
    {
        void* found = FindChildByType(child, type);
        if (found) return found;
        child = *(void**)((uintptr_t)child + CXWndOffsets::NextSibling);
    }
    return nullptr;
}

// Read the server name from a CListWnd row/col.
static std::string GetListCellText(void* listWnd, int row, int col)
{
    if (!listWnd) return {};
    ArrayClass<SListWndLine>* items =
        (ArrayClass<SListWndLine>*)((uintptr_t)listWnd + CListWndOffsets::ItemsArray);
    if (row < 0 || row >= items->Count || !items->Data)
        return {};
    SListWndLine& line = items->Data[row];
    if (col < 0 || col >= line.Cells.Count || !line.Cells.Data)
        return {};
    return ReadCXStr((uintptr_t)&line.Cells.Data[col].Text);
}

// Find a server by name using the CListWnd on the server select screen.
// Returns the server ID (from the line's Data field) or -1.
int FindServerIDByNameViaUI(const char* serverName)
{
    if (!serverName || !*serverName) return -1;

    void* selWnd = GetTopLevelWindow("serverselect");
    if (!selWnd) return -1;

    void* listWnd = FindChildByType(selWnd, WRT_LISTWND);
    if (!listWnd) return -1;

    if (!IsWindowVisible(listWnd) || !IsWindowEnabled(listWnd))
        return -1;

    ArrayClass<SListWndLine>* items =
        (ArrayClass<SListWndLine>*)((uintptr_t)listWnd + CListWndOffsets::ItemsArray);
    if (!items->Data) return -1;

    for (int i = 0; i < items->Count; i++)
    {
        std::string name = GetListCellText(listWnd, i, 0);
        if (name.empty()) continue;
        if (_stricmp(name.c_str(), serverName) == 0)
        {
            uint64_t data = items->Data[i].Data;
            int serverID = (int)(data & 0xFFFFFFFF);
            ConsolePrintf("[Overlay][AutoLogin] Found server \"%s\" at row %d, Data=0x%llX (ID=%d)\n",
                serverName, i, data, serverID);
            return serverID;
        }
    }
    return -1;
}

// Returns the server ID for a named server, or -1.
int FindServerIDByName(const char* serverName)
{
    void* pServer = FindServerByName(serverName);
    if (!pServer) return -1;
    return ((EQClientServerData*)pServer)->ID;
}

// Read server status flags (eServerStatus enum at +0x78)
int GetServerStatusFlags(void* serverData)
{
    if (!serverData) return -1;
    return *(int*)((uintptr_t)serverData + 0x78);
}

// Join a server by ID
void JoinServer(int serverID)
{
    void* pAPI = GetLoginServerAPI();
    if (!pAPI || !g_realEQMainBase) return;

    uintptr_t func = FixEQMainOffset(EQMain__LoginServerAPI__JoinServer_x);
    if (!func) return;

    // Validate pAPI and func before calling
    if (!IsReadablePtr(pAPI, 0x10))
    {
        ConsolePrintf("[Overlay][AutoLogin] JoinServer: bad pAPI=0x%llX\n", (unsigned long long)(uintptr_t)pAPI);
        return;
    }

    // Check that func looks like executable code
    bool funcIsExecutable = false;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)func, &mbi, sizeof(mbi)) && mbi.State == MEM_COMMIT
        && (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
        funcIsExecutable = true;

    if (!funcIsExecutable)
    {
        ConsolePrintf("[Overlay][AutoLogin] JoinServer: func=0x%llX is not executable (protect=0x%X)\n",
            (unsigned long long)func, mbi.Protect);
        return;
    }

    ConsolePrintf("[Overlay][AutoLogin] JoinServer: pAPI=0x%llX func=0x%llX serverID=%d\n",
        (unsigned long long)(uintptr_t)pAPI, (unsigned long long)func, serverID);

    typedef unsigned int(__fastcall* JoinServerFn)(void*, int, void*, int);
    ((JoinServerFn)func)(pAPI, serverID, nullptr, 10);
}

// ---------------------------------------------------------------------------
// Character select via game API
// ---------------------------------------------------------------------------

void SelectCharacterViaAPI(void* charScreen, int index)
{
    if (!charScreen) return;
    uintptr_t func = FixEQGameOffset(CCharacterListWnd__SelectCharacter_x);
    if (!func) return;
    typedef int(__fastcall* SelectCharFn)(void*, int, bool, bool);
    ConsolePrintf("[Overlay][AutoLogin] SelectCharacter: screen=0x%llX func=0x%llX index=%d\n",
        (unsigned long long)(uintptr_t)charScreen, (unsigned long long)func, index);
    ((SelectCharFn)func)(charScreen, index, true, false);
}

void EnterWorldViaAPI(void* charScreen)
{
    if (!charScreen) return;
    uintptr_t func = FixEQGameOffset(CCharacterListWnd__EnterWorld_x);
    if (!func) return;

    bool funcIsExecutable = false;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)func, &mbi, sizeof(mbi)) && mbi.State == MEM_COMMIT
        && (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
        funcIsExecutable = true;

    if (!funcIsExecutable)
    {
        ConsolePrintf("[Overlay][AutoLogin] EnterWorld: func=0x%llX is not executable\n",
            (unsigned long long)func);
        return;
    }

    ConsolePrintf("[Overlay][AutoLogin] EnterWorld: screen=0x%llX func=0x%llX\n",
        (unsigned long long)(uintptr_t)charScreen, (unsigned long long)func);
    typedef int(__fastcall* EnterWorldFn)(void*);
    ((EnterWorldFn)func)(charScreen);
}

// ---------------------------------------------------------------------------
// Splash screen / EULA helpers
// ---------------------------------------------------------------------------

bool IsWindowVisible(const char* screenName)
{
    void* wnd = GetTopLevelWindow(screenName);
    return wnd && IsWindowVisible(wnd);
}

void ClickWindowButton(const char* screenName, const char* buttonName)
{
    void* wnd = GetTopLevelWindow(screenName);
    if (!wnd) return;
    void* btn = GetChildWindow(wnd, buttonName);
    if (btn) ClickButton(btn);
}

// Use CLoginViewManager::HandleLButtonUp to click through splash
void HandleSplashClick()
{
    if (!g_realEQMainBase) return;

    void* pViewMgr = *(void**)FixEQMainOffset(EQMain__pinstCLoginViewManager_x);
    if (!pViewMgr) return;

    uintptr_t func = FixEQMainOffset(EQMain__CLoginViewManager__HandleLButtonUp_x);
    if (!func) return;

    // The function takes CXPoint (two ints: x=1, y=1)
    struct CXPoint { int x, y; };
    CXPoint pt = { 1, 1 };

    typedef void(__fastcall* HandleLButtonUpFn)(void*, CXPoint*);
    ((HandleLButtonUpFn)func)(pViewMgr, &pt);
}