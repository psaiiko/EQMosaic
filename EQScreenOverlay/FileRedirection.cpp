#include <windows.h>
#include <atomic>
#include <string>
#include <string_view>
#include <filesystem>
#include <algorithm>
#include "FileRedirection.h"
#include "LogConsole.h"
#include "MinHook.h"

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Defines
// ---------------------------------------------------------------------------
#define KERNEL32_DLL L"kernel32.dll"

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

static std::atomic<bool> s_redirectEnabled{ false };
static std::wstring      s_redirectSuffixW; // L"_<account>.ini"
static std::string       s_redirectSuffixA; //  "_<account>.ini" (ANSI)

static decltype(&CreateFileW) s_originalCreateFileW = nullptr;
static decltype(&CreateFileA) s_originalCreateFileA = nullptr;

// Profile API function pointers
static decltype(&GetPrivateProfileStringA)   s_originalGetPrivateProfileStringA = nullptr;
static decltype(&GetPrivateProfileStringW)   s_originalGetPrivateProfileStringW = nullptr;
static decltype(&WritePrivateProfileStringA) s_originalWritePrivateProfileStringA = nullptr;
static decltype(&WritePrivateProfileStringW) s_originalWritePrivateProfileStringW = nullptr;
static decltype(&GetPrivateProfileSectionA) s_originalGetPrivateProfileSectionA = nullptr;
static decltype(&GetPrivateProfileSectionW) s_originalGetPrivateProfileSectionW = nullptr;
static decltype(&WritePrivateProfileSectionA) s_originalWritePrivateProfileSectionA = nullptr;
static decltype(&WritePrivateProfileSectionW) s_originalWritePrivateProfileSectionW = nullptr;
static decltype(&GetPrivateProfileIntA)     s_originalGetPrivateProfileIntA = nullptr;
static decltype(&GetPrivateProfileIntW)     s_originalGetPrivateProfileIntW = nullptr;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Generic case-insensitive suffix check
template <typename CharT>
static bool EndsWithCaseInsensitive(std::basic_string_view<CharT> str, std::basic_string_view<CharT> suffix)
{
    if (str.length() < suffix.length())
        return false;

    auto endPart = str.substr(str.length() - suffix.length());
    return std::equal(endPart.begin(), endPart.end(), suffix.begin(), [](CharT a, CharT b) {
        return std::tolower(a, std::locale::classic()) == std::tolower(b, std::locale::classic());
        });
}

static bool IsEqClientIni(const wchar_t* fname)
{
    if (!fname) return false;
    return EndsWithCaseInsensitive<wchar_t>(fs::path(fname).filename().c_str(), L"eqclient.ini");
}

static bool IsEqClientIniA(const char* fname)
{
    if (!fname) return false;
    return EndsWithCaseInsensitive<char>(fs::path(fname).filename().string(), "eqclient.ini");
}

// ---------------------------------------------------------------------------
// Path building
// Returns true if redirected, fills buf with the redirected path.
// Returns false if no redirection needed (buf untouched).
// ---------------------------------------------------------------------------

static bool BuildRedirectedPath(const wchar_t* lpFileName, std::wstring& outPath)
{
    if (!s_redirectEnabled.load(std::memory_order_acquire) || !lpFileName)
        return false;

    fs::path originalPath(lpFileName);
    if (!IsEqClientIni(originalPath.c_str()))
        return false;

    fs::path dir = originalPath.parent_path();
    std::wstring newFilename = L"eqclient" + s_redirectSuffixW;

    outPath = (dir / newFilename).wstring();
    return true;
}

static bool BuildRedirectedPathA(const char* lpFileName, std::string& outPath)
{
    if (!s_redirectEnabled.load(std::memory_order_acquire) || !lpFileName)
        return false;

    fs::path originalPath(lpFileName);
    if (!IsEqClientIniA(originalPath.string().c_str()))
        return false;

    fs::path dir = originalPath.parent_path();
    std::string newFilename = "eqclient" + s_redirectSuffixA;

    outPath = (dir / newFilename).string();
    return true;
}

