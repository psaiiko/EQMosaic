#pragma once
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <atomic>

// ---------------------------------------------------------------------------
// CStrRep: the ref-counted string body that CXStr points to (24-byte header)
// ---------------------------------------------------------------------------

struct CStrRep
{
/*0x00*/ std::atomic<int> refCount;   // 4 bytes
/*0x04*/ uint32_t         alloc;      // 4 bytes
/*0x08*/ uint32_t         length;     // 4 bytes
/*0x0c*/ uint32_t         encoding;   // 4 bytes (EStringEncoding)
/*0x10*/ void*            freeList;   // 8 bytes (CXFreeList*)
/*0x18*/ char             utf8[1];    // variable-length string data
};

// ---------------------------------------------------------------------------
// EStringEncoding: CXStr encoding type
// ---------------------------------------------------------------------------

enum EStringEncoding : uint32_t
{
    StringEncodingUtf8  = 0,
    StringEncodingUtf16 = 1,
};

// ---------------------------------------------------------------------------
// CXStr: read-only wrapper matching EQ's 8-byte (CStrRep* handle) layout
// ---------------------------------------------------------------------------

class CXStr
{
public:
    CXStr() : m_data(nullptr) {}

    // Read-only accessors — dereference through CStrRep handle
    const char* c_str() const
    {
        if (!m_data) return "";
        return m_data->utf8;
    }

    bool empty() const
    {
        return !m_data || m_data->length == 0;
    }

    int length() const
    {
        return m_data ? (int)m_data->length : 0;
    }

    CStrRep* m_data;  // 0x00, 8 bytes
};

static_assert(sizeof(CXStr) == 8, "CXStr must be 8 bytes");

// ---------------------------------------------------------------------------
// ArrayClass<T>: template matching EQ's array container
// Inherits from CDynamicArrayBase (int m_length, 4b pad) then:
//   +0x08: T*   m_array
//   +0x10: int  m_alloc
//   +0x14: bool m_isValid
// ---------------------------------------------------------------------------

template<typename T>
struct ArrayClass
{
    int   Count;      // 0x00: CDynamicArrayBase::m_length
    int   _pad;       // 0x04: padding for 8-byte alignment
    T*    Data;       // 0x08: m_array
    int   Allocated;  // 0x10: m_alloc
    bool  IsValid;    // 0x14: m_isValid

    T& operator[](int i) { return Data[i]; }
    const T& operator[](int i) const { return Data[i]; }
};

// ---------------------------------------------------------------------------
// SListWndCell: a single cell in a list line
// ---------------------------------------------------------------------------

struct SListWndCell
{
    void* pTA;       // 0x00 CTextureAnimation*
    CXStr Text;      // 0x08
    uint32_t Color;  // 0x10 COLORREF
    uint8_t _pad1[4];// 0x14 (bOnlyDrawTexture bool + 3 bytes padding)
    void* pWnd;      // 0x18
    void* Unknown1;  // 0x20
}; // sizeof = 0x28 (must match game layout for correct array indexing)

static_assert(sizeof(SListWndCell) == 0x28, "SListWndCell must be 0x28 bytes");

// ---------------------------------------------------------------------------
// SListWndLine: a single row in a CListWnd
// ---------------------------------------------------------------------------

struct STreeData
{
    int  Depth;          // 0x00
    bool bIsExpandable;  // 0x04
    uint8_t _pad2[3];    // 0x05
}; // sizeof = 0x08

struct SListWndLine
{
    ArrayClass<SListWndCell> Cells;    // 0x000
    uint64_t                 Data;     // 0x018 (server ID or EQClientServerData*)
    int                      Height;   // 0x020
    bool                     bSelected;// 0x024
    bool                     bEnabled; // 0x025
    uint8_t _pad3[2];                 // 0x026
    STreeData                Treedata; // 0x028
    char                     TooltipText[256]; // 0x030
    bool                     bVisible; // 0x130
    uint8_t _pad4[3];                 // 0x131
    uint32_t                 bIsSeparator; // 0x134
}; // sizeof = 0x138 (must match game layout for correct array indexing)

