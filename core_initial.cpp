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
#include <charconv>
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
    QFile osReleaseFile("/usr/lib/os-release");

    if (!osReleaseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "os-release failed to open" << osReleaseFile.errorString();
        return false;
    }

    QMap<QString, QString> expected = {
        {"NAME", "\"Xray_OS\""},
        {"PRETTY_NAME", "\"Xray OS\""},
        {"ID", "xray_os"},
        {"BUILD_ID", "rolling"},
        {"ANSI_COLOR", "\"38;2;23;147;209\""},
        {"HOME_URL", "\"https://xray-os.github.io/xray_os-website/index.html\""},
        {"DOCUMENTATION_URL", "\"https://xray-os.github.io/xray_os-website/get-started.html\""},
        {"SUPPORT_URL", "\"https://discord.com/invite/dBR7wR3ABk/\""},
        {"BUG_REPORT_URL", "\"https://github.com/Xray-OS/Xray_OS/issues\""},
        {"PRIVACY_POLICY_URL", "\"https://xray-os.github.io/xray_os-website/index.html#about-xray-os\""},
        {"LOGO", "xray-logo"},
        {"IMAGE_ID", "xray"}
    };

    // Get IMAGE_VERSION from tolitica.conf
    QString homeDir = QDir::homePath();
    QFile toliticaConf(homeDir+"/tolitica-home-settings/tolitica.conf");
    QString imageVersion;
    if (toliticaConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream confIn(&toliticaConf);
        while (!confIn.atEnd()) {
            QString line = confIn.readLine().trimmed();
            if (line.startsWith("xray_img_ver=")) {
                imageVersion = line.split("=")[1];
                break;
            }
        }
        toliticaConf.close();
    }

    if (imageVersion.isEmpty()) {
        imageVersion = "unknown"; // fallback
    }
    expected["IMAGE_VERSION"] = imageVersion;

    QTextStream in(&osReleaseFile);
    int matches = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList parts = line.split('=', Qt::KeepEmptyParts);
        if (parts.size() == 2) {
            QString key = parts[0];
            QString value = parts[1];
            if (expected.contains(key) && expected[key] == value) {
                matches++;
            }
        }
    }
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
            "sudo sed -i 's/IMAGE_ID=xray/IMAGE_ID=archlinux/g' /usr/lib/os-release",
            QString("sudo sed -i 's/^IMAGE_VERSION=.*/IMAGE_VERSION=%1/' /usr/lib/os-release").arg(imageVersion)
        };

        for (const QString &cmd : commands) {
            QProcess::execute("pkexec", QStringList() << "bash" << "-c" << cmd);
        }
    } else {
        QString homeDir = QDir::homePath();
        QFile confFile(homeDir + "/tolitica-home-settings/tolitica.conf");
        QString imageVersion = "v18"; // default xray fallback

        if (confFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream confIn(&confFile);
            while (!confIn.atEnd()) {
                QString line = confIn.readLine();
                if (line.startsWith("xray_img_ver = ")) {
                    imageVersion = line.split(" = ")[1];
                    break;
                }
            }
        }

        // Force set to Xray values regardless of current content
        QStringList commands = {
            "sudo sed -i 's/^NAME=.*/NAME=\"Xray_OS\"/' /usr/lib/os-release",
            "sudo sed -i 's/^PRETTY_NAME=.*/PRETTY_NAME=\"Xray OS\"/' /usr/lib/os-release",
            "sudo sed -i 's/^ID=.*/ID=xray_os/' /usr/lib/os-release",
            "sudo sed -i 's/^BUILD_ID=.*/BUILD_ID=rolling/' /usr/lib/os-release",
            "sudo sed -i 's/^ANSI_COLOR=.*/ANSI_COLOR=\"38;2;23;147;209\"/' /usr/lib/os-release",
            "sudo sed -i 's|^HOME_URL=.*|HOME_URL=\"https://xray-os.github.io/xray_os-website/index.html\"|' /usr/lib/os-release",
            "sudo sed -i 's|^DOCUMENTATION_URL=.*|DOCUMENTATION_URL=\"https://xray-os.github.io/xray_os-website/get-started.html\"|' /usr/lib/os-release",
            "sudo sed -i 's|^SUPPORT_URL=.*|SUPPORT_URL=\"https://discord.com/invite/dBR7wR3ABk/\"|' /usr/lib/os-release",
            "sudo sed -i 's|^BUG_REPORT_URL=.*|BUG_REPORT_URL=\"https://github.com/Xray-OS/Xray_OS/issues\"|' /usr/lib/os-release",
            "sudo sed -i 's|^PRIVACY_POLICY_URL=.*|PRIVACY_POLICY_URL=\"https://xray-os.github.io/xray_os-website/index.html#about-xray-os\"|' /usr/lib/os-release",
            "sudo sed -i 's/^LOGO=.*/LOGO=xray-logo/' /usr/lib/os-release",
            "sudo sed -i 's/^IMAGE_ID=.*/IMAGE_ID=xray/' /usr/lib/os-release",
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
            if (value == "Xray.profile") {
                return true;
            }
        }
    }
    return false;
}
// ################################################################
// Manual konsolerc profile toggle snippet
// Works fine, but QSettings is cleaner if you want less parsing
// ################################################################
// void CoreInitial::setKonsoleProfile() {
//     QString homeDir = QDir::homePath();
//     QFile konsolerecFile(homeDir + "/.config/konsolerc");

