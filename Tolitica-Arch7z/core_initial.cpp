#include "core_initial.h"
#include "connectivityChecker.h"

#include <QProcess>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

// for progress bar
#include <QProgressDialog>
#include <QTimer>
#include <cstddef>

CoreInitial::CoreInitial(QObject *parent)
    : QObject(parent)
{}

bool CoreInitial::themeStatus()
{
    const QString configDir = QStandardPaths::writableLocation(
        QStandardPaths::ConfigLocation
    );
    const QString cfgFile = QDir(configDir).filePath("kdeglobals");

    QSettings settings(cfgFile, QSettings::IniFormat);

    settings.beginGroup("KDE");
    const QString current = settings.value("LookAndFeelPackage", QString()).toString();
    settings.endGroup();

    return (current == "org.kde.breezedark.desktop");
}

bool CoreInitial::xrayThemeStatus()
{
    const QString configDir = QStandardPaths::writableLocation(
        QStandardPaths::ConfigLocation
    );
    const QString cfgFile = QDir(configDir).filePath("kdeglobals");

    QSettings settings(cfgFile, QSettings::IniFormat);

    settings.beginGroup("KDE");
    const QString current = settings.value("LookAndFeelPackage", QString()).toString();
    settings.endGroup();

    return (current == "XRAY-DARK.desktop");
}

void CoreInitial::applyGlobalTheme(const QString &themeId)
{
    qDebug() << "Applying theme:" << themeId;

    // Use lookandfeeltool exactly like systemsettings does
    QProcess::execute("lookandfeeltool", {"--apply", themeId});

    emit themeApplied(themeId);
}

void CoreInitial::reloadPlasmaByReplace()
{
    // Load the layout that matches the currently applied theme
    QDBusMessage layoutMessage = QDBusMessage::createMethodCall("org.kde.plasmashell", "/PlasmaShell", "org.kde.PlasmaShell", "loadLookAndFeelDefaultLayout");
    QList<QVariant> args;
    args << (xrayThemeStatus() ? "XRAY-DARK.desktop" : "org.kde.breezedark.desktop");
    layoutMessage.setArguments(args);
    QDBusConnection::sessionBus().call(layoutMessage, QDBus::NoBlock);

    QDBusMessage kwinMessage = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "reloadConfig");
    QDBusConnection::sessionBus().send(kwinMessage);

    QDBusMessage plasmaMessage = QDBusMessage::createSignal("/PlasmaShell", "org.kde.PlasmaShell", "refreshCurrentShell");
    QDBusConnection::sessionBus().send(plasmaMessage);

    emit reloadFinished();
}

void CoreInitial::reloadPlasmaByDBus()
{
    // Theme is applied, no reload action needed
    emit reloadFinished();
}

bool CoreInitial::osreleaseStatus() {
    QFile configFile("/usr/lib/os-release");

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QMap<QString, QString> expected = {
        {"NAME", "\"Xray_OS\""},
        {"PRETTY_NAME", "\"Xray_OS\""},
        {"ID", "xray_os"},
        {"BUILD_ID", "rolling"},
        {"ANSI_COLOR", "\"38;2;23;147;209\""},
        {"HOME_URL", "\"https://xray-os.github.io/xray_os-website/index.html\""},
        {"DOCUMENTATION_URL", "\"https://xray-os.github.io/xray_os-website/get-started.html\""},
        {"SUPPORT_URL", "\"https://discord.com/invite/dBR7wR3ABk/\""},
        {"BUG_REPORT_URL", "\"https://github.com/Xray-OS/Xray_OS/issues\""},
        {"PRIVACY_POLICY_URL", "\"https://xray-os.github.io/xray_os-website/index.html#about-xray-os\""},
        {"LOGO", "xray-logo"},
        {"IMAGE_ID", "xray_os"}
    };

    // Get IMAGE_VERSION from tolitica.conf
    QString homeDir = QDir::homePath();
    QFile confFile(homeDir+"/tolitica-home-settings/tolitica.conf");
    QString imageVersion;
    if (confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream confIn(&confFile);
        while (!confIn.atEnd()) {
            QString line = confIn.readLine();
            if (line.startsWith("xrayos_img_ver = ")) {
                imageVersion = line.split(" = ")[1];
                break;
            }
        }
    }
    expected["IMAGE_VERSION"] = imageVersion;

    QTextStream in(&configFile);
    int matches = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        qDebug() << "Line:" << line;
        QStringList parts = line.split('=', Qt::KeepEmptyParts);
        if (parts.size() == 2) {
            QString key = parts[0];
            QString value = parts[1];
            if (expected.contains(key) && expected[key] == value) {
                matches++;
            }
        }
    }

    qDebug() << "Matches:" << matches << "Expected size:" << expected.size();
    return matches == expected.size();
}

