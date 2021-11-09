#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QFuture>
#include <QSettings>

class SapphireLauncher;
class SquareLauncher;
class SquareBoot;

struct ProfileSettings {
    QString name;

    int language = 1; // 1 is english, thats all i know
    QString gamePath, winePath, winePrefixPath;
    QString bootVersion, gameVersion;
    bool useEsync, useGamescope, useGamemode;
    bool useDX9 = false;
    bool enableDXVKhud = false;

    bool isSapphire = false;
    QString lobbyURL;
    bool rememberUsername, rememberPassword;
};

struct LoginInformation {
    QString username, password, oneTimePassword;
};

struct LoginAuth {
    QString SID;
    int region = 2; // america?
    int maxExpansion = 1;

    // if empty, dont set on the client
    QString lobbyhost, frontierHost;
};

class LauncherWindow : public QMainWindow {
Q_OBJECT
public:
    explicit LauncherWindow(QWidget* parent = nullptr);

    ~LauncherWindow() override;

    QNetworkAccessManager* mgr;

    ProfileSettings currentProfile() const;
    ProfileSettings& currentProfile();

    ProfileSettings getProfile(int index) const;
    ProfileSettings& getProfile(int index);

    void setProfile(QString name);
    void setProfile(int index);
    int getProfileIndex(QString name);
    QList<QString> profileList() const;
    int addProfile();

    void launchGame(const LoginAuth auth);
    void launchExecutable(const QStringList args);
    void buildRequest(QNetworkRequest& request);
    void setSSL(QNetworkRequest& request);
    QString readVersion(QString path);
    void readInitialInformation();

    QSettings settings;

signals:
    void settingsChanged();

private:
    SapphireLauncher* sapphireLauncher;
    SquareBoot* squareBoot;
    SquareLauncher* squareLauncher;

    QList<ProfileSettings> profileSettings;
    int currentProfileIndex = 0;
};
