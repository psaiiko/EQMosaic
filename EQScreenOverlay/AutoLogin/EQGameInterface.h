#pragma once
#include <cstdint>
#include <string>
#include "EQTypes.h"

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void InitEQInterface();
bool IsEQReady();
void DumpWindows();

// Returns false if the running eqgame.exe version doesn't match the compiled-in
// __ExpectedVersionDate / __ExpectedVersionTime. On mismatch shows a MessageBox
// explaining auto-login will be disabled.
bool CheckEQVersion();

// ---------------------------------------------------------------------------
// Game state
// ---------------------------------------------------------------------------

EQGameState GetGameState();
const char* GetGameStateName(EQGameState state);

// ---------------------------------------------------------------------------
// Module base
// ---------------------------------------------------------------------------

uintptr_t GetEQGameBase();
uintptr_t GetEQMainBase();

// ---------------------------------------------------------------------------
// CXWnd helpers
// ---------------------------------------------------------------------------

void* GetTopLevelWindow(const char* screenName);
void* GetChildWindow(void* parentWnd, const char* childName);
void* GetChildWindowTyped(void* parentWnd, const char* childName);
bool  IsWindowVisible(void* wnd);
bool  IsWindowEnabled(void* wnd);
void* GetActiveWindow(const char* screenName);
void* FindEditField(int index);
void* FindChildEditField(void* parentWnd, int index);

// ---------------------------------------------------------------------------
// Edit window
// ---------------------------------------------------------------------------

void SetEditTextDirect(void* editWnd, const char* text);

// ---------------------------------------------------------------------------
// Button clicks
// ---------------------------------------------------------------------------

void ClickButton(void* buttonWnd);
void ClickButtonUp(void* buttonWnd);

// ---------------------------------------------------------------------------
// Dialog helpers
// ---------------------------------------------------------------------------

std::string GetOkDialogText();
std::string GetYesNoDialogText();
bool IsOkDialogVisible();
bool IsYesNoDialogVisible();
void ClickOkButton();
void ClickYesButton();
void ClickNoButton();

// ---------------------------------------------------------------------------
// List window
// ---------------------------------------------------------------------------

int         GetListCurSel(void* listWnd);
void        SetListCurSel(void* listWnd, int sel);
int         GetListItemCount(void* listWnd);
std::string GetListItemText(void* listWnd, int row, int col);

// ---------------------------------------------------------------------------
// LoginClient / Server list
// ---------------------------------------------------------------------------

void* GetLoginClient();
void* FindServerByName(const char* serverName);
int   FindServerIDByName(const char* serverName);
int   FindServerIDByNameViaUI(const char* serverName);
int   GetServerStatusFlags(void* serverData);
void  JoinServer(int serverID);

// ---------------------------------------------------------------------------
// Character select
// ---------------------------------------------------------------------------

// Use game's CCharacterListWnd functions directly (more reliable)
void SelectCharacterViaAPI(void* charScreen, int index);
void EnterWorldViaAPI(void* charScreen);

// ---------------------------------------------------------------------------
// Splash/EULA
// ---------------------------------------------------------------------------

bool IsWindowVisible(const char* screenName);
void ClickWindowButton(const char* screenName, const char* buttonName);
void HandleSplashClick();