// ---------------------------------------------------------------------------
// File copy on first access
// ---------------------------------------------------------------------------

static void EnsureRedirectedFile(const wchar_t* original, const wchar_t* redirected)
{
    if (GetFileAttributesW(redirected) != INVALID_FILE_ATTRIBUTES)
        return;
    ConsolePrintf("[Overlay][FileRedirect] Creating %ls from %ls\n", redirected, original);
    if (!CopyFileW(original, redirected, FALSE))
        ConsolePrintf("[Overlay][FileRedirect] CopyFileW failed: %u\n", GetLastError());
}

static void EnsureRedirectedFileA(const char* original, const char* redirected)
{
    std::wstring wOrig = fs::path(original).wstring();
    std::wstring wRedir = fs::path(redirected).wstring();
    EnsureRedirectedFile(wOrig.c_str(), wRedir.c_str());
}

// ---------------------------------------------------------------------------
// CreateFile detours
// ---------------------------------------------------------------------------

static HANDLE WINAPI hkCreateFileW(
    LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    std::wstring redirected;
    if (BuildRedirectedPath(lpFileName, redirected))
    {
        EnsureRedirectedFile(lpFileName, redirected.c_str());
        return s_originalCreateFileW(redirected.c_str(), dwDesiredAccess, dwShareMode,
            lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }

    return s_originalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static HANDLE WINAPI hkCreateFileA(
    LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    return s_originalCreateFileA(lpFileName, dwDesiredAccess, dwShareMode,
        lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

// ---------------------------------------------------------------------------
// Profile API detours
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
static DWORD WINAPI hkGetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault,LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalGetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault,
            lpReturnedString, nSize, lpFileName);

    std::string redirected;
    if (BuildRedirectedPathA(lpFileName, redirected))
    {
        EnsureRedirectedFileA(lpFileName, redirected.c_str());
        return s_originalGetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault,
            lpReturnedString, nSize, redirected.c_str());
    }
    return s_originalGetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault,
        lpReturnedString, nSize, lpFileName);
}

// ---------------------------------------------------------------------------
static DWORD WINAPI hkGetPrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalGetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault,
            lpReturnedString, nSize, lpFileName);

    std::wstring redirected;
    if (BuildRedirectedPath(lpFileName, redirected))
    {
        EnsureRedirectedFile(lpFileName, redirected.c_str());
        return s_originalGetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault,
            lpReturnedString, nSize, redirected.c_str());
    }
    return s_originalGetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault,
        lpReturnedString, nSize, lpFileName);
}

// ---------------------------------------------------------------------------
static DWORD WINAPI hkGetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalGetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, lpFileName);

    std::string redirected;
    if (BuildRedirectedPathA(lpFileName, redirected))
    {
        EnsureRedirectedFileA(lpFileName, redirected.c_str());
        return s_originalGetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, redirected.c_str());
    }
    return s_originalGetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, lpFileName);
}

// ---------------------------------------------------------------------------
static UINT WINAPI hkGetPrivateProfileIntW(LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalGetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, lpFileName);

    std::wstring redirected;
    if (BuildRedirectedPath(lpFileName, redirected))
    {
        EnsureRedirectedFile(lpFileName, redirected.c_str());
        return s_originalGetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, redirected.c_str());
    }
    return s_originalGetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, lpFileName);
}

// ---------------------------------------------------------------------------
static BOOL WINAPI hkWritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalWritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);

    std::string redirected;
    if (BuildRedirectedPathA(lpFileName, redirected))
    {
        EnsureRedirectedFileA(lpFileName, redirected.c_str());
        return s_originalWritePrivateProfileStringA(lpAppName, lpKeyName, lpString, redirected.c_str());
    }
    return s_originalWritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
}

// ---------------------------------------------------------------------------
static BOOL WINAPI hkWritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalWritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);

    std::wstring redirected;
    if (BuildRedirectedPath(lpFileName, redirected))
    {
        EnsureRedirectedFile(lpFileName, redirected.c_str());
        return s_originalWritePrivateProfileStringW(lpAppName, lpKeyName, lpString, redirected.c_str());
    }
    return s_originalWritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);
}