static_assert(sizeof(SListWndLine) == 0x138, "SListWndLine must be 0x138 bytes");

// ---------------------------------------------------------------------------
// Doubly linked list node and list (generic, for server list traversal)
// ---------------------------------------------------------------------------

template<typename T>
struct LinkedListNode
{
    T                m_object; // 0x00 (inline T)
    LinkedListNode*  m_pNext;  // 0x08
    LinkedListNode*  m_pPrev;  // 0x10
};

template<typename T>
struct DoublyLinkedList
{
    void*             vtable;           // 0x00 (virtual destructor)
    LinkedListNode<T>* m_pHead;         // 0x08
    LinkedListNode<T>* m_pTail;         // 0x10
    LinkedListNode<T>* m_pCurObject;    // 0x18
    LinkedListNode<T>* m_pCurObjectNext; // 0x20
    LinkedListNode<T>* m_pCurObjectPrev; // 0x28
    int                m_numObjects;    // 0x30
    int                m_refCount;      // 0x34
};

// ---------------------------------------------------------------------------
// EQClientServerData: server entry (in the ServerList DoublyLinkedList)
// Reference: eqlib/game/LoginFrontend.h : 96
// ---------------------------------------------------------------------------

struct EQClientServerData
{
    int         ID;              // 0x00 (ServerID is int)
    CXStr       ServerName;      // 0x08
    CXStr       HostName;        // 0x10
    CXStr       ServerIP;        // 0x18
    int         ExternalPort;    // 0x20
    int         InternalPort;    // 0x24
    // Date  DateCreated at 0x28 (struct, 0x28 bytes = 0x28..0x4F)
    // Flags at 0x50
    int         StatusFlags;     // 0x78 (eServerStatus enum)
    int         PopulationRanking; // 0x7C
    int         Expansion;       // 0x80
    int         TrueBoxStatus;   // 0x84
};

// ---------------------------------------------------------------------------
// UIType enum — returned by CXWnd::GetType() via CXMLDataManager::GetWindowType
// Reference: eqlib/game/UIBase.h
// ---------------------------------------------------------------------------

enum UIType : int
{
    UIT_UNKNOWN          = 0,
    UIT_WND              = 1,
    UIT_BUTTON           = 2,
    UIT_CHECKBOX         = 3,
    UIT_RADIOBUTTON      = 4,
    UIT_SLIDER           = 5,
    UIT_EDIT             = 6,
    UIT_LIST             = 7,
    UIT_COMBO            = 8,
    UIT_SCROLL           = 9,
    UIT_STML             = 10,
    UIT_STATIC           = 11,
    UIT_TABBOX           = 12,
    UIT_TOOLTIP          = 13,
    UIT_SPINNER          = 14,
    UIT_TITLEBAR         = 15,
    UIT_LINK              = 16,
    UIT_CUSTOM            = 100,
};

// ---------------------------------------------------------------------------
// EWndRuntimeType enum — checked by CXWnd::IsType(EWndRuntimeType)
// Reference: eqlib/include/eqlib/game/CXWnd.h : 159
// These are CONSECUTIVE runtime class IDs, NOT arbitrary enum values.
// ---------------------------------------------------------------------------

enum EWndRuntimeType : int
{
    WRT_WND            = 0,
    WRT_LISTWND        = 1,
    WRT_EDITWND        = 2,   // CEditBaseWnd
    WRT_TREEWND        = 3,
    WRT_PAGEWND        = 4,
    WRT_TABWND         = 5,
    WRT_HOTKEYWND      = 6,
    WRT_EDITHOTKEYWND  = 7,
    WRT_RANGESLIDERWND = 8,
    WRT_STMLWND        = 9,
    WRT_BROWSERWND     = 10,
    WRT_MODALMESSAGEWND = 11,
    WRT_CHECKBOXWND    = 12,
    WRT_SIDLSCREENWND  = 13,
    WRT_SLIDERWND      = 14,
    WRT_LABEL          = 15,
    WRT_BUTTON         = 16,
    WRT_GAUGE          = 17,
    WRT_COMBOBOX       = 18,
    WRT_CHATWND        = 19,
    WRT_HELPWND        = 20,
};

