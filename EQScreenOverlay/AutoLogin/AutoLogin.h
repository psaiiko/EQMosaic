#pragma once
#include <string>
#include <cstdint>
#include "EQTypes.h"

struct ProfileRecord
{
    std::string accountName;
    std::string accountPassword;
    std::string serverName;
    std::string characterName;
};

enum class LoginPhase
{
    Idle,
    SplashScreen,
    Connect,
    ConnectConfirm,
    ServerSelect,
    ServerSelectConfirm,
    ServerSelectKick,
    CharSelect,
    CharSelectWait,
    WaitConnect,
    WaitServerSelect,
    InGame,
    Failed,
};


class AutoLogin
{
public:
    AutoLogin();
    ~AutoLogin();

    bool LoadCredentialsFromCommandLine();
    void SetCredentials(const ProfileRecord& record);
    ProfileRecord GetProfile() const { return m_profile; }
    bool IsValid() const { return m_isValid; }

    void Update();

    LoginPhase GetPhase() const { return m_phase; }
    const char* GetPhaseName() const;
    const char* GetAccountName() const { return m_profile.accountName.c_str(); }
    const char* GetPlayerName() const { return m_profile.characterName.c_str(); }
    bool IsRunning() const;
    bool IsDone() const;

    void Start();
    void Stop();

private:
    void TransitionTo(LoginPhase newPhase);

    // Entry states (one-shot, run once then transition to a wait state)
    void OnSplashScreen();
    void OnConnect();
    void OnConnectConfirm();
    void OnServerSelect();
    void OnServerSelectConfirm();
    void OnServerSelectKick();
    void OnCharSelect();

    // Wait states (poll with minimum delay between checks)
    void PollWaitConnect();
    void PollWaitServerSelect();
    void PollCharSelectWait();

    void HandlePreSelectDialogs();

    // Helpers
    void RetryConnect();
    void StartConnect();

    void TransitionFromGameState();
    bool CheckServerSelectScreen();
    bool CheckCharSelectScreen();
    bool CheckOkDialog();
    bool CheckYesNoDialog();

    ProfileRecord   m_profile;
    LoginPhase      m_phase = LoginPhase::Idle;
    EQGameState     m_lastGameState = GAMESTATE_PRECHARSELECT;
    uint64_t        m_lastTransitionTime = 0;
    uint64_t        m_lastPollTime = 0;
    uint64_t        m_lastServerNotFoundLog = 0;
    int             m_retryCount = 0;
    int             m_targetServerID = -1;
    bool            m_isValid = false;

    static constexpr int kStepDelay = 2000;
    static constexpr int kConnectTimeout = 45000;
    static constexpr int kMaxRetries = 5;
    static constexpr int kPollDelay = 1000;
    static constexpr int kCharSelectDelay = 3000;
};
