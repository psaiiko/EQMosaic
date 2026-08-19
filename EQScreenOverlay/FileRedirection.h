#pragma once
#include <string>

// Install hook on kernel32!CreateFileW and .ini read/write APIs to redirect eqclient.ini accesses
// to eqclient_<accountname>.ini. Must be called after MH_Initialize().
void InstallFileHooks();

// Like InstallFileHooks + SetRedirectAccount in one step, but reads the
// account name from the credentials file (login_<PID>.txt) itself.
// Call from DllMain while process is still suspended for earliest coverage.
void InstallFileHooksWithCredentials(const wchar_t* dllPath, std::string accountName);

// Set the account name and enable redirection. After this, any CreateFileW
// call with a path ending in "eqclient.ini" will be redirected.
void SetRedirectAccount(const char* accountName);

// Remove the CreateFileW hook. Call before MH_Uninitialize().
void RemoveFileHooks();