//     if (!konsolerecFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         if (!konsolerecFile.exists()) {
//             qWarning() << "The file is not present in: \"" << homeDir << "\",";
//             return;
//         }
//         qWarning() << "konsolerc can't be opened for edit" <<
//         konsolerecFile.errorString();
//         return;
//     }

//     // Get current profile value first
//     QString currentProfile;
//     QTextStream in(&konsolerecFile);
//     while (!in.atEnd()) {
//         QString line = in.readLine();
//         if (line.startsWith("DefaultProfile=")) {
//             currentProfile = line.mid(15).trimmed();
//             currentProfile.remove('"');
//             break;
//         }
//     }
//     konsolerecFile.close();

//     // Save current profile to tolitica.conf if it's not Xray_OS.profile
//     if (currentProfile != "Xray.profile") {
//         QFile toliticaConf(homeDir + "/tolitica-home-settings/tolitica.conf");
//         QStringList lines;

//         if (!toliticaConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
//             if (!toliticaConf.exists()) {
//                 qWarning() << "tolitica.conf is missing in: \"" <<
//                 homeDir << "\" ";
//                 return;
//             }
//             qWarning() << "tolitica.conf can't be opened" <<
//             toliticaConf.errorString();
//             return;
//         }

//         QTextStream tIn(&toliticaConf);
//         bool found = false;
//         while (!tIn.atEnd()) {
//             QString line = tIn.readLine();
//             if (line.startsWith("lastKonsoleProfile=")) {
//                 lines << "lastKonsoleProfile=" + currentProfile;
//                 found = true;
//             } else {
//                 lines << line;
//             }
//         }
//         if (!found) {
//             lines << "lastKonsoleProfile=" + currentProfile;
//         }
//         toliticaConf.close();

//         if (toliticaConf.open(QIODevice::WriteOnly | QIODevice::Text |
//             QIODevice::Truncate)) {
//             QTextStream tOut(&toliticaConf);
//             for (const QString &line : lines) {
//                 tOut << line << "\n";
//             }
//         }
//     }

//     // Now modify konsolerc
//     QStringList lines;
//     if (!konsolerecFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qWarning() << "Failed to reopen konsolerc";
//         return;
//     }

//     in.setDevice(&konsolerecFile);
//     bool foundDEsection = false;
//     bool foundDefaultProfile = false;

//     while (!in.atEnd()) {
//         QString line = in.readLine();

//         if (line.startsWith('[') && line.endsWith(']')) {
//             foundDEsection = (line.trimmed() == "[Desktop Entry]");
//             lines << line;
//         } else if (foundDEsection && line.trimmed().startsWith("DefaultProfile=")) {
//             if (currentProfile == "Xray.profile") {
//                 // Get saved profile from tolitica.conf
//                 QString savedProfile = "Arch.profile"; // default fallback
//                 QFile toliticaConf(homeDir + "/tolitica-home-settings/tolitica.conf");