void CoreInitial::setOSrelease() {
    if (osreleaseStatus()) {
        // Get version from tolitica.conf first
        QString homeDir = QDir::homePath();
        QFile confFile(homeDir + "/tolitica-home-settings/tolitica.conf");
        QString imageVersion = "v25.07.18.01"; // default arch fallback

        if (confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream confIn(&confFile);
            while (!confIn.atEnd()) {
                QString line = confIn.readLine();
                if (line.startsWith("arch_img_ver = ")) {
                    imageVersion = line.split(" = ")[1];
                    break;
                }
            }
        }

        // Convert to ArchLinux
        QStringList commands = {
            "sudo sed -i 's/\"Xray_OS\"/\"Arch Linux\"/g' /usr/lib/os-release",
            "sudo sed -i 's/ID=xray_os/ID=arch/g' /usr/lib/os-release",
            "sudo sed -i 's|https://xray-os.github.io/xray_os-website/index.html|https://archlinux.org/|g' /usr/lib/os-release",
            "sudo sed -i 's|https://xray-os.github.io/xray_os-website/get-started.html|https://wiki.archlinux.org/|g' /usr/lib/os-release",
            "sudo sed -i 's|https://discord.com/invite/dBR7wR3ABk/|https://bbs.archlinux.org/|g' /usr/lib/os-release",
            "sudo sed -i 's|https://github.com/Xray-OS/Xray_OS/issues|https://gitlab.archlinux.org/groups/archlinux/-/issues|g' /usr/lib/os-release",
            "sudo sed -i 's|https://xray-os.github.io/xray_os-website/index.html#about-xray-os|https://terms.archlinux.org/docs/privacy-policy/|g' /usr/lib/os-release",
            "sudo sed -i 's/xray-logo/archlinux-logo/g' /usr/lib/os-release",
            "sudo sed -i 's/IMAGE_ID=xray_os/IMAGE_ID=archlinux/g' /usr/lib/os-release",
            QString("sudo sed -i 's/^IMAGE_VERSION=.*/IMAGE_VERSION=%1/' /usr/lib/os-release").arg(imageVersion)
        };

        for (const QString &cmd : commands) {
            QProcess::execute("pkexec", QStringList() << "bash" << "-c" << cmd);
        }
    } else {
        QString homeDir = QDir::homePath();
        QFile confFile(homeDir + "/tolitica-home-settings/tolitica.conf");
        QString imageVersion = "v17"; // default xray fallback

        if (confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream confIn(&confFile);
            while (!confIn.atEnd()) {
                QString line = confIn.readLine();
                if (line.startsWith("xrayos_img_ver = ")) {
                    imageVersion = line.split(" = ")[1];
                    break;
                }
            }
        }

        // Force set to Xray_OS values regardless of current content
        QStringList commands = {
            "sudo sed -i 's/^NAME=.*/NAME=\"Xray_OS\"/' /usr/lib/os-release",
            "sudo sed -i 's/^PRETTY_NAME=.*/PRETTY_NAME=\"Xray_OS\"/' /usr/lib/os-release",
            "sudo sed -i 's/^ID=.*/ID=xray_os/' /usr/lib/os-release",
            "sudo sed -i 's/^BUILD_ID=.*/BUILD_ID=rolling/' /usr/lib/os-release",
            "sudo sed -i 's/^ANSI_COLOR=.*/ANSI_COLOR=\"38;2;23;147;209\"/' /usr/lib/os-release",
            "sudo sed -i 's|^HOME_URL=.*|HOME_URL=\"https://xray-os.github.io/xray_os-website/index.html\"|' /usr/lib/os-release",
            "sudo sed -i 's|^DOCUMENTATION_URL=.*|DOCUMENTATION_URL=\"https://xray-os.github.io/xray_os-website/get-started.html\"|' /usr/lib/os-release",
            "sudo sed -i 's|^SUPPORT_URL=.*|SUPPORT_URL=\"https://discord.com/invite/dBR7wR3ABk/\"|' /usr/lib/os-release",
            "sudo sed -i 's|^BUG_REPORT_URL=.*|BUG_REPORT_URL=\"https://github.com/Xray-OS/Xray_OS/issues\"|' /usr/lib/os-release",
            "sudo sed -i 's|^PRIVACY_POLICY_URL=.*|PRIVACY_POLICY_URL=\"https://xray-os.github.io/xray_os-website/index.html#about-xray-os\"|' /usr/lib/os-release",
            "sudo sed -i 's/^LOGO=.*/LOGO=xray-logo/' /usr/lib/os-release",
            "sudo sed -i 's/^IMAGE_ID=.*/IMAGE_ID=xray_os/' /usr/lib/os-release",
            QString("sudo sed -i 's/^IMAGE_VERSION=.*/IMAGE_VERSION=%1/' /usr/lib/os-release").arg(imageVersion)
        };

        for (const QString &cmd : commands) {
            QProcess::execute("pkexec", QStringList() << "bash" << "-c" << cmd);
        }
    }
}

