// ------------------------------------------------------------------------------------------------
// Includes
// ------------------------------------------------------------------------------------------------

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "AutoLogin.h"
#include "EQGameInterface.h"
#include "EQTypes.h"
#include "EQGameOffsets.h"
#include "EncryptionHelper.h"
#include "../LogConsole.h"

// ------------------------------------------------------------------------------------------------
// Globals / Statics
// ------------------------------------------------------------------------------------------------

static bool s_Debug_TrackStateChanges = true;

// ------------------------------------------------------------------------------------------------
// Code
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
AutoLogin::AutoLogin() = default;

// ------------------------------------------------------------------------------------------------
AutoLogin::~AutoLogin()
{ 
    Stop(); 
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::LoadCredentialsFromCommandLine()
{
    m_isValid = false;

    auto cmdLine = GetCommandLineA();

    const char* prefix = "/autologin:";

    const char* pos = strstr(cmdLine, prefix);

    if (!pos)
        return false;

    pos += strlen(prefix);

    std::string encrypted(pos);

    std::string decrypted;

    if (!UnprotectString(encrypted, decrypted))
        return false;

    ProfileRecord record;

    char* context = nullptr;
    char* buffer = _strdup(decrypted.c_str());

    if (!buffer)
        return false;

    auto nextToken = [&](char* str = nullptr) -> const char*
        {
            return strtok_s(str, "|", &context);
        };

    if (const char* acct = nextToken(buffer))
        record.accountName = acct;

    if (const char* pass = nextToken())
        record.accountPassword = pass;

    if (const char* svr = nextToken())
        record.serverName = svr;

    if (const char* chr = nextToken())
        record.characterName = chr;

    free(buffer);

    if (record.accountName.empty())
        return false;

    m_isValid = true;
    SetCredentials(record);

    return true;
}

void AutoLogin::SetCredentials(const ProfileRecord& record)
{
    m_profile = record;
}

// ------------------------------------------------------------------------------------------------
const char* AutoLogin::GetPhaseName() const
{
    switch (m_phase)
    {
    case LoginPhase::Idle:              return "Idle";
    case LoginPhase::SplashScreen:      return "SplashScreen";
    case LoginPhase::Connect:           return "Connect";
    case LoginPhase::ConnectConfirm:    return "ConnectConfirm";
    case LoginPhase::ServerSelect:      return "ServerSelect";
    case LoginPhase::ServerSelectConfirm: return "ServerSelectConfirm";
    case LoginPhase::ServerSelectKick:  return "ServerSelectKick";
    case LoginPhase::CharSelect:        return "CharSelect";
    case LoginPhase::CharSelectWait:    return "CharSelectWait";
    case LoginPhase::WaitConnect:       return "WaitConnect";
    case LoginPhase::WaitServerSelect:  return "WaitServerSelect";
    case LoginPhase::InGame:            return "InGame";
    case LoginPhase::Failed:            return "Failed";
    default:                            return "Unknown";
    }
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::IsRunning() const
{
    return m_phase != LoginPhase::Idle &&
        m_phase != LoginPhase::InGame &&
        m_phase != LoginPhase::Failed;
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::IsDone() const
{
    return m_phase == LoginPhase::InGame || m_phase == LoginPhase::Failed;
}

// ------------------------------------------------------------------------------------------------
void AutoLogin::Start()
{
    m_retryCount = 0;
    m_targetServerID = -1;
    m_lastTransitionTime = GetTickCount64();
    m_lastPollTime = 0;
    m_lastGameState = GetGameState();

    EQGameState state = GetGameState();
    if (state == GAMESTATE_INGAME)
    {
        TransitionTo(LoginPhase::InGame);
    }
    else if (state == GAMESTATE_CHARSELECT)
    {
        TransitionTo(LoginPhase::CharSelect);
    }
    else
    {
        TransitionTo(LoginPhase::WaitConnect);
    }
}

// ------------------------------------------------------------------------------------------------
void AutoLogin::Stop()
{
    TransitionTo(LoginPhase::Idle);
}

// ------------------------------------------------------------------------------------------------
void AutoLogin::TransitionTo(LoginPhase newPhase)
{
    if (m_phase == newPhase) return;
    m_phase = newPhase;
    m_lastTransitionTime = GetTickCount64();
    ConsolePrintf("Transitioning to state %s\n", GetPhaseName());
}

// ------------------------------------------------------------------------------------------------
void AutoLogin::RetryConnect()
{
    m_retryCount++;
    if (m_retryCount >= kMaxRetries)
        TransitionTo(LoginPhase::Failed);
    else
        TransitionTo(LoginPhase::WaitConnect);
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::CheckServerSelectScreen()
{
    return IsWindowVisible("serverselect");
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::CheckCharSelectScreen()
{
    return GetGameState() == GAMESTATE_CHARSELECT && GetTopLevelWindow("CLW_CharactersScreen") != nullptr;
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::CheckOkDialog()
{
    return IsOkDialogVisible();
}

// ------------------------------------------------------------------------------------------------
bool AutoLogin::CheckYesNoDialog()
{
    return IsYesNoDialogVisible();
}

// ------------------------------------------------------------------------------------------------
void AutoLogin::HandlePreSelectDialogs()
{
    if (IsWindowVisible("seizurewarning"))
    {
        ClickWindowButton("seizurewarning", "OK_SeizureButton");
        return;
    }

    if (IsWindowVisible("EulaWindow"))
    {
        ClickWindowButton("EulaWindow", "EULA_AcceptButton");
        return;
    }

    if (IsWindowVisible("OrderWindow"))
    {
        ClickWindowButton("OrderWindow", "OD_DeclineButton");
        return;
    }

    if (IsWindowVisible("OrderExpansionWindow"))
    {
        ClickWindowButton("OrderExpansionWindow", "OED_DeclineButton");
        return;
    }

    if (IsWindowVisible("news"))
    {
        ClickWindowButton("news", "NEWS_OKButton");
        return;
    }

    if (IsWindowVisible("soesplash") || IsWindowVisible("dbgsplash"))
    {
        HandleSplashClick();
        return;
    }

    if (GetTopLevelWindow("CLW_CharactersScreen") && IsWindowVisible("okdialog"))
    {
        std::string text = GetOkDialogText();
        if (text.find("Loading") != std::string::npos ||
            text.find("loading") != std::string::npos)
            return;
    }

    if (GetTopLevelWindow("CLW_CharactersScreen") && IsWindowVisible("ConfirmationDialogBox"))
    {
        ClickWindowButton("ConfirmationDialogBox", "CD_Yes_Button");
        return;
    }
}

// ------------------------------------------------------------------------------------------------
static void HandleSplashDialogs()
{
    if (IsWindowVisible("seizurewarning"))
    {
        ClickWindowButton("seizurewarning", "OK_SeizureButton");
        return;
    }
    if (IsWindowVisible("EulaWindow"))
    {
        ClickWindowButton("EulaWindow", "EULA_AcceptButton");
        return;
    }
    if (IsWindowVisible("OrderWindow"))
    {
        ClickWindowButton("OrderWindow", "OD_DeclineButton");
        return;
    }
    if (IsWindowVisible("OrderExpansionWindow"))
    {
        ClickWindowButton("OrderExpansionWindow", "OED_DeclineButton");
        return;
    }
    if (IsWindowVisible("news"))
    {
        ClickWindowButton("news", "NEWS_OKButton");
        return;
    }
    if (IsWindowVisible("soesplash") || IsWindowVisible("dbgsplash"))
    {
        HandleSplashClick();
        return;
    }
}


// ------------------------------------------------------------------------------------------------
void AutoLogin::Update()
{
    InitEQGameBase();
    InitEQMainBase();

    if (m_phase == LoginPhase::Idle || m_phase == LoginPhase::InGame || m_phase == LoginPhase::Failed)
    {
        if (s_Debug_TrackStateChanges)
        {
            EQGameState current = GetGameState();
            if (current != m_lastGameState)
            {
                ConsolePrintf("[Overlay][AutoLogin] GameState transition: %s -> %s\n",
                    GetGameStateName(m_lastGameState), GetGameStateName(current));
                m_lastGameState = current;
            }
        }
        return;
    }

    EQGameState current = GetGameState();
    if (s_Debug_TrackStateChanges && current != m_lastGameState)
    {
        ConsolePrintf("[Overlay][AutoLogin] GameState transition: %s -> %s\n",
            GetGameStateName(m_lastGameState), GetGameStateName(current));
        m_lastGameState = current;
    }

    switch (m_phase)
    {
    case LoginPhase::SplashScreen:      OnSplashScreen();        break;
    case LoginPhase::Connect:           OnConnect();             break;
    case LoginPhase::ConnectConfirm:    OnConnectConfirm();      break;
    case LoginPhase::ServerSelect:      OnServerSelect();        break;
    case LoginPhase::ServerSelectConfirm: OnServerSelectConfirm(); break;
    case LoginPhase::ServerSelectKick:  OnServerSelectKick();    break;
    case LoginPhase::CharSelect:        OnCharSelect();          break;
    case LoginPhase::CharSelectWait:    PollCharSelectWait();    break;
    case LoginPhase::WaitConnect:       PollWaitConnect();       break;
    case LoginPhase::WaitServerSelect:  PollWaitServerSelect();   break;
    default: break;
    }
}

// ---------------------------------------------------------------------------
// Entry: SplashScreen (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnSplashScreen()
{
    HandlePreSelectDialogs();
    TransitionTo(LoginPhase::WaitConnect);
}

// ---------------------------------------------------------------------------
// Entry: Connect - fill credentials and click LOGIN (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnConnect()
{
    // The CXWndManager IS initialized by now (the phase transitioned to Connect
    // because GetTopLevelWindow found the "connect" window). Find the UI elements
    // and interact with them directly.
    void* pConnect = GetTopLevelWindow("connect");
    void* pLoginBtn = pConnect ? GetChildWindow(pConnect, "LOGIN") : nullptr;
    if (!pConnect || !pLoginBtn)
    {
        TransitionTo(LoginPhase::WaitConnect);
        return;
    }

    // Find edit fields as children of the "connect" window
    void* pAccount = FindChildEditField(pConnect, 0);
    void* pPassword = FindChildEditField(pConnect, 1);

    // Fallback: try top-level edit windows
    if (!pAccount)
    {
        pAccount = FindEditField(0);
        pPassword = FindEditField(1);
    }

    if (!pAccount)
    {
        ConsolePrintf("[Overlay][AutoLogin] Connect: no edit fields found\n");
        TransitionTo(LoginPhase::WaitConnect);
        return;
    }

    // Write directly to InputText at eqmain offset 0x278
    SetEditTextDirect(pAccount, m_profile.accountName.c_str());
    if (pPassword && !m_profile.accountPassword.empty())
        SetEditTextDirect(pPassword, m_profile.accountPassword.c_str());

    ConsolePrintf("[Overlay][AutoLogin] Clicking LOGIN\n");
    ClickButton(pLoginBtn);
    TransitionTo(LoginPhase::WaitConnect);
}

// ---------------------------------------------------------------------------
// Entry: ConnectConfirm - read okdialog, retry or stop (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnConnectConfirm()
{
    if (!CheckOkDialog())
    {
        TransitionTo(LoginPhase::WaitConnect);
        return;
    }

    std::string text = GetOkDialogText();

    if (text.empty() ||
        text.find("Logging in") != std::string::npos ||
        text.find("Please wait") != std::string::npos)
    {
        TransitionTo(LoginPhase::WaitConnect);
        return;
    }

    if (text.find("Invalid") != std::string::npos ||
        text.find("invalid") != std::string::npos ||
        text.find("failed") != std::string::npos ||
        text.find("Failed") != std::string::npos ||
        text.find("unable") != std::string::npos ||
        text.find("Unable") != std::string::npos ||
        text.find("timeout") != std::string::npos ||
        text.find("Timeout") != std::string::npos)
    {
        ClickOkButton();
        ConsolePrintf("[Overlay][AutoLogin] Connect error, retry %d/%d\n", m_retryCount + 1, kMaxRetries);
        RetryConnect();
        return;
    }

    ClickOkButton();
    RetryConnect();
}

// ---------------------------------------------------------------------------
// Entry: ServerSelect - find server and JoinServer (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnServerSelect()
{
    if (!CheckServerSelectScreen())
    {
        if (CheckCharSelectScreen()) { TransitionTo(LoginPhase::CharSelect); return; }
        if (CheckOkDialog()) { TransitionTo(LoginPhase::ServerSelectConfirm); return; }
        if (CheckYesNoDialog()) { TransitionTo(LoginPhase::ServerSelectKick); return; }
        if (GetGameState() == GAMESTATE_INGAME) { TransitionTo(LoginPhase::InGame); return; }
        uint64_t elapsed = GetTickCount64() - m_lastTransitionTime;
        if (elapsed > kConnectTimeout) { TransitionTo(LoginPhase::Failed); return; }
        TransitionTo(LoginPhase::WaitServerSelect);
        return;
    }

    if (m_profile.serverName.empty())
    {
        ConsolePrintf("[Overlay][AutoLogin] ServerSelect: server name is empty, stopping\n");
        TransitionTo(LoginPhase::Failed);
        return;
    }

    int serverID = -1;
    int statusFlags = 0;

    void* pServer = FindServerByName(m_profile.serverName.c_str());
    if (pServer)
    {
        serverID = ((EQClientServerData*)pServer)->ID;
        statusFlags = GetServerStatusFlags(pServer);
        ConsolePrintf("[Overlay][AutoLogin] ServerSelect: found \"%s\" via LoginClient ID=%d flags=0x%X\n",
            m_profile.serverName.c_str(), serverID, statusFlags);
    }
    else
    {
        serverID = FindServerIDByNameViaUI(m_profile.serverName.c_str());
        if (serverID > 0)
            ConsolePrintf("[Overlay][AutoLogin] ServerSelect: found \"%s\" via UI, ID=%d\n",
                m_profile.serverName.c_str(), serverID);
    }

    if (serverID <= 0)
    {
        TransitionTo(LoginPhase::WaitServerSelect);
        return;
    }

    if (statusFlags & (eServerStatus_Down | eServerStatus_Locked))
    {
        ConsolePrintf("[Overlay][AutoLogin] ServerSelect: server is down/locked (0x%X), waiting\n", statusFlags);
        TransitionTo(LoginPhase::WaitServerSelect);
        return;
    }

    ConsolePrintf("[Overlay][AutoLogin] ServerSelect: joining server ID %d\n", serverID);
    JoinServer(serverID);
    m_targetServerID = serverID;
    TransitionTo(LoginPhase::WaitServerSelect);
}

// ---------------------------------------------------------------------------
// Entry: ServerSelectConfirm - read okdialog after JoinServer (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnServerSelectConfirm()
{
    if (!CheckOkDialog())
    {
        if (CheckCharSelectScreen()) { TransitionTo(LoginPhase::CharSelect); return; }
        if (GetGameState() == GAMESTATE_INGAME) { TransitionTo(LoginPhase::InGame); return; }
        if (CheckServerSelectScreen()) { m_targetServerID = -1; TransitionTo(LoginPhase::ServerSelect); return; }
        TransitionTo(LoginPhase::WaitServerSelect);
        return;
    }

    std::string text = GetOkDialogText();

    if (text.find("maximum capacity") != std::string::npos ||
        text.find("full") != std::string::npos)
    {
        TransitionTo(LoginPhase::WaitServerSelect);
        return;
    }

    if (text.find("not a free-play") != std::string::npos ||
        text.find("denied") != std::string::npos ||
        text.find("cannot") != std::string::npos)
    {
        ClickOkButton();
        TransitionTo(LoginPhase::Failed);
        return;
    }

    ClickOkButton();
    m_targetServerID = -1;
    TransitionTo(LoginPhase::ServerSelect);
}

// ---------------------------------------------------------------------------
// Entry: ServerSelectKick - handle "already logged in" dialog (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnServerSelectKick()
{
    if (!CheckYesNoDialog())
    {
        TransitionTo(LoginPhase::WaitConnect);
        return;
    }

    std::string text = GetYesNoDialogText();

    if (text.find("already have a character") != std::string::npos ||
        text.find("OFFLINE TRADER") != std::string::npos)
    {
        ClickYesButton();
        TransitionTo(LoginPhase::WaitConnect);
    }
    else
    {
        ClickNoButton();
        TransitionTo(LoginPhase::WaitConnect);
    }
}

// ---------------------------------------------------------------------------
// Entry: CharSelect - find target character and select it (one-shot)
// ---------------------------------------------------------------------------

void AutoLogin::OnCharSelect()
{
    // Safety: bail if EQ window manager isn't ready
    if (!g_realEQMainBase && !g_realEQGameBase)
    {
        TransitionTo(LoginPhase::CharSelect);
        return;
    }

    // Handle confirmation dialogs that appear on character select
    if (IsWindowVisible("ConfirmationDialogBox"))
    {
        ClickWindowButton("ConfirmationDialogBox", "CD_Yes_Button");
        return;
    }

    if (IsWindowVisible("okdialog"))
    {
        std::string text = GetOkDialogText();
        if (text.find("Loading") != std::string::npos ||
            text.find("loading") != std::string::npos)
        {
            TransitionTo(LoginPhase::CharSelect);
            return;
        }
        ClickOkButton();
        TransitionTo(LoginPhase::CharSelect);
        return;
    }

    if (m_profile.characterName.empty())
    {
        ConsolePrintf("[Overlay][AutoLogin] CharSelect: character name is empty, stopping\n");
        TransitionTo(LoginPhase::Failed);
        return;
    }

    // Find the character screen — MUST use "CharacterListWnd" SidlText, NOT
    // "CLW_CharactersScreen". "CharacterListWnd" is the actual CCharacterListWnd
    // object (matches MQ2's pCharacterListWnd ForeignPointer).
    void* pCharScreen = GetTopLevelWindow("CharacterListWnd");
    if (!pCharScreen)
    {
        TransitionTo(LoginPhase::CharSelect);
        return;
    }

    // Find the character list
    void* pCharList = GetChildWindowTyped(pCharScreen, "Character_List");
    if (!pCharList)
    {
        ConsolePrintf("[Overlay][AutoLogin] CharSelect: Character_List not found\n");
        TransitionTo(LoginPhase::CharSelect);
        return;
    }

    int itemCount = GetListItemCount(pCharList);
    ConsolePrintf("[Overlay][AutoLogin] CharSelect: Found %d character(s)\n", itemCount);

    int targetIndex = -1;
    for (int i = 0; i < itemCount; i++)
    {
        std::string charName = GetListItemText(pCharList, i, 2);
        ConsolePrintf("[Overlay][AutoLogin] CharSelect: [%d] %s\n", i, charName.c_str());

        if (targetIndex < 0 && _stricmp(charName.c_str(), m_profile.characterName.c_str()) == 0)
        {
            targetIndex = i;
        }
    }

    if (targetIndex < 0)
    {
        ConsolePrintf("[Overlay][AutoLogin] CharSelect: character \"%s\" not found, stopping\n",
            m_profile.characterName.c_str());
        TransitionTo(LoginPhase::Failed);
        return;
    }

    ConsolePrintf("[Overlay][AutoLogin] CharSelect: selecting character \"%s\" at index %d\n",
        m_profile.characterName.c_str(), targetIndex);

    // Call CCharacterListWnd::SelectCharacter via the game API (NOT the
    // Character_List child — the CCharacterListWnd itself sets internal state)
    SelectCharacterViaAPI(pCharScreen, targetIndex);
    TransitionTo(LoginPhase::CharSelectWait);
}

// ---------------------------------------------------------------------------
// Wait state: CharSelectWait - wait then enter world
// ---------------------------------------------------------------------------

void AutoLogin::PollCharSelectWait()
{
    uint64_t now = GetTickCount64();
    uint64_t elapsed = now - m_lastTransitionTime;

    // If game state changed to ingame, we're done
    if (GetGameState() == GAMESTATE_INGAME)
    {
        ConsolePrintf("[Overlay][AutoLogin] CharSelectWait -> InGame\n");
        TransitionTo(LoginPhase::InGame);
        return;
    }

    // If we got kicked back to character select, re-select
    if (GetGameState() == GAMESTATE_CHARSELECT && elapsed > kCharSelectDelay / 2)
    {
        // Re-verify we're still on char select and retry
        void* pCharScreen = GetTopLevelWindow("CharacterListWnd");
        if (pCharScreen)
        {
            void* pCharList = GetChildWindowTyped(pCharScreen, "Character_List");
            if (pCharList)
            {
                int curSel = GetListCurSel(pCharList);
                if (curSel >= 0)
                {
                    std::string curName = GetListItemText(pCharList, curSel, 2);
                    if (_stricmp(curName.c_str(), m_profile.characterName.c_str()) == 0)
                    {
                        // Selection is correct, proceed to enter world
                    }
                    else
                    {
                        // Wrong selection, re-select
                        ConsolePrintf("[Overlay][AutoLogin] CharSelectWait: wrong selection, re-selecting\n");
                        OnCharSelect();
                        return;
                    }
                }
            }
        }
    }

    // Wait for the delay then enter world via the game's CCharacterListWnd::EnterWorld
    if (elapsed >= kCharSelectDelay)
    {
        void* pCharScreen = GetTopLevelWindow("CharacterListWnd");
        if (!pCharScreen)
        {
            ConsolePrintf("[Overlay][AutoLogin] CharSelectWait: CharacterListWnd not found\n");
            TransitionTo(LoginPhase::Failed);
            return;
        }

        // Verify the correct character is still selected before entering
        void* pCharList = GetChildWindowTyped(pCharScreen, "Character_List");
        if (pCharList)
        {
            int curSel = GetListCurSel(pCharList);
            if (curSel >= 0)
            {
                std::string curName = GetListItemText(pCharList, curSel, 2);
                if (_stricmp(curName.c_str(), m_profile.characterName.c_str()) != 0)
                {
                    ConsolePrintf("[Overlay][AutoLogin] CharSelectWait: wrong character selected, re-selecting\n");
                    OnCharSelect();
                    return;
                }
            }
        }

        ConsolePrintf("[Overlay][AutoLogin] CharSelectWait: entering world via API screen=0x%p func=0x%p\n",
            pCharScreen, (void*)(uintptr_t)CCharacterListWnd__EnterWorld_x);
        EnterWorldViaAPI(pCharScreen);

        // Don't transition immediately; let the next pulse detect the new game state
        TransitionTo(LoginPhase::WaitConnect);
        return;
    }

    // Handle splash/EULA dialogs that may appear
    HandlePreSelectDialogs();

    // Timeout if we've been waiting too long
    if (elapsed > kConnectTimeout)
    {
        ConsolePrintf("[Overlay][AutoLogin] CharSelectWait: timeout\n");
        TransitionTo(LoginPhase::Failed);
    }
}

// ---------------------------------------------------------------------------
// Wait state: WaitConnect - poll for next state after LOGIN click
// ---------------------------------------------------------------------------

void AutoLogin::PollWaitConnect()
{
    uint64_t now = GetTickCount64();
    if (now - m_lastPollTime < kPollDelay)
        return;
    m_lastPollTime = now;

    uint64_t elapsed = now - m_lastTransitionTime;

    if (GetGameState() == GAMESTATE_INGAME)
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect -> InGame\n");
        TransitionTo(LoginPhase::InGame);
        return;
    }

    if (CheckCharSelectScreen())
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect -> CharSelect\n");
        TransitionTo(LoginPhase::CharSelect);
        return;
    }

    if (CheckOkDialog())
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect -> ConnectConfirm\n");
        TransitionTo(LoginPhase::ConnectConfirm);
        return;
    }

    if (CheckYesNoDialog())
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect -> ServerSelectKick\n");
        TransitionTo(LoginPhase::ServerSelectKick);
        return;
    }

    if (CheckServerSelectScreen())
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect -> ServerSelect\n");
        TransitionTo(LoginPhase::ServerSelect);
        return;
    }

    void* pConnect = GetTopLevelWindow("connect");
    if (pConnect && IsWindowVisible(pConnect))
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect -> Connect\n");
        TransitionTo(LoginPhase::Connect);
        return;
    }

    // Handle splash/EULA/order windows (only checked in wait states, not per-frame)
    HandleSplashDialogs();

    if (elapsed > kConnectTimeout)
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitConnect: timeout\n");
        TransitionTo(LoginPhase::Failed);
    }
}

// ---------------------------------------------------------------------------
// Wait state: WaitServerSelect - poll for next state after JoinServer
// Only checks game state (no window iteration to avoid transition crash)
// ---------------------------------------------------------------------------

void AutoLogin::PollWaitServerSelect()
{
    uint64_t now = GetTickCount64();
    uint64_t elapsed = now - m_lastTransitionTime;

    // Use GetGameState() only - no window iteration during transition
    if (GetGameState() == GAMESTATE_CHARSELECT)
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitServerSelect -> CharSelect\n");
        TransitionTo(LoginPhase::CharSelect);
        return;
    }

    if (GetGameState() == GAMESTATE_INGAME)
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitServerSelect -> InGame\n");
        TransitionTo(LoginPhase::InGame);
        return;
    }

    // Longer delay for this state to avoid checking during transition
    if (now - m_lastPollTime < kPollDelay)
        return;
    m_lastPollTime = now;

    if (elapsed > kConnectTimeout)
    {
        ConsolePrintf("[Overlay][AutoLogin] WaitServerSelect: timeout\n");
        TransitionTo(LoginPhase::Failed);
    }
}