// ---------------------------------------------------------------------------
static DWORD WINAPI hkGetPrivateProfileSectionA(LPCSTR lpAppName, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalGetPrivateProfileSectionA(lpAppName, lpReturnedString, nSize, lpFileName);

    std::string redirected;
    if (BuildRedirectedPathA(lpFileName, redirected))
    {
        EnsureRedirectedFileA(lpFileName, redirected.c_str());
        return s_originalGetPrivateProfileSectionA(lpAppName, lpReturnedString, nSize, redirected.c_str());
    }
    return s_originalGetPrivateProfileSectionA(lpAppName, lpReturnedString, nSize, lpFileName);
}

// ---------------------------------------------------------------------------
static DWORD WINAPI hkGetPrivateProfileSectionW(LPCWSTR lpAppName, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalGetPrivateProfileSectionW(lpAppName, lpReturnedString, nSize, lpFileName);

    std::wstring redirected;
    if (BuildRedirectedPath(lpFileName, redirected))
    {
        EnsureRedirectedFile(lpFileName, redirected.c_str());
        return s_originalGetPrivateProfileSectionW(lpAppName, lpReturnedString, nSize, redirected.c_str());
    }
    return s_originalGetPrivateProfileSectionW(lpAppName, lpReturnedString, nSize, lpFileName);
}

// ---------------------------------------------------------------------------
static BOOL WINAPI hkWritePrivateProfileSectionA(LPCSTR lpAppName, LPCSTR lpString, LPCSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalWritePrivateProfileSectionA(lpAppName, lpString, lpFileName);

    std::string redirected;
    if (BuildRedirectedPathA(lpFileName, redirected))
    {
        EnsureRedirectedFileA(lpFileName, redirected.c_str());
        return s_originalWritePrivateProfileSectionA(lpAppName, lpString, redirected.c_str());
    }
    return s_originalWritePrivateProfileSectionA(lpAppName, lpString, lpFileName);
}

// ---------------------------------------------------------------------------
static BOOL WINAPI hkWritePrivateProfileSectionW(LPCWSTR lpAppName, LPCWSTR lpString, LPCWSTR lpFileName)
{
    if (lpFileName == nullptr)
        return s_originalWritePrivateProfileSectionW(lpAppName, lpString, lpFileName);

    std::wstring redirected;
    if (BuildRedirectedPath(lpFileName, redirected))
    {
        EnsureRedirectedFile(lpFileName, redirected.c_str());
        return s_originalWritePrivateProfileSectionW(lpAppName, lpString, redirected.c_str());
    }
    return s_originalWritePrivateProfileSectionW(lpAppName, lpString, lpFileName);
}