bool CoreInitial::konsoleProfStatus() {
    QString homeDir = QDir::homePath();
    QFile configFile(homeDir + "/.config/konsolerc");

    if(!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&configFile);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("DefaultProfile=")) {
            QString value = line.mid(15).trimmed();
            value.remove('"');
            if (value == "Xray_OS.profile") {
                return true;
            }
        }
    }
    return false;
}

void CoreInitial::setKonsoleProfile() {
    QString homeDir = QDir::homePath();
    QFile configFile(homeDir + "/.config/konsolerc");

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    // Get current profile value first
    QString currentProfile;
    QTextStream in(&configFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("DefaultProfile=")) {
            currentProfile = line.mid(15).trimmed();
            currentProfile.remove('"');
            break;
        }
    }
    configFile.close();

    // Save current profile to tolitica.conf if it's not Xray_OS.profile
    if (currentProfile != "Xray_OS.profile") {
        QFile toliticaConf(homeDir + "/tolitica-home-settings/tolitica.conf");
        QStringList lines;

        if (toliticaConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream tIn(&toliticaConf);
            bool found = false;
            while (!tIn.atEnd()) {
                QString line = tIn.readLine();
                if (line.startsWith("lastKonsoleProfile=")) {
                    lines << "lastKonsoleProfile=" + currentProfile;
                    found = true;
                } else {
                    lines << line;
                }
            }
            if (!found) {
                lines << "lastKonsoleProfile=" + currentProfile;
            }
            toliticaConf.close();
        } else {
            lines << "lastKonsoleProfile=" + currentProfile;
        }

        if (toliticaConf.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream tOut(&toliticaConf);
            for (const QString &line : lines) {
                tOut << line << "\n";
            }
        }
    }

    // Now modify konsolerc
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QStringList lines;
    in.setDevice(&configFile);
    bool foundDefaultProfile = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith("DefaultProfile=")) {
            if (currentProfile == "Xray_OS.profile") {
                // Get saved profile from tolitica.conf
                QString savedProfile = "Arch.profile"; // default fallback
                QFile toliticaConf(homeDir + "/tolitica-home-settings/tolitica.conf");
                if (toliticaConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream tIn(&toliticaConf);
                    while (!tIn.atEnd()) {
                        QString tLine = tIn.readLine();
                        if (tLine.startsWith("lastKonsoleProfile=")) {
                            savedProfile = tLine.mid(19).trimmed();
                            break;
                        }
                    }
                }
                lines << "DefaultProfile=" + savedProfile;
            } else {
                lines << "DefaultProfile=Xray_OS.profile";
            }
            foundDefaultProfile = true;
        } else {
            lines << line;
        }
    }

    if (!foundDefaultProfile) {
        lines << "DefaultProfile=Xray_OS.profile";
    }

    configFile.close();

    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);
        for (const QString &line : lines) {
            out << line << "\n";
        }
    }
}

