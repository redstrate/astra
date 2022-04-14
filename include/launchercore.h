#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QFuture>
#include <QSettings>
#include <QUuid>
#include <QProcess>
#include <QMessageBox>

class SapphireLauncher;
class SquareLauncher;
class SquareBoot;
class AssetUpdater;
class Watchdog;

enum class GameLicense {
    WindowsStandalone,
    WindowsSteam,
    macOS,
    FreeTrial
};

enum class WineType {
    System,
    Custom,
    Builtin, // macos only
    XIVOnMac // macos only
};

enum class DalamudChannel {
    Stable,
    Staging,
    Net5
};

struct ProfileSettings {
    QUuid uuid;
    QString name;

    // game
    int language = 1; // 1 is english, thats all i know
    QString gamePath, winePath, winePrefixPath;
    QString bootVersion, gameVersion, wineVersion;
    int installedMaxExpansion = -1;
    QList<QString> expansionVersions;
    bool enableWatchdog = false;

    bool isGameInstalled() const {
        return !gameVersion.isEmpty();
    }

    bool isWineInstalled() const {
        return !wineVersion.isEmpty();
    }

#if defined(Q_OS_MAC)
    WineType wineType = WineType::Builtin;
#else
    WineType wineType = WineType::System;
#endif

    bool useEsync = false, useGamescope = false, useGamemode = false;
    bool useDX9 = false;
    bool enableDXVKhud = false;

    struct GamescopeOptions {
        bool fullscreen = true;
        bool borderless = true;
        int width = 0;
        int height = 0;
        int refreshRate = 0;
    } gamescope;

    struct DalamudOptions {
        bool enabled = false;
        bool optOutOfMbCollection = false;
        DalamudChannel channel = DalamudChannel::Stable;
    } dalamud;

    // login
    bool encryptArguments = true;
    bool isSapphire = false;
    QString lobbyURL;
    bool rememberUsername = false, rememberPassword = false;
    bool useOneTimePassword = false;

    GameLicense license = GameLicense::WindowsStandalone;
};

struct AppSettings {
    bool closeWhenLaunched = true;
    bool showBanners = true;
    bool showNewsList = true;
};

struct LoginInformation {
    ProfileSettings* settings = nullptr;

    QString username, password, oneTimePassword;
};

struct LoginAuth {
    QString SID;
    int region = 2; // america?
    int maxExpansion = 1;

    // if empty, dont set on the client
    QString lobbyhost, frontierHost;
};

class LauncherCore : public QObject {
Q_OBJECT
public:
    LauncherCore();
    ~LauncherCore();

    QNetworkAccessManager* mgr;

    ProfileSettings getProfile(int index) const;
    ProfileSettings& getProfile(int index);

    int getProfileIndex(QString name);
    QList<QString> profileList() const;
    int addProfile();
    int deleteProfile(QString name);

    void launchGame(const ProfileSettings& settings, LoginAuth auth);

    void launchExecutable(const ProfileSettings& settings, QStringList args);

    /*
     * Used for processes that should be wrapped in gamescope, etc.
     */
    void launchGameExecutable(const ProfileSettings& settings, QProcess* process, QStringList args);

    /*
     * This just wraps it in wine if needed.
     */
    void launchExecutable(const ProfileSettings& settings, QProcess* process, QStringList args, bool isGame, bool needsRegistrySetup);

    /*
     * Launches an external tool. Gamescope for example is intentionally excluded.
     */
    void launchExternalTool(const ProfileSettings& settings, QStringList args);

    void addRegistryKey(const ProfileSettings& settings, QString key, QString value, QString data);

    void buildRequest(const ProfileSettings& settings, QNetworkRequest& request);
    void setSSL(QNetworkRequest& request);
    QString readVersion(QString path);
    void readInitialInformation();
    void readGameVersion();
    void readWineInfo(ProfileSettings& settings);
    void saveSettings();

    void addUpdateButtons(const ProfileSettings& settings, QMessageBox& messageBox);

    QSettings settings;

    SapphireLauncher* sapphireLauncher;
    SquareBoot* squareBoot;
    SquareLauncher* squareLauncher;
    AssetUpdater* assetUpdater;
    Watchdog* watchdog;

    bool gamescopeAvailable = false;
    bool gamemodeAvailable = false;

    AppSettings appSettings;

    QString dalamudVersion;
    int dalamudAssetVersion = -1;
    QString runtimeVersion;
    QString nativeLauncherVersion;

    int defaultProfileIndex = 0;
signals:
    void settingsChanged();
    void successfulLaunch();
    void gameClosed();

private:
    void readExpansionVersions(ProfileSettings& info, int max);
    bool checkIfInPath(QString program);

    QString getDefaultGamePath();
    QString getDefaultWinePrefixPath();

    QVector<ProfileSettings> profileSettings;
};