// ---------------------------------------------------------------------------
// Hook installation helper
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
static void InstallOneHook(const char* procName, void* detour, void** original)
{
    MH_STATUS status = MH_CreateHookApi(KERNEL32_DLL, procName, detour, original);
    if (status == MH_OK)
    {
        HMODULE module = ::GetModuleHandleW(KERNEL32_DLL);
        if (module)
        {
            void* target = ::GetProcAddress(module, procName);
            if (target != nullptr)
            {
                MH_EnableHook(target);
                ConsolePrintf("[Overlay][FileRedirect] %s hook installed\n", procName);
                return;
            }
        }
    }

    ConsolePrintf("[Overlay][FileRedirect] %s hook failed: %s\n", procName, MH_StatusToString(status));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
void InstallFileHooks()
{
    // CreateFile hooks
    InstallOneHook("CreateFileW", hkCreateFileW, (void**)&s_originalCreateFileW);
    InstallOneHook("CreateFileA", hkCreateFileA, (void**)&s_originalCreateFileA);

    // Profile API hooks
    InstallOneHook("GetPrivateProfileStringA", hkGetPrivateProfileStringA, (void**)&s_originalGetPrivateProfileStringA);
    InstallOneHook("GetPrivateProfileStringW", hkGetPrivateProfileStringW, (void**)&s_originalGetPrivateProfileStringW);
    InstallOneHook("GetPrivateProfileIntA", hkGetPrivateProfileIntA, (void**)&s_originalGetPrivateProfileIntA);
    InstallOneHook("GetPrivateProfileIntW", hkGetPrivateProfileIntW, (void**)&s_originalGetPrivateProfileIntW);
    InstallOneHook("WritePrivateProfileStringA", hkWritePrivateProfileStringA, (void**)&s_originalWritePrivateProfileStringA);
    InstallOneHook("WritePrivateProfileStringW", hkWritePrivateProfileStringW, (void**)&s_originalWritePrivateProfileStringW);
    InstallOneHook("GetPrivateProfileSectionA", hkGetPrivateProfileSectionA, (void**)&s_originalGetPrivateProfileSectionA);
    InstallOneHook("GetPrivateProfileSectionW", hkGetPrivateProfileSectionW, (void**)&s_originalGetPrivateProfileSectionW);
    InstallOneHook("WritePrivateProfileSectionA", hkWritePrivateProfileSectionA, (void**)&s_originalWritePrivateProfileSectionA);
    InstallOneHook("WritePrivateProfileSectionW", hkWritePrivateProfileSectionW, (void**)&s_originalWritePrivateProfileSectionW);
}

// ---------------------------------------------------------------------------
void InstallFileHooksWithCredentials(const wchar_t* dllPath, std::string accountName)
{
    InstallFileHooks();
    SetRedirectAccount(accountName.c_str());
}

// ---------------------------------------------------------------------------
void SetRedirectAccount(const char* accountName)
{
    if (!accountName || !*accountName)
    {
        ConsolePrintf("[Overlay][FileRedirect] SetRedirectAccount: empty name, redirection disabled\n");
        return;
    }

    // Build wide & narrow suffix: _<account>.ini
    s_redirectSuffixA = "_" + std::string(accountName) + ".ini";
    s_redirectSuffixW = L"_" + fs::path(accountName).wstring() + L".ini";

    s_redirectEnabled.store(true, std::memory_order_release);
    ConsolePrintf("[Overlay][FileRedirect] Redirection enabled for account \"%s\"\n", accountName);
}

// ---------------------------------------------------------------------------
void RemoveFileHooks()
{
    HMODULE hModule = ::GetModuleHandleW(KERNEL32_DLL);
    if (hModule == 0)
        return;

    auto disableLambda = [hModule](const char* name, auto& ptr)
        {
            if (ptr)
            {
                FARPROC procAddress = ::GetProcAddress(hModule, name);
                MH_DisableHook(procAddress);
                ptr = nullptr;
            }
        };

    disableLambda("CreateFileW", s_originalCreateFileW);
    disableLambda("CreateFileA", s_originalCreateFileA);
    disableLambda("GetPrivateProfileStringA", s_originalGetPrivateProfileStringA);
    disableLambda("GetPrivateProfileStringW", s_originalGetPrivateProfileStringW);
    disableLambda("GetPrivateProfileIntA", s_originalGetPrivateProfileIntA);
    disableLambda("GetPrivateProfileIntW", s_originalGetPrivateProfileIntW);
    disableLambda("WritePrivateProfileStringA", s_originalWritePrivateProfileStringA);
    disableLambda("WritePrivateProfileStringW", s_originalWritePrivateProfileStringW);
    disableLambda("GetPrivateProfileSectionA", s_originalGetPrivateProfileSectionA);
    disableLambda("GetPrivateProfileSectionW", s_originalGetPrivateProfileSectionW);
    disableLambda("WritePrivateProfileSectionA", s_originalWritePrivateProfileSectionA);
    disableLambda("WritePrivateProfileSectionW", s_originalWritePrivateProfileSectionW);

    s_redirectEnabled.store(false, std::memory_order_relaxed);
    ConsolePrintf("[Overlay][FileRedirect] All hooks removed\n");
}