bool CoreInitial::grubThemeStatus() {
    QFile configFile("/etc/default/grub");

    if(!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&configFile);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("GRUB_THEME=")) {
            QString value = line.mid(11).trimmed();
            value.remove('"');
            if (value == "/boot/grub/themes/xray_os/theme.txt") {
                return true;
            }
        }
    }
    return false;
}

void CoreInitial::setGrubTheme() {
    bool status = grubThemeStatus();
    int readable = grubThemeStatus();

    QString newValue = (status) ? "/boot/grub/themes/Arch-Linux/theme.txt" :
    "/boot/grub/themes/xray_os/theme.txt";
    QString command = QString("sed -i 's|^GRUB_THEME=.*|GRUB_THEME=%1|' /etc/default/grub && grub-mkconfig -o /boot/grub/grub.cfg").arg(newValue);

    QProcess process;
    process.start("pkexec", QStringList() << "bash" << "-c" << command);
    process.waitForFinished();

    // Debug output
    qDebug() << "Command:" << command;
    qDebug() << "Exit code:" << process.exitCode();
    qDebug() << "Standard output:" << process.readAllStandardOutput();
    qDebug() << "Standard error:" << process.readAllStandardError();

    if (process.exitCode() != 0) {
        QMessageBox::critical(nullptr, "Error",
            QString("Failed to modify grub. Exit code: %1\nError: %2")
                .arg(process.exitCode())
                .arg(QString(process.readAllStandardError())));
    }
}

QString CoreInitial::currentIcons() {
    QString homeDir = QDir::homePath();
    QFile configFile(homeDir + "/.config/kdeglobals");

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "breeze-dark";
    }

    QTextStream in(&configFile);
    bool inIconsSection = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']')) {
            inIconsSection = (line == "[Icons]");
            continue;
        }

        if (inIconsSection && line.startsWith("Theme=")) {
            QString themeValue = line.mid(6).trimmed();
            themeValue.remove('"');
            if (themeValue == "Dracula") {
                return "Dracula";
            } else if (themeValue == "Surfn-Tela") {
                return "Surfn-Tela";
            } else {
                return "breeze-dark";
            }
        }
    }
    return "breeze-dark";
}

void CoreInitial::setIcons(const QString &icons) {
    QString homeDir = QDir::homePath();
    QFile configFile(homeDir + "/.config/kdeglobals");

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QStringList lines;
    QTextStream in(&configFile);
    bool inIconSection = false;
    bool foundIconsSection = false;

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.startsWith('[') && line.endsWith(']')) {
            inIconSection = (line.trimmed() == "[Icons]");
            if (inIconSection) foundIconsSection = true;
            lines << line;
        } else if (inIconSection && line.trimmed().startsWith("Theme=")) {
            lines << "Theme=" + icons;
        } else {
            lines << line;
        }
    }
    configFile.close();

    // Add [Icons] section if it doesn't exist
    if (!foundIconsSection) {
        qDebug() << "Adding missing [Icons] section";
        lines << "[Icons]";
        lines << "Theme=" + icons;
    }

    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&configFile);
        for (const QString &line : lines) {
            out << line << "\n";
        }
    }

    // Reload plasma to apply icon changes
    QProcess::execute("kquitapp6", QStringList() << "plasmashell");
    QProcess::execute("kstart", QStringList() << "plasmashell");
}

bool CoreInitial::aurStatus(const QString &aur) {
    QProcess process;
    QString command = QString("pacman -Q %1").arg(aur);
    qDebug() << "command: " << command;

    process.start("bash", QStringList() << "-c" << command);
    qDebug() << "process: " << process.exitCode();
    process.waitForFinished();

    bool aurEnabled = (process.exitCode() == 0);

    // qDebug() << ("%1 installed?: ").arg(aur) << yay;
    return aurEnabled ? true : false;
}