//                 if (toliticaConf.open(QIODevice::ReadOnly | QIODevice::Text)) {
//                     QTextStream tIn(&toliticaConf);
//                     while (!tIn.atEnd()) {
//                         QString tLine = tIn.readLine();
//                         if (tLine.startsWith("lastKonsoleProfile=")) {
//                             savedProfile = tLine.mid(19).trimmed();
//                             break;
//                         }
//                     }
//                     toliticaConf.close();
//                 }
//                 lines << "DefaultProfile=" + savedProfile;
//             } else {
//                 lines << "DefaultProfile=Xray.profile";
//             }
//             foundDefaultProfile = true;
//         } else {
//             lines << line;
//         }
//     }

//     if (!foundDefaultProfile) {
//         lines << "DefaultProfile=Xray.profile";
//     }

//     konsolerecFile.close();

//     if (konsolerecFile.open(QIODevice::WriteOnly | QIODevice::Text |
//         QIODevice::Truncate)) {
//         QTextStream out(&konsolerecFile);
//         for (const QString &line : lines) {
//             out << line << "\n";
//         }
//         if (out.status() != QTextStream::Ok) {
//             qWarning() << "Failed to write konsolerc properly";
//         }
//     }
// }
// ################################################################
// END OF THE OLD FUNCTION (some code snippets for some fellow travelers)
// ################################################################

void CoreInitial::setKonsoleProfile() {
    QString homeDir = QDir::homePath();

    QSettings konsolerc(homeDir + "/.config/konsolerc", QSettings::IniFormat);

    // Get current profile value first
    QString currentProfile = konsolerc.value("Desktop Entry/DefaultProfile").toString();

    // Save current profile to tolitica.conf if it's not Xray_OS.profile
    if (currentProfile != "Xray.profile") {
       QSettings toliticaConf(homeDir +
            "/tolitica-home-settings/tolitica.conf", QSettings::IniFormat);
        toliticaConf.setValue("lastKonsoleProfile", currentProfile);
        toliticaConf.sync();
    }

    if (currentProfile == "Xray.profile") {
        QSettings toliticaConf(homeDir + "/tolitica-home-settings/tolitica.conf",
        QSettings::IniFormat);
        QString savedProfile = toliticaConf.value("lastKonsoleProfile", "Arch.profile").toString();
        konsolerc.setValue("Desktop Entry/DefaultProfile", savedProfile);
    } else {
        konsolerc.setValue("Desktop Entry/DefaultProfile", "Xray.profile");
    }

    konsolerc.sync();

    QFile file(homeDir + "/.config/konsolerc");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = file.readAll();
        file.close();

        // Replace any "%20" with a space (thanks QSettings)
        content.replace("%20", " ");

        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&file);
            out << content;
        }
    }
}

// QStringList CoreInitial::listGrubThemes() {

//     QStringList availableGrubThemes;
//     QDir grubThemesDir("/usr/share/grub/themes");

//     QFileInfoList dirList = grubThemesDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

//     for (int i = 0; i < dirList.size(); ++i) {
//         QFileInfo dirInfo = dirList.at(i);

//         if (themeDir.exists("theme.txt")) {
//             availableGrubThemes.append(QString(dirInfo.fileName()));
//         }
//     }

//     return availableGrubThemes;
// }

bool CoreInitial::grubThemeStatus() {
    QFile configFile("/etc/default/grub");

    if(!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&configFile);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Skip empty of commented lines
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        if (line.startsWith("GRUB_THEME=")) {
            return true;
        }
    }
    return false;
}

QStringList CoreInitial::listGrubThemes() {
    QDir grubThemesDir("/usr/share/grub/themes");
    QStringList dirs = grubThemesDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList availableGrubThemes;

    for (const QString &dirName : dirs) {
        QDir themeDir(grubThemesDir.absoluteFilePath(dirName));
        if (themeDir.exists("theme.txt")) {
            availableGrubThemes.append(dirName);
        }
    }
    return availableGrubThemes;
}