// ---------------------------------------------------------------------------
// Window notification message constants (from MQ2AutoLogin.h)
// ---------------------------------------------------------------------------

enum : uint32_t
{
    XWM_LCLICK      = 1,
    XWM_RCLICK      = 2,
    XWM_LMOUSEDOWN  = 3,
    XWM_LMOUSEUP    = 4,
    XWM_RMOUSEDOWN  = 5,
    XWM_RMOUSEUP    = 6,
    XWM_CHECKED     = 22,
    XWM_UNCHECKED   = 23,
    XWM_CLOSE       = 48,
    XWM_NEWVALUE    = 60,
    XWM_LIST_SELECT = 66,
};

// ---------------------------------------------------------------------------
// Server status enum
// ---------------------------------------------------------------------------

enum eServerStatus : uint32_t
{
    eServerStatus_Up             = 0x00,
    eServerStatus_Down           = 0x01,
    eServerStatus_Locked         = 0x04,
    eServerStatus_PopulationLow  = 0x10,
    eServerStatus_PopulationHigh = 0x20,
};

// ---------------------------------------------------------------------------
// CXWnd offsets — eqmain, reference: LoginFrontend.h : 270
// Inheritance: CXWnd : TListNode (m_pPrev, m_pNext, m_pList), TList (m_pFirstNode)
// Layout: [vfptr(8) | TListNode(24) | TList(16)] -> CXWnd members at 0x30
// ---------------------------------------------------------------------------

struct CXWndOffsets
{
    static constexpr uintptr_t FirstChild  = 0x20;  // TList::m_pFirstNode
    static constexpr uintptr_t NextSibling = 0x10;  // TListNode::m_pNext
    static constexpr uintptr_t PrevSibling = 0x08;  // TListNode::m_pPrev
    static constexpr uintptr_t dShow       = 0x078; // bool (IsVisible)
    static constexpr uintptr_t Enabled     = 0x168; // bool
    static constexpr uintptr_t WindowText  = 0x158; // CXStr
    static constexpr uintptr_t DataStr     = 0x1F8; // CXStr (SIDL piece name)
    static constexpr uintptr_t ParentWindow = 0x240; // CXWnd*
    // SidlText: CSidlScreenWnd member — offset depends on which module created the window:
    //   eqmain.dll (CXWnd=0x260): 0x268
    //   eqgame.exe (CXWnd=0x268): 0x270
    // Both are used in GetWindowScreenName / GetTopLevelWindow.
    static constexpr uintptr_t SidlTextEqmain = 0x268;
    static constexpr uintptr_t SidlTextEqgame = 0x270;
    // Rect coords for DumpWindows
    static constexpr uintptr_t RectLeft   = 0x124; // int
    static constexpr uintptr_t RectTop    = 0x128; // int
    static constexpr uintptr_t RectRight  = 0x12C; // int
    static constexpr uintptr_t RectBottom = 0x130; // int
};

// CXWndManager offsets
struct CXWndManagerOffsets
{
    static constexpr uintptr_t WindowsArray = 0x008; // ArrayClass<CXWnd*> (count at +0x00, data at +0x08)
    static constexpr uintptr_t MainWindow   = 0x088; // CXWnd*
};

// ---------------------------------------------------------------------------
// CXWnd vtable indices — eqmain, reference: LoginFrontend.h : 577
// NOTE: IsType and GetType are NOT virtual functions — do NOT add them here.
// ---------------------------------------------------------------------------

