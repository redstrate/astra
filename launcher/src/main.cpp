// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QQuickStyle>
#include <QtWebView>
#include <qcoroqml.h>

#include "astra-version.h"
#include "compatibilitytoolinstaller.h"
#include "gameinstaller.h"
#include "launchercore.h"
#include "logger.h"
#include "sapphirelauncher.h"

int main(int argc, char *argv[])
{
    initializeLogging();

    QtWebView::initialize();

    QApplication app(argc, argv);

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
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
                     i18n("© 2023 Joshua Goins"));
    about.addAuthor(i18n("Joshua Goins"),
                    i18n("Maintainer"),
                    QStringLiteral("josh@redstrate.com"),
                    QStringLiteral("https://redstrate.com/"),
                    QUrl("https://redstrate.com/rss-image.png"));
    about.setHomepage(QStringLiteral("https://xiv.zone/astra"));
    about.addComponent(QStringLiteral("physis"),
                       QStringLiteral("Library to access FFXIV data"),
                       physis_get_physis_version(),
                       QStringLiteral("https://xiv.zone/physis"),
                       KAboutLicense::GPL_V3);
    about.addComponent(QStringLiteral("libphysis"),
                       QStringLiteral("C bindings for physis"),
                       physis_get_libphysis_version(),
                       QStringLiteral("https://git.sr.ht/~redstrate/libphysis"),
                       KAboutLicense::GPL_V3);
    about.setDesktopFileName(QStringLiteral("zone.xiv.astra"));
    about.setBugAddress(QByteArrayLiteral("https://lists.sr.ht/~redstrate/public-inbox"));
    about.setComponentName(QStringLiteral("astra"));
    about.setProgramLogo(QStringLiteral("zone.xiv.astra"));

    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Linux FFXIV Launcher"));

    QCommandLineOption steamOption(QStringLiteral("steam"), QStringLiteral("Used for booting the launcher from Steam."), QStringLiteral("verb"));
    steamOption.setFlags(QCommandLineOption::HiddenFromHelp);
    parser.addOption(steamOption);

    about.setupCommandLine(&parser);
    parser.parse(QCoreApplication::arguments());
    about.processCommandLine(&parser);

    // We must handle these manually, since we use parse() and not process()
    if (parser.isSet("help")) {
        parser.showHelp();
    }

    if (parser.isSet("version")) {
        parser.showVersion();
    }

    if (parser.isSet(steamOption)) {
#ifndef ENABLE_STEAM
        QMessageBox::warning(nullptr,
                             i18n("Warning"),
                             i18n("You somehow launched Astra through Steam, despite it not being compiled with Steam support. Some features may not work!"));
#endif

        const QStringList args = parser.positionalArguments();
        // Steam tries to use as a compatibility tool, running installation scripts (like DirectX), so try to ignore it.
        if (!args.empty() && !args[0].contains(QLatin1String("ffxivboot.exe"))) {
            return 0;
        }
    }

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
