#include "settingswindow.h"

#include <QFormLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QLabel>
#include <QFileDialog>
#include <QCheckBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QProcess>
#include <QGridLayout>
#include <QLineEdit>

#include "xivlauncher.h"

SettingsWindow::SettingsWindow(LauncherWindow& window, QWidget* parent) : window(window), QWidget(parent) {
    setWindowTitle("Settings");
    setWindowModality(Qt::WindowModality::ApplicationModal);

    auto mainLayout = new QGridLayout(this);
    setLayout(mainLayout);

    profileWidget = new QListWidget();
    profileWidget->addItem("INVALID *DEBUG*");
    profileWidget->setCurrentRow(0);

    connect(profileWidget, &QListWidget::currentRowChanged, [=]() {
        reloadControls();
    });

    mainLayout->addWidget(profileWidget, 0, 0);

    auto addProfileButton = new QPushButton("Add Profile");
    connect(addProfileButton, &QPushButton::pressed, [=] {
       profileWidget->setCurrentRow(this->window.addProfile());
    });
    mainLayout->addWidget(addProfileButton, 1, 0);

    auto gameBox = new QGroupBox("Game Options");
    auto gameBoxLayout = new QFormLayout();
    gameBox->setLayout(gameBoxLayout);

    mainLayout->addWidget(gameBox, 0, 1);

    directXCombo = new QComboBox();
    directXCombo->addItem("DirectX 11");
    directXCombo->addItem("DirectX 9");
    gameBoxLayout->addRow("DirectX Version", directXCombo);

    connect(directXCombo, &QComboBox::currentIndexChanged, [=](int index) {
        this->window.getProfile(profileWidget->currentRow()).useDX9 = directXCombo->currentIndex() == 1;
    });

    auto currentGameDirectory = new QLabel(window.currentProfile().gamePath);
    currentGameDirectory->setWordWrap(true);
    gameBoxLayout->addRow("Game Directory", currentGameDirectory);

    auto selectDirectoryButton = new QPushButton("Select Game Directory");
    connect(selectDirectoryButton, &QPushButton::pressed, [this, currentGameDirectory] {
        this->window.currentProfile().gamePath = QFileDialog::getExistingDirectory(this, "Open Game Directory");
        currentGameDirectory->setText(this->window.currentProfile().gamePath);

        this->window.readInitialInformation();
    });
    gameBoxLayout->addWidget(selectDirectoryButton);

    auto gameDirectoryButton = new QPushButton("Open Game Directory");
    connect(gameDirectoryButton, &QPushButton::pressed, [this] {
        openPath(this->window.currentProfile().gamePath);
    });
    gameBoxLayout->addWidget(gameDirectoryButton);

    auto loginBox = new QGroupBox("Login Options");
    auto loginBoxLayout = new QFormLayout();
    loginBox->setLayout(loginBoxLayout);

    mainLayout->addWidget(loginBox, 1, 1);

    auto serverType = new QComboBox();
    serverType->insertItem(0, "Square Enix");
    serverType->insertItem(1, "Sapphire");
    //serverType->setCurrentIndex(savedServerType);

    loginBoxLayout->addRow("Server Lobby", serverType);

    auto lobbyServerURL = new QLineEdit();
    //lobbyServerURL->setText(savedLobbyURL);
    loginBoxLayout->addRow("Lobby URL", lobbyServerURL);

    auto rememberUsernameBox = new QCheckBox();
    //rememberUsernameBox->setChecked(shouldRememberUsername);
    loginBoxLayout->addRow("Remember Username?", rememberUsernameBox);

    auto rememberPasswordBox = new QCheckBox();
    //rememberPasswordBox->setChecked(shouldRememberPassword);
    loginBoxLayout->addRow("Remember Password?", rememberPasswordBox);

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    auto wineBox = new QGroupBox("Wine Options");
    auto wineBoxLayout = new QFormLayout();
    wineBox->setLayout(wineBoxLayout);

    mainLayout->addWidget(wineBox, 0, 2);

    auto infoLabel = new QLabel("This is a list of possible enhancements you can make to your Wine gaming experience.\n"
                                "This is all stuff you can do outside of the launcher, but we can take care of it for you.");
    infoLabel->setWordWrap(true);
    wineBoxLayout->addWidget(infoLabel);

    auto winePathLabel = new QLabel(window.currentProfile().winePath);
    winePathLabel->setWordWrap(true);
    wineBoxLayout->addRow("Wine Executable", winePathLabel);

    auto wineVersionCombo = new QComboBox();

#if defined(Q_OS_MAC)
    wineVersionCombo->insertItem(2, "FFXIV Built-In");
#endif

    wineVersionCombo->insertItem(0, "System Wine");
    wineVersionCombo->insertItem(1, "Custom Path...");
    wineVersionCombo->setCurrentIndex(window.settings.value("wineVersion", 0).toInt());
    wineBoxLayout->addWidget(wineVersionCombo);

    auto selectWineButton = new QPushButton("Select Wine Executable");
    selectWineButton->setEnabled(window.settings.value("wineVersion", 0).toInt() == 2);
    wineBoxLayout->addWidget(selectWineButton);

    connect(wineVersionCombo, &QComboBox::currentIndexChanged, [this, selectWineButton, winePathLabel](int index) {
        this->window.settings.setValue("wineVersion", index);
        selectWineButton->setEnabled(index == 1);

        this->window.readInitialInformation();
        winePathLabel->setText(this->window.currentProfile().winePath);
    });

    connect(selectWineButton, &QPushButton::pressed, [this, winePathLabel] {
        this->window.currentProfile().winePath = QFileDialog::getOpenFileName(this, "Open Wine Executable");
        this->window.settings.setValue("winePath", this->window.currentProfile().winePath);

        this->window.readInitialInformation();
        winePathLabel->setText(this->window.currentProfile().winePath);
    });

    auto winePrefixDirectory = new QLabel(window.currentProfile().winePrefixPath);
    winePrefixDirectory->setWordWrap(true);
    wineBoxLayout->addRow("Wine Prefix", winePrefixDirectory);

    auto selectPrefixButton = new QPushButton("Select Wine Prefix");
    connect(selectPrefixButton, &QPushButton::pressed, [this, winePrefixDirectory] {
        this->window.currentProfile().winePrefixPath = QFileDialog::getExistingDirectory(this, "Open Wine Prefix");
        winePrefixDirectory->setText(this->window.currentProfile().winePrefixPath);

        this->window.readInitialInformation();
    });
    wineBoxLayout->addWidget(selectPrefixButton);

    auto openPrefixButton = new QPushButton("Open Wine Prefix");
    connect(openPrefixButton, &QPushButton::pressed, [this] {
        openPath(this->window.currentProfile().winePrefixPath);
    });
    wineBoxLayout->addWidget(openPrefixButton);

    auto enableDXVKhud = new QCheckBox("Enable DXVK HUD");
    enableDXVKhud->setChecked(window.currentProfile().enableDXVKhud);
    wineBoxLayout->addWidget(enableDXVKhud);

    connect(enableDXVKhud, &QCheckBox::stateChanged, [this](int state) {
        this->window.currentProfile().enableDXVKhud = state;
        this->window.settings.setValue("enableDXVKhud", static_cast<bool>(state));
    });
#endif

#if defined(Q_OS_LINUX)
    auto useEsync = new QCheckBox("Use Esync");
    useEsync->setChecked(window.currentProfile().useEsync);
    wineBoxLayout->addWidget(useEsync);

    auto esyncLabel = new QLabel("Improves general game performance, but requires a Wine built with the Esync patches.\n"
                                 "If you use the latest Wine staging, it should work.");
    esyncLabel->setWordWrap(true);
    wineBoxLayout->addWidget(esyncLabel);

    connect(useEsync, &QCheckBox::stateChanged, [this](int state) {
        this->window.currentProfile().useEsync = state;
        this->window.settings.setValue("useEsync", static_cast<bool>(state));
    });

    auto useGamescope = new QCheckBox("Use Gamescope");
    useGamescope->setChecked(window.currentProfile().useGamescope);
    wineBoxLayout->addWidget(useGamescope);

    auto gamescopeLabel = new QLabel("Use the SteamOS compositor that uses Wayland.\n"
                                 "If you are experiencing input issues on XWayland, try this option if you have it installed.");
    gamescopeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamescopeLabel);

    connect(useGamescope, &QCheckBox::stateChanged, [this](int state) {
        this->window.currentProfile().useGamescope = state;
        this->window.settings.setValue("useGamescope", static_cast<bool>(state));
    });

    auto useGamemode = new QCheckBox("Use Gamemode");
    useGamemode->setChecked(window.currentProfile().useGamemode);
    wineBoxLayout->addWidget(useGamemode);

    auto gamemodeLabel = new QLabel("Use Feral Interactive's GameMode, which applies a couple of performance enhancements.\n"
                                     "May give a slight performance boost, but requires GameMode to be installed.\n");
    gamemodeLabel->setWordWrap(true);
    wineBoxLayout->addWidget(gamemodeLabel);

    connect(useGamemode, &QCheckBox::stateChanged, [this](int state) {
        this->window.currentProfile().useGamemode = state;
        this->window.settings.setValue("useGamemode", static_cast<bool>(state));
    });
#endif

    reloadControls();
}

void SettingsWindow::reloadControls() {
    if(currentlyReloadingControls)
        return;

    currentlyReloadingControls = true;

    auto oldRow = profileWidget->currentRow();

    profileWidget->clear();

    for(auto profile : window.profileList()) {
        profileWidget->addItem(profile);
    }
    profileWidget->setCurrentRow(oldRow);

    ProfileSettings& profile = window.getProfile(profileWidget->currentRow());
    directXCombo->setCurrentIndex(profile.useDX9 ? 1 : 0);

    currentlyReloadingControls = false;
}

void SettingsWindow::openPath(const QString path) {
#if defined(Q_OS_WIN)
    // for some reason, windows requires special treatment (what else is new?)
        const QFileInfo fileInfo(path);

        QProcess::startDetached("explorer.exe", QStringList(QDir::toNativeSeparators(fileInfo.canonicalFilePath())));
#else
    QDesktopServices::openUrl("file://" + path);
#endif
}