struct CXWndVtable
{
    static constexpr uintptr_t WndNotification = 0x110; // int(__fastcall)(void*,void*,uint32_t,void*)
    static constexpr uintptr_t Show            = 0x1B0;
    static constexpr uintptr_t SetWindowText   = 0x250;
    static constexpr uintptr_t GetWindowName   = 0x268;
};

// ---------------------------------------------------------------------------
// CEditBaseWnd : CXWnd — eqmain, reference: LoginFrontend.h : 791
// ---------------------------------------------------------------------------

struct CEditBaseWndOffsets
{
    // eqmain CEditBaseWnd (CXWnd=0x260): InputText at 0x278
    // (was 0x280 when eqmain CXWnd was 0x268)
    static constexpr uintptr_t InputText = 0x278; // CXStr
};

// CStmlWnd STMLText: eqgame (UI.h) at +0x268, eqmain (LoginFrontend.h) at +0x248
struct CStmlWndOffsets
{
    static constexpr uintptr_t STMLTextEqgame = 0x268; // CXStr
    static constexpr uintptr_t STMLTextEqmain = 0x248; // CXStr
};

// ---------------------------------------------------------------------------
// CListWnd : CXWnd — eqmain, reference: LoginFrontend.h : 812
// ---------------------------------------------------------------------------

struct CListWndOffsets
{
    // Game-side CListWnd (eqgame.exe CXWnd = 0x268): ItemsArray at 0x270
    // Login CListWnd (eqmain.dll CXWnd = 0x260): ItemsArray at 0x268
    // Use game-side offsets since character select is a game screen.
    static constexpr uintptr_t ItemsArray = 0x270; // ArrayClass<SListWndLine>
    static constexpr uintptr_t Columns    = 0x288; // ArrayClass<SListWndColumn>
    static constexpr uintptr_t CurSel     = 0x2A0; // int
    static constexpr uintptr_t CurCol     = 0x2A4; // int
};

// ---------------------------------------------------------------------------
// LoginClient offsets
// ---------------------------------------------------------------------------

struct LoginClientOffsets
{
    static constexpr uintptr_t LoginName    = 0x130; // CXStr
    static constexpr uintptr_t Password     = 0x138; // CXStr
    static constexpr uintptr_t LoginNameCopy = 0x140; // CXStr (copy for change detection)
    static constexpr uintptr_t PasswordCopy  = 0x148; // CXStr
    static constexpr uintptr_t IsLoggingIn  = 0x16c; // bool
    static constexpr uintptr_t ServerList   = 0x178; // DoublyLinkedList<EQClientServerData*>
};

// ---------------------------------------------------------------------------
// CEverQuest game state offset (eqgame)
// ---------------------------------------------------------------------------

enum EQGameState : int
{
    GAMESTATE_UNKNOWN          = 0,
    GAMESTATE_PRECHARSELECT    = -1,
    GAMESTATE_UNKNOWN1         = 0,
    GAMESTATE_CHARSELECT       = 1,
    GAMESTATE_CHARCREATE       = 2,
    GAMESTATE_POSTCHARSELECT   = 3,
    GAMESTATE_SOMETHING        = 4,
    GAMESTATE_INGAME           = 5,
    GAMESTATE_LOADING_WORLD    = 6,
    GAMESTATE_LOGGINGIN        = 253,
    GAMESTATE_UNLOADING        = 255,
};

// ---------------------------------------------------------------------------
// CSidlManager - from openvanilla/eqlib
// ---------------------------------------------------------------------------

struct CSidlManager
{
    void* vtable;                          // 0x00
};

struct CXMLDataManager
{
};

// Forward declarations
struct CXWnd {
    void* vtable;
};

struct CListWnd : CXWnd {
};

struct CEverQuestOffsets
{
    static constexpr uintptr_t GameState = 0x5E4; // int
};