void CoreInitial::getRemoveAUR(QWidget *parent, const QString &aurHelper, std::function<void(bool)> callback) {
    bool status = aurStatus(aurHelper);
    qDebug() << "getRemoveAUR-status: " << status;

    QString command = QString((status) ? "pacman -Rns --noconfirm %1" :
        "pacman -S --noconfirm %1").arg(aurHelper);
    qDebug() << "getRemoveAUR-command: " << command;

    // Verify if there is online connection
    QString offlinePath = QDir::homePath() +
    QString("/tolitica-home-settings/offline-packages/%1").arg(aurHelper);
    QDir dir(offlinePath);

    QStringList zstFiles = dir.entryList({ "*.zst" }, QDir::Files, QDir::Name);

    if (!dir.exists()) {
        qWarning() << "Offline directory missing:" << offlinePath;
        return;
    } else if (zstFiles.isEmpty()) {
        qWarning() << "No .zst package found in" << offlinePath;
        return;
    }

    QProgressDialog *progress = new QProgressDialog(
        QString((status) ? "Removing %1" : "Installing %1").arg(aurHelper), nullptr, 0, 100,
        parent);
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setCancelButton(nullptr);
    progress->setValue(0);
    progress->show();

    QTimer *monitorTimer = new QTimer(parent);
    int progressValue = 0;

    ConnectivityChecker *internetChecker = new ConnectivityChecker(this);
    connect(internetChecker, &ConnectivityChecker::connectivityChecked,
    this, [internetChecker, command, offlinePath, status, callback,
    progress, monitorTimer, progressValue, parent]
        (bool isConnected) {
            qDebug() << "OUTPUT: isConnected =" << isConnected;
            QString cmdToRun;
            if (!isConnected && !status) {
                cmdToRun = QString("sudo pacman -U --noconfirm %1/*.zst").arg(offlinePath);
            } else {
                cmdToRun = command;
            }
            qDebug() << "CMD_TO_RUN: " << cmdToRun;

        connect(monitorTimer, &QTimer::timeout, parent, [=]() mutable {
            if (progressValue < 95) {
                progressValue += 2;
                progress->setValue(progressValue);
            }
        });
        monitorTimer->start(250);

        QProcess *process = new QProcess();
        int attempts = 0;
        do {
            process->start("pkexec", QStringList() << "bash" << "-c" << cmdToRun);
            process->waitForFinished();

            if (process->exitCode() != 0 && attempts < 2) {
                attempts++;
            }
        } while (process->exitCode() != 0 && attempts < 3);

        qDebug() << "PROCESS-OUTPUT: " << process->exitCode();

        monitorTimer->stop();
        progress->setValue(100);

        bool success = (process->exitCode() == 0);
        process->deleteLater();
        progress->deleteLater();
        monitorTimer->deleteLater();

        if (callback) callback(success);
        if (success)
            internetChecker->deleteLater();
    });
    // kick off the check
    internetChecker->checkConnectivity();
}

bool CoreInitial::storeStatus(const QString &store) {
    QProcess process;
    QString command = QString("pacman -Q %1").arg(store);
    qDebug() << "command: " << command;

    process.start("bash", QStringList() << "-c" << command);
    qDebug() << "process: " << process.exitCode();
    process.waitForFinished();

    bool storeEnabled = (process.exitCode() == 0);
    return storeEnabled ? true : false;
}

void::CoreInitial::getRemoveStore(QWidget *parent, const QString &store, std::function<void(bool)> callback) {
    bool status = storeStatus(store);
    qDebug() << "getRemoveStore-status: " << status;

    QString command = QString((status) ? "pacman -Rns --noconfirm %1" :
        "pacman -S --noconfirm %1").arg(store);
    qDebug() << "getRemoveStore-command: " << command;

    // Verify if there is online connection
    QString offlinePath = QDir::homePath() +
    QString("/tolitica-home-settings/offline-packages/%1").arg(store);
    QDir dir(offlinePath);

    QStringList zstFiles = dir.entryList({ "*.zst" }, QDir::Files, QDir::Name);

    if (!dir.exists()) {
        qWarning() << "Offline directory missing: " << offlinePath;
        return;
    } else if (zstFiles.isEmpty()) {
        qWarning() << "No .zst package found in: " << offlinePath;
        return;
    }

    QProgressDialog *progress = new QProgressDialog(
        QString((status) ? "Removing %1" : "Installing %1").arg(store), nullptr, 0, 100,
        parent);
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setCancelButton(nullptr);
    progress->setValue(0);
    progress->show();

    QTimer *monitorTimer = new QTimer(parent);
    int progressValue = 0;

    ConnectivityChecker *internetChecker = new ConnectivityChecker(this);
    connect(internetChecker, &ConnectivityChecker::connectivityChecked,
        this, [internetChecker, command, offlinePath, status, callback,
        progress, monitorTimer, progressValue, parent]
    (bool isConnected) mutable {
        qDebug() << "OUTPUT: is connected = " << isConnected;
        QString cmdToRun;
        if (!isConnected && !status) {
            cmdToRun = QString("sudo pacman -U --noconfirm %1/*.zst").arg(offlinePath);
        } else {
            cmdToRun = command;
        }
        qDebug() << "CMD_TO_RUN: " << cmdToRun;

        connect(monitorTimer, &QTimer::timeout, parent, [=]() mutable {
            if (progressValue < 95) {
                progressValue += 2;
                progress->setValue(progressValue);
            }
        });
        monitorTimer->start(250);

        QProcess *process = new QProcess();
        int attempts = 0;
        do {
            process->start("pkexec", QStringList() << "bash" << "-c" << cmdToRun);
            process->waitForFinished();

            if (process->exitCode() != 0 && attempts < 2) {
                attempts++;
            }
        } while (process->exitCode() != 0 && attempts < 3);

        qDebug() << "PROCESS-OUTPUT: " << process->exitCode();

        monitorTimer->stop();
        progress->setValue(100);

        bool success = (process->exitCode() == 0);
        process->deleteLater();
        progress->deleteLater();
        monitorTimer->deleteLater();

        if (callback) callback(success);
        if (success)
            internetChecker->deleteLater();
    });
    // Kick off the check
    internetChecker->checkConnectivity();
}

