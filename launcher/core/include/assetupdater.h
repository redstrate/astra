#pragma once

#include <QJsonArray>
#include <QObject>
#include <QProgressDialog>
#include <QTemporaryDir>

#include "launchercore.h"

class LauncherCore;
class QNetworkReply;
struct ProfileSettings;

class AssetUpdater : public QObject {
    Q_OBJECT
public:
    explicit AssetUpdater(LauncherCore& launcher);

    void update(const ProfileSettings& profile);
    void beginInstall();

    void checkIfCheckingIsDone();
    void checkIfDalamudAssetsDone();
    void checkIfFinished();

signals:
    void finishedUpdating();

private:
    LauncherCore& launcher;

    QProgressDialog* dialog;

    DalamudChannel chosenChannel;

    QString remoteDalamudVersion;
    QString remoteRuntimeVersion;

    QTemporaryDir tempDir;

    bool doneDownloadingDalamud = false;
    bool doneDownloadingRuntimeCore = false;
    bool doneDownloadingRuntimeDesktop = false;
    bool needsRuntimeInstall = false;
    bool needsDalamudInstall = false;

    int remoteDalamudAssetVersion = -1;
    QList<QString> dalamudAssetNeededFilenames;
    QJsonArray remoteDalamudAssetArray;

    QString dataDir;
};