QString CoreInitial::currentGrubTheme() {
    QFile configFile("/etc/default/grub");

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Error while trying to retrieve the current grub theme:"
                   << configFile.errorString();
        return "Error";
    }

    QTextStream in(&configFile);
    QString value = "No theme present";

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }

        // Active theme
        if (line.startsWith("GRUB_THEME=")) {
            value = line.mid(QString("GRUB_THEME=").length()).trimmed();

            if (value.startsWith('"') && value.endsWith('"')) {
                value = value.mid(1, value.length() - 2);
            }
            if (value.endsWith("/theme.txt")) {
                value.chop(QString("/theme.txt").length());
            }

            QFileInfo fi(value);
            value = fi.fileName();
            break;
        }

        // Disabled theme
        if (line.startsWith("#GRUB_THEME=")) {
            value = line.mid(QString("#GRUB_THEME=").length()).trimmed();

            if (value.startsWith('"') && value.endsWith('"')) {
                value = value.mid(1, value.length() - 2);
            }
            if (value.endsWith("/theme.txt")) {
                value.chop(QString("/theme.txt").length());
            }

            QFileInfo fi(value);
            value = fi.fileName() + " (Disabled)";
            break;
        }
    }

    return value;
}

void CoreInitial::setGrubTheme(const QString &grubTheme) {
    qDebug() << "\n=== setGrubTheme CALLED ===";
    qDebug() << "Theme parameter:" << grubTheme;

    bool status = grubThemeStatus();
    qDebug() << "Current grub theme status (enabled):" << status;

    QFile configFile("/etc/default/grub");
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Error while trying to retrieve the current grub theme:"
        << configFile.errorString();
        return;
    }

    QTextStream in(&configFile);
    bool foundGrubTheme = false;

    while(!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("GRUB_THEME=") || line.startsWith("#GRUB_THEME=")) {
            foundGrubTheme = true;
            qDebug() << "Found existing GRUB_THEME line:" << line;
            break;
        }
    }
    configFile.close();
    qDebug() << "Found existing grub theme line:" << foundGrubTheme;

    // For change button: always enable the new theme
    // For disable button: toggle current theme on/off
    QString command;
    if (foundGrubTheme) {
        // Check if this is being called from disable button (same theme) or change button (different theme)
        QString currentThemeName = currentGrubTheme();
        if (currentThemeName.contains(" (Disabled)")) {
            currentThemeName = currentThemeName.replace(" (Disabled)", "");
        }

        if (grubTheme == currentThemeName) {
            // This is disable/enable toggle - use old logic
            command = status
                ? QString("sed -i 's|^GRUB_THEME=.*|#GRUB_THEME=\"/boot/grub/themes/%1/theme.txt\"|' /etc/default/grub")
                    .arg(grubTheme)
                : QString("sed -i 's|^#GRUB_THEME=.*|GRUB_THEME=\"/boot/grub/themes/%1/theme.txt\"|' /etc/default/grub")
                    .arg(grubTheme);
        } else {
            // This is theme change - always enable
            command = QString("sed -i 's|^#*GRUB_THEME=.*|GRUB_THEME=\"/boot/grub/themes/%1/theme.txt\"|' /etc/default/grub")
                .arg(grubTheme);
        }
    } else {
        command = QString("echo 'GRUB_THEME=\"/boot/grub/themes/%1/theme.txt\"' >> /etc/default/grub")
            .arg(grubTheme);
        qDebug() << "No existing theme line found, will add new one";
    }

    qDebug() << "Command to execute:" << command;
    qDebug() << "Starting process...";

    QString fullCommand = command + " && grub-mkconfig -o /boot/grub/grub.cfg";
    qDebug() << "Full command with grub-mkconfig:" << fullCommand;

    QProcess process;
    process.start("pkexec", QStringList() << "bash" << "-c" << fullCommand);
    process.waitForFinished();

    qDebug() << "Process finished with exit code:" << process.exitCode();
    qDebug() << "Process stdout:" << process.readAllStandardOutput();
    qDebug() << "Process stderr:" << process.readAllStandardError();
    qDebug() << "=== setGrubTheme FINISHED ===\n";
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
    process.start("bash", QStringList() << "-c" << "pacman -Q xray-gaming-meta");
    process.waitForFinished();

    bool gamingEnabled = (process.exitCode() == 0);

    return gamingEnabled ? true : false;
}

void CoreInitial::getXrayGamingMeta(QWidget *parent,
    std::function<void(bool)>callback) {
        bool status = gamingMetaStatus();

        QString command = (status) ? "pacman -Rns --noconfirm xray-gaming-meta"
            : "pacman -S --noconfirm xray-gaming-meta";

        QProgressDialog *progress = new QProgressDialog(
            (status) ? "Removing Xray Gaming Meta..."
            : "Installing Xray Gaming Meta", nullptr, 0, 100, parent);
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
