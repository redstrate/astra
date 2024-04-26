// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QQuickStyle>
#include <kdsingleapplication.h>
#include <qcoroqml.h>

#ifdef HAVE_WEBVIEW
#include <QtWebView>
#endif

#include "astra-version.h"
#include "compatibilitytoolinstaller.h"
#include "gameinstaller.h"
#include "launchercore.h"
#include "logger.h"
#include "physis_logger.h"
#include "sapphirelogin.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
#ifdef HAVE_WEBVIEW
    QtWebView::initialize();
#endif

    if (qEnvironmentVariable("SteamDeck") == QStringLiteral("1")) {
        qputenv("QT_SCALE_FACTOR", "1.25");
        qputenv("QT_QUICK_CONTROLS_MOBILE", "1");
    }

    QGuiApplication app(argc, argv);

    KDSingleApplication singleApplication;
    if (!singleApplication.isPrimaryInstance()) {
        return 0;
    }

    // Default to a sensible message pattern
    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qputenv("QT_MESSAGE_PATTERN", "[%{time yyyy-MM-dd h:mm:ss.zzz}] %{if-category}[%{category}] %{endif}[%{type}] %{message}");
    }

    KLocalizedString::setApplicationDomain("astra");

    KAboutData about(QStringLiteral("astra"),
                     i18n("Astra"),
                     QStringLiteral(ASTRA_VERSION_STRING),
                     i18n("FFXIV Launcher"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2021-2024 Joshua Goins"));
    about.setOtherText(
        i18n("This software requires that you have a legitimate and active subscription to FINAL FANTASY XIV. By using this software, you may be in violation "
             "of your User Agreement.\n\nFINAL FANTASY, FINAL FANTASY XIV, FFXIV, SQUARE ENIX, and the SQUARE ENIX logo are registered trademarks or "
             "trademarks of Square Enix Holdings Co., Ltd.\n"));
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QUrl(QStringLiteral("https://redstrate.com/rss-image.png")));
    about.setHomepage(QStringLiteral("https://xiv.zone/astra"));
    about.addComponent(QStringLiteral("physis"),
                       i18n("Library to access FFXIV data"),
                       QString::fromLatin1(physis_get_physis_version()),
                       QStringLiteral("https://xiv.zone/physis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("libphysis"),
                       i18n("C bindings for physis"),
                       QString::fromLatin1(physis_get_libphysis_version()),
                       QStringLiteral("https://git.sr.ht/~redstrate/libphysis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("KDSingleApplication"),
                       i18n("Helper class for single-instance policy applications "),
                       QStringLiteral("1.1.1"),
                       QStringLiteral("https://github.com/KDAB/KDSingleApplication"),
                       KAboutLicense::MIT);
    about.addComponent(QStringLiteral("libcotp"),
                       i18n(" C library that generates TOTP and HOTP "),
                       QStringLiteral("3.0.0"),
                       QStringLiteral("https://github.com/paolostivanin/libcotp"),
                       KAboutLicense::Unknown);
    about.setDesktopFileName(QStringLiteral("zone.xiv.astra"));
    about.setBugAddress(QByteArrayLiteral("https://lists.sr.ht/~redstrate/public-inbox"));
    about.setComponentName(QStringLiteral("astra"));
    about.setProgramLogo(QStringLiteral("zone.xiv.astra"));
    about.setOrganizationDomain(QByteArrayLiteral("xiv.zone"));

    KAboutData::setApplicationData(about);

    initializeLogging();
    setup_physis_logging();

    QCommandLineParser parser;
    about.setupCommandLine(&parser);

    QCommandLineOption steamOption(QStringLiteral("steam"), QString(), QStringLiteral("verb"));
    steamOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(steamOption);

    parser.parse(QCoreApplication::arguments());
    about.processCommandLine(&parser);

    // We must handle these manually, since we use parse() and not process()
    if (parser.isSet(QStringLiteral("help")) || parser.isSet(QStringLiteral("help-all"))) {
        parser.showHelp();
    }

    if (parser.isSet(QStringLiteral("version"))) {
        parser.showVersion();
    }

    bool isSteamDeck = false;
    if (parser.isSet(steamOption)) {
        const QStringList args = parser.positionalArguments();
        // Steam tries to use as a compatibility tool, running installation scripts (like DirectX), so try to ignore it.
        if (!args.empty() && !args.join(QLatin1Char(';')).contains("ffxivboot.exe"_L1)) {
            return 0;
        }

        if (qEnvironmentVariable("SteamDeck") == QStringLiteral("1")) {
            isSteamDeck = true;
        }
    }

#if defined(Q_OS_LINUX)
    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        if (isSteamDeck) {
            QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
        } else {
            QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
        }
    }
#endif

    QCoro::Qml::registerTypes();

    QQmlApplicationEngine engine;

    auto core = engine.singletonInstance<LauncherCore *>(QStringLiteral("zone.xiv.astra"), QStringLiteral("LauncherCore"));
    if (parser.isSet(steamOption)) {
        core->initializeSteam();
    }

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QObject::connect(&engine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);

    engine.loadFromModule(QStringLiteral("zone.xiv.astra"), QStringLiteral("Main"));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return QCoreApplication::exec();
}