bool CoreInitial::gamingMetaStatus() {
    QProcess process;
    process.start("bash", QStringList() << "-c" << "pacman -Q arch7z-gaming-meta");
    process.waitForFinished();

    bool gamingEnabled = (process.exitCode() == 0);

    return gamingEnabled ? true : false;
}

void CoreInitial::getArch7zGamingMeta(QWidget *parent,
    std::function<void(bool)>callback) {
        bool status = gamingMetaStatus();

        QString command = (status) ? "pacman -Rns --noconfirm arch7z-gaming-meta"
            : "pacman -S --noconfirm arch7z-gaming-meta";

        QProgressDialog *progress = new QProgressDialog(
            (status) ? "Removing Arch7z Gaming Meta..."
            : "Installing Arch7z Gaming Meta", nullptr, 0, 100, parent);
        progress->setWindowModality(Qt::ApplicationModal);
        progress->setCancelButton(nullptr);
        progress->setValue(0);
        progress->show();

        QTimer *monitorTimer = new QTimer(parent);
        int progressValue = 0;

        ConnectivityChecker *internetChecker = new ConnectivityChecker(this);
        connect(internetChecker, &ConnectivityChecker::connectivityChecked,
            this, [internetChecker, command, status, callback, progress,
            monitorTimer, progressValue, parent](bool isConnected)
            mutable {
                qDebug() << "OUTPUT: is connected = " << isConnected;
                if (!isConnected) {
                    QMessageBox::warning(parent, "Failed to start installation",
                        "Your system is not connected to internet, check connection first");
                    return;
                }

                connect(monitorTimer, &QTimer::timeout, parent, [=]() mutable {
                    if (progressValue < 95) {
                        progressValue += 2;
                        progress->setValue(progressValue);
                    }
                });
                monitorTimer->start(250);

                QProcess *process = new QProcess();
                int attempts = 0;
                do {
                    process->start("pkexec", QStringList() << "bash" << "-c" << command);
                    process->waitForFinished();

                    if (process->exitCode() != 0) {
                        QProcess cleanProcess;
                        cleanProcess.start("pkexec", QStringList() << "bash" << "-c" <<
                            "pacman -Scc --noconfirm");
                        cleanProcess.waitForFinished();

                        cleanProcess.start("pkexec", QStringList() << "bash" << "-c" <<
                            "pacman -Sy");
                        cleanProcess.waitForFinished();

                        attempts++;
                    }
                } while (process->exitCode() != 0 && attempts < 3);
                qDebug() << "PROCESS-OUTPUT: " << process->exitCode();

                monitorTimer->stop();
                progress->setValue(100);

                bool success = (process->exitCode() == 0);
                process->kill();
                process->waitForFinished(3000);
                process->deleteLater();
                progress->deleteLater();
                monitorTimer->deleteLater();

                if (callback) callback(success);
                if (success)
                    internetChecker->deleteLater();
            });
        internetChecker->checkConnectivity();
    }
