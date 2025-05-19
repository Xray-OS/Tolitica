#include "widget.h"
#include "./ui_widget.h"
#include <QStackedWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QProgressDialog>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>

// CUSTOM CLASSES
#include "core_functions.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE TWEAKS PAGE /////////////////////// /////////////////////// ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// TWEAKS::CLEAN ORPHANS FUCNTION
//////////////////////////////////////////////////
void Widget::cleanOrphans() {
    // Check if there are orphans to clean
    QProcess checkOrphans;
    checkOrphans.start("bash", QStringList() << "-c" << "pacman -Qtdq");
    checkOrphans.waitForFinished();

    QString orphanOutput = checkOrphans.readAllStandardOutput().trimmed();

    if(orphanOutput.isEmpty()) {
        QMessageBox::information(this, "No orphans Found", "There are no orphaned packages to clean!");
        return; // Exit fucntion early if nothing to remove
    }

    // Check if yay is installed
    QProcess checkYayProcess;
    checkYayProcess.start("bash", QStringList() << "-c" << "command -v yay");
    checkYayProcess.waitForFinished();

    bool yayInstalled = (checkYayProcess.exitStatus() == QProcess::NormalExit && !checkYayProcess.readAllStandardOutput().isEmpty());

    // Create the command
    QStringList args;
    if (yayInstalled) {
        args << "bash" << "-c" << "yay -Yc --noconfirm";
    } else {
        args << "bash" << "-c" << "pacman -Rns $(pacman -Qtdq) --noconfirm";
    }

    // Run the cleanup process
    QProcess cleanOrph;
    cleanOrph.start("pkexec", args);
    cleanOrph.waitForFinished();

    // Capture error messages
    QString errorOutput = cleanOrph.readAllStandardError();

    if (cleanOrph.exitStatus() == QProcess::NormalExit && cleanOrph.exitCode() == 0) {
        QMessageBox::information(this, "Unused dependencies Removed!", "All unused dependencies have been removed from your system!");

    } else {
        QMessageBox::warning(this, "Error", "Something wnet wrong:\n" + errorOutput);
    }
}

///////////////////////////////////////////////////
/// TWEAKS::CLEAN PACKAGE CACHE FUNCTION
//////////////////////////////////////////////////

void Widget::cleanPkgCache() {
    QString userName = qgetenv("USER");
    if (userName.isEmpty()) {
        userName = QDir::homePath().section("/", -1);
    }

    QStringList cacheDirs = {
        "/home/" + userName + "/.cache/yay",
        "/var/cache/pacman/pkg",
        "/var/lib/pacman"
    };

    QStringList excludeFiles = { "completion.cache", "vcs.json" };
    QStringList excludeDirs = { "local", "sync" };

    bool filesFound = false;

    for (const QString& dirPath : cacheDirs) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }

        QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
        for (int i = 0; i < fileList.size(); ++i) {
            const QFileInfo &entry = fileList.at(i);
            QString name = entry.fileName();

            if (excludeFiles.contains(name) || excludeDirs.contains(name)) {
                continue;
            }
            filesFound = true;

            // Ensure files owned by root are deleted with pkexec
            if (entry.isDir()) {
                if (!QDir(entry.absoluteFilePath()).removeRecursively()) {
                    QProcess::execute("pkexec", QStringList() << "rm" << "-f" << entry.absoluteFilePath());
                }
            }
        }
    }

    if (!filesFound) {
        QMessageBox::information(this, "Package Cache Clean", "No package cache was found!");
        return;
    }
    QMessageBox::information(this, "Package Cache Cleaned!", "All unnecessary package cache files have been removed successfully!");
}

///////////////////////////////////////////////////
/// TWEAKS::SYSTEM UPDATE FUNCTION
//////////////////////////////////////////////////
void Widget::systemUpdate() {
    QProcess *sysUp = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this); // High-frequency monitoring

    // Create the process bar dynamically
    QProgressDialog *progress = new QProgressDialog("Updating system...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // Agressive monitoring (every 250ms)
    connect(monitorTimer, &QTimer::timeout, this, [=]() mutable {
        // Read CPU load from /proc/loadavg
        QFile cpuFile("/proc/loadavg");
        if (cpuFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&cpuFile);
            QStringList loadValues = in.readLine().split(" ");
            cpuFile.close();

            double cpuLoad = loadValues[0].toDouble(); // First value represents recent system load

            // Read disk activity from /proc/diskstats
            QFile diskFile("/proc/diskstats");

            if (diskFile.open(QIODevice::ReadOnly)) {
                QTextStream diskStream(&diskFile);
                QString diskData = diskStream.readAll();
                diskFile.close();

                int diskOps = diskData.count("\n"); // Approximate active disk operations

                // Dynamically adjust progress speed
                int loadFactor = static_cast<int>((cpuLoad * 10) + (diskOps % 10)); // Simple scaling

                progressValue += qMin(loadFactor, 5); // Limit aggressive increments
                progress->setValue(qMin(progressValue, 95)); // Prevent overflow
            }
        }
        // Stop tracking when update process finishes
        if(sysUp->state() != QProcess::Running) {
            monitorTimer->stop();
        }
    });

    // Detect when process finishes
    connect(sysUp, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=] (int exitCode,
                                                                                            QProcess::ExitStatus status) {
        QString output = QString::fromUtf8(sysUp->readAllStandardOutput());

        if (output.contains("Nothing to do") && output.contains("there is nothing to do")) {
            QMessageBox::information(nullptr, "System Already Upto Date", "No updates were found");
        } else {
            progress->setValue(100);
            QMessageBox::information(nullptr, "Ada Has Been Updated", "Your Operating System has been updated successfully");
        }

        // Cleanup Memory
        progress->deleteLater();
        sysUp->deleteLater();
        monitorTimer->deleteLater(); // Stop aggressive monitoring
    });

    // Start update process
    sysUp->start("pkexec", QStringList() << "bash" << "-c" << "pacman -Syu --noconfirm && flatpak update --assumeyes");

    // Start aggressive monitoring
    monitorTimer->start(250); // Updates every 250ms
}

///////////////////////////////////////////////////
/// TWEAKS::REMOVE db.lck FUNCTION
//////////////////////////////////////////////////
void Widget::removeDBLock() {
    QProcess checkPacman;
    checkPacman.start("pgrep pacman", QStringList() << "pacman", QIODevice::ReadWrite); // Check if pacman is running
    checkPacman.waitForFinished();

    QString output = QString::fromUtf8(checkPacman.readAllStandardOutput()).trimmed();

    QString dblckFilePath = { "/var/lib/pacman/db.lck" };

    bool dblckExists = false;

    // Check if db.lck exists
    QFile dblckFile(dblckFilePath);

    if (!dblckFile.exists()) {
        QMessageBox::information(this, "No db.lck Found", "Pacman is not currently locked, no actions needs to be taken.");
        return;
    }

    // Ensure pacman isn't running
    if (!output.isEmpty()) {
        QMessageBox::warning(this, "Pacman is Running", "Pacman is currently in use. Please wait for it to finish before removing the lock");
        return;
    }

    QProcess unlockPacman;
    unlockPacman.start("pkexec", QStringList() << "rm " << "-f" << "/var/lib/pacman/db.lck", QIODevice::ReadWrite); // Remove lock safely
    unlockPacman.waitForFinished();

    QMessageBox::information(this, "Lock Removed", "Pacman database lock has been successfully removed.");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE ADDONS PAGE /////////////////////// /////////////////////// ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// ADDONS::INSTALL ADA-GAMING-META FUNCTION
//////////////////////////////////////////////////
void Widget::adaGamingMeta() {
    QProcess *installAGM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this); // High-frequency monitoring

    // Create the progress bar dynamically
    QProgressDialog *progress = new QProgressDialog("Installing Ada Gaming Meta...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // **Fix: Real-time progress update using process output**
    connect(installAGM, &QProcess::readyReadStandardOutput, this, [=]() mutable {

        // Adjust progress dynamically based on output
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));

        QCoreApplication::processEvents(); // Ensure UI refresh
    });

    // **Fix: Reliable package detection using `exitCode()`**
    connect(installAGM, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]
            (int exitCode, QProcess::ExitStatus status){

                // Check if the package is already installed
                QProcess checkInstalled;
                checkInstalled.start("bash", QStringList() << "-c" << "pacman -Q ada-gaming-meta");
                checkInstalled.waitForFinished();  // Ensure process completes before reading output

                if (checkInstalled.exitCode() == 0) {
                    // Package is already installed
                    progress->setValue(100);
                    QMessageBox::information(nullptr, "Ada Gaming Meta Already Installed",
                                             "Ada Gaming Meta packages are already installed on your system.");
                } else {
                    // Package was just installed
                    progress->setValue(100);
                    QMessageBox::information(nullptr, "Ada Gaming Meta Has Been Installed",
                                             "Ada Gaming Meta packages were installed successfully!");
                }

                // Cleanup Memory
                progress->deleteLater();
                installAGM->deleteLater();
                monitorTimer->deleteLater(); // Stop aggressive monitoring
            });

    // Start installing process
    installAGM->start("pkexec", QStringList() << "bash" << "-c" << "pacman -S ada-gaming-meta --noconfirm");

    // Start aggressive monitoring
    monitorTimer->start(250); // Updates every 250ms
}
///////////////////////////////////////////////////
/// ADDONS::REMOVE ADA-GAMING-META FUNCTION
//////////////////////////////////////////////////
void Widget::removeAdaGamingMeta() {
    QProcess *removeAGM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this);

    QProgressDialog *progress = new QProgressDialog("Removing Ada Gaming Meta...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // Ensure yay detection
    QProcess checkYay;
    checkYay.start("bash", QStringList() << "-c" << "pacman -Q yay");
    checkYay.waitForFinished();
    bool yayInstalled = (checkYay.exitCode() == 0);

    QStringList removeCommands = {
        "pacman -R ada-gaming-meta --noconfirm",
        "pacman -R steam-native-runtime --noconfirm",
        "pacman -R steam lutris bottles dosbox goverlay heroic-games-launcher-bin portproton protonplus protonup-qt vkbasalt gamemode lib32-gamemode q4wine-git wine-gecko wine-mono wine-nine protontricks --noconfirm",
        "pacman -R winetricks wine-staging --noconfirm",
        "pacman -R gamescope mangohud vkd3d proton-ge-custom-bin --noconfirm",
        "pacman -R lib32-vkd3d vkbasalt-cli reshade-shaders-git --noconfirm"
    };

    if (yayInstalled) {
        removeCommands.append("yay -Yc --noconfirm");
    } else {
        removeCommands.append("pacman -Rns $(pacman -Qtdq) --noconfirm");
    }

    qDebug() << "Package removal list:";
    for (const auto &cmd : removeCommands) {
        qDebug() << cmd;
    }

    int currentStep = 0;

    // **Loop Execution: Process Commands Sequentially**
    connect(removeAGM, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]() mutable {
        if (currentStep < removeCommands.size()) {
            qDebug() << "Executing command:" << removeCommands[currentStep];

            removeAGM->start("pkexec", QStringList() << "bash" << "-c" << removeCommands[currentStep]);

            progressValue += (100 / removeCommands.size()); // Update progress dynamically
            progress->setValue(qMin(progressValue, 95));
            QCoreApplication::processEvents();

            currentStep++; // Move to next command in sequence
        } else {
            qDebug() << "All packages removed successfully!";
            progress->setValue(100);
            QMessageBox::information(nullptr, "Ada Gaming Meta Removed",
                                     "Ada Gaming Meta packages have been successfully removed.");
            progress->deleteLater();
            removeAGM->deleteLater();
            monitorTimer->deleteLater();
        }
    });

    removeAGM->start("pkexec", QStringList() << "bash" << "-c" << removeCommands[currentStep]); // Start first command
    monitorTimer->start(250);
}

///////////////////////////////////////////////////
/// ADDONS:: ADA-DEVELOPMENT-META FUNCTION
//////////////////////////////////////////////////
void Widget::adaDevelopmentMeta() {
    QProcess *installADM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this); // High-frequency monitoring

    // Create the progress bar dynamically
    QProgressDialog *progress = new QProgressDialog("Installing Ada Development Meta...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // ** Fix: Real-time progress update using process output **
    connect(installADM, &QProcess::readyReadStandardOutput, this, [=]() mutable {

    // Adjust progress dynamically based on output
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));

        QCoreApplication::processEvents(); // Ensure UI refresh
    });

    // **Fix: Reliable package detection using 'exitCode()'**
    connect(installADM, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]
            (int exitCode, QProcess::ExitStatus status) {
                // Check if the package is already installed
        QProcess checkInstalled;
        checkInstalled.start("bash", QStringList() << "-c" << "pacman -Q ada-development-meta");
        checkInstalled.waitForFinished(); // Ensure process completes before reading output

        if (checkInstalled.exitCode() == 0) {
            // Package is already installed
            progress->setValue(100);
            QMessageBox::information(nullptr, "Ada Development Meta Already Installed",
                                              "Ada Development Meta packages are already installed on your system");
        } else {
            // Package was just installed
            progress->setValue(100);
            QMessageBox::information(nullptr, "Ada Development Meta has been installed",
                                     "Ada Development Meta packages were installed successfully!");
        }

        // Cleanup Memory
        progress->deleteLater();
        installADM->deleteLater();
        monitorTimer->deleteLater();
    });

    // Start installing process
    installADM->start("pkexec", QStringList() << "bash" << "-c" << "pacman -S ada-development-meta "
                                                 "--noconfirm");
    // Start aggressive monitoring
    monitorTimer->start(250); // Updates every 250ms
};

///////////////////////////////////////////////////
/// ADDONS:: REMOVE ADA-DEVELOPMENT-META FUNCTION
//////////////////////////////////////////////////
void Widget::removeAdaDevelopmentMeta() {
    QProcess *removeADM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this);

    QProgressDialog *progress = new QProgressDialog("Removing Ada Development Meta...", nullptr, 0, 100,
                                                    this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // Ensure yay detection
    QProcess checkYay;
    checkYay.start("bash", QStringList() << "-c" << "pacman -Q yay");
    checkYay.waitForFinished();

    bool yayInstalled = (checkYay.exitCode() == 0);

    QStringList removeCommands = {
        "pacman -R ada-development-meta --noconfirm",
        "pacman -R geany-themes --noconfirm",
        "pacman -R geany visual-studio-code-bin zed jetbrains-toolbox github-desktop sublime-text-4 --noconfirm"
    };

    if (yayInstalled) {
        removeCommands.append("yay -Yc --noconfirm");
    } else {
        removeCommands.append("pacman -Rns &(pacman -Qtdq) --noconfirm");
    }

    int CurrentStep = 0;

    // **Loop Execution: Process Commands Sequentially**
    connect(removeADM, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]()
            mutable {
        if (CurrentStep <removeCommands.size()) {
            removeADM->start("pkexec", QStringList() << "bash" << "-c" <<
                                           removeCommands[CurrentStep]);

            progressValue += (100 / removeCommands.size()); // Update progress dynamically
            progress->setValue(qMin(progressValue, 95));
            QCoreApplication::processEvents();

            CurrentStep++; // Move to next command in sequence
        } else {
            progress->setValue(100);
            QMessageBox::information(nullptr, "Ada Development Meta Removed",
                                     "Ada Development Meta packages have been successfully removed");
            progress->deleteLater();
            removeADM->deleteLater();
            monitorTimer->deleteLater();
        }
    });

    removeADM->start("pkexec", QStringList() << "bash" << "-c" << removeCommands[CurrentStep]); // Start first commands
    monitorTimer->start(250);
}

///////////////////////////////////////////////////
/// ADDONS:: CHECK CHAOTIC-AUR STATUS
//////////////////////////////////////////////////
int Widget::checkChaoticAURStatus() {
    QProcess checkChaotic;

    // Check if the key is properly imported
    checkChaotic.start("bash", QStringList() << "-c" << "pacman-key --list-keys 3056513887B78AEB");
    checkChaotic.waitForFinished();
    bool keyExist = (checkChaotic.exitCode() == 0);

    // Check if the required packages are installed
    checkChaotic.start("bash", QStringList() << "-c" << "pacman -Q chaotic-keyring chaotic-mirrorlist");
    checkChaotic.waitForFinished();
    bool packagesInstalled = (checkChaotic.exitCode() == 0);

    // **Fix: Check if repo exists in pacman.conf
    // **Check if [chaotic-aur] header exists**
    checkChaotic.start("bash", QStringList() << "-c" << "grep -q '\\[chaotic-aur\\]' /etc/pacman.conf");
    checkChaotic.waitForFinished();
    bool repoHeaderExists = (checkChaotic.exitCode() == 0);

    // **Check if Include line exists**
    checkChaotic.start("bash", QStringList() << "-c" << "grep -q 'Include = /etc/pacman.d/chaotic-mirrorlist' /etc/pacman.conf");
    checkChaotic.waitForFinished();
    bool includeLineExists = (checkChaotic.exitCode() == 0);

    // **Refined Status Logic**
    if (keyExist && packagesInstalled && repoHeaderExists && includeLineExists) {
        return 1; // Fully configured
    } else if (!keyExist && !packagesInstalled && !repoHeaderExists && !includeLineExists) {
        return 2; // No setup detected
    } else {
        return 3; // Partial setup detected (Repair needed)
    }
}

///////////////////////////////////////////////////
/// ADDONS:: BACKUP PACMAN CONFIG FUNCTION
//////////////////////////////////////////////////
void Widget::backupPacmanConfig() {
    int status = checkChaoticAURStatus();

    if (status != 1) {
        return; // Backup is only made when status is 1 (working)
    }

    QProcess createBackupDir;
    createBackupDir.start("pkexec", QStringList() << "bash" << "-c" << "mkdir -p /usr/share/tolitica-backups");
    createBackupDir.waitForFinished();

    QString errorOutput = createBackupDir.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        QMessageBox::warning(this, "Error", "Failed to create backup directory:\n" + errorOutput);
    }

    QProcess backupCheck;
    backupCheck.start("bash", QStringList() << "-c" << "cmp -s /etc/pacman.conf /usr/share/tolitica-backups/pacman.conf || pkexec cp -r /etc/pacman.conf /usr/share/tolitica-backups/");
    backupCheck.waitForFinished();
}

///////////////////////////////////////////////////
/// ADDONS:: CHAOTIC-AUR
//////////////////////////////////////////////////
void Widget::chaoticAUR() {
    int status = checkChaoticAURStatus();

    if (status == 1)
    {
        removeChaoticAUR();
    }

    QProcess addChaoticAUR;

    if(status == 2) {
        addChaoticAUR.start("pkexec", QStringList() << "bash" << "-c" <<
                            "pacman-key --recv-key 3056513887B78AEB --keyserver keyserver.ubuntu.com && "
                            "pkexec pacman-key --lsign-key 3056513887B78AEB && "
                            "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-keyring.pkg.tar.zst --noconfirm && "
                            "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-mirrorlist.pkg.tar.zst --noconfirm &&"
                            "echo -e '\\n[chaotic-aur]\\nInclude = /etc/pacman.d/chaotic-mirrorlist' | pkexec tee -a /etc/pacman.conf");
        addChaoticAUR.waitForFinished();
        backupPacmanConfig();

        if(addChaoticAUR.exitCode() == 0) {
            QMessageBox::information(this, "Chaotic AUR Added", "Chaotic AUR repositories has been successfully setup in Ada");
        } else {
            QMessageBox::warning(this, "error", "error: " + addChaoticAUR.readAllStandardError());
        }
    }
    //**Fix broken repo entries if Chaotic AUR is partially broken or if the repo is messed
    if (status == 3) {
        QProcess restoreChaotic;

        if(!QFile::exists("/etc/pacman.d/chaotic-mirrorlist")) {
            QProcess removeRepo;
            removeRepo.start("pkexec", QStringList() << "bash" << "-c" << "sed -i '/\\[chaotic-aur\\]/,+1d' /etc/pacman.conf");
            removeRepo.waitForFinished();
;        }
        restoreChaotic.start("pkexec", QStringList() << "bash" << "-c" << "pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-mirrorlist.pkg.tar.zst --noconfirm &&"
                                                                          "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-keyring.pkg.tar.zst --noconfirm &&"
                                                                          "pacman-key --recv-key 3056513887B78AEB --keyserver keyserver.ubuntu.com && "
                                                                          "pkexec pacman-key --lsign-key 3056513887B78AEB");
        restoreChaotic.waitForFinished();

        restoreChaotic.start("pkexec", QStringList() << "bash" << "-c" << "cp -r /usr/share/tolitica-backups/pacman.conf /etc/pacman.conf");
        restoreChaotic.waitForFinished();

        if(restoreChaotic.exitCode() == 0)
        {
            QMessageBox::information(this, "Restore complete", "Chaotic AUR has been repaired!");
        } else {
            QMessageBox::warning(this, "Error", "Something went wrong restoring pacman.conf" + restoreChaotic.readAllStandardError());
        }
    }

    // **Fix broken repo entries if Chaotic AUR is partially broken or if the repo is messed**

}

///////////////////////////////////////////////////
/// ADDONS:: REMOVE-CHAOTIC-AUR
//////////////////////////////////////////////////
void Widget::removeChaoticAUR() {
    QProcess removeChaoticAUR;
    removeChaoticAUR.start("pkexec", QStringList() << "bash" << "-c" << "sed -i '/\\[chaotic-aur\\]/,+1d' /etc/pacman.conf && "
                           "pkexec pacman -Rns chaotic-keyring chaotic-mirrorlist --noconfirm &&"
                           "pkexec pacman-key --delete 3056513887B78AEB");
    removeChaoticAUR.waitForFinished();
    // deleting pacman.conf from /usr/share/tolitica-backups if it was already created
    if (QFile::exists("/usr/share/tolitica-backups/pacman.conf")) {
        QProcess delPacman;

        delPacman.start("pkexec", QStringList() << "bash" << "-c" << "rm -r /usr/share/tolitica-backups/pacman.conf");
        delPacman.waitForFinished();
    }

    // Capture standard error output
    QString errorOutput = removeChaoticAUR.readAllStandardError();

    if (removeChaoticAUR.exitCode() == QProcess::NormalExit && removeChaoticAUR.exitCode() == 0) {
        QMessageBox::information(this, "Chaotic AUR Removed", "Chaotic AUR repositories have been removed successfully");
    } else {
        QMessageBox::warning(this, "Error", "Something went wrong removing Chaotic AUR repositories" + errorOutput);
    }
}

///////////////////////////////////////////////////
/// ADDONS:: CHECK VMWARE SERVICES STATUS
//////////////////////////////////////////////////
bool Widget::vmwareServiceStatus() {
    QStringList services = {
        "vmware-networks.service",
        "vmware-usbarbitrator.service"
    };

    bool allServicesActive = true;

    for (const QString &service : services) {
        QProcess checkEnabled;
        checkEnabled.start("bash", QStringList() << "-c" << "systemctl is-enabled " + service);
        checkEnabled.waitForFinished();
        bool isEnabled = (checkEnabled.readAllStandardOutput().trimmed() == "enabled");

        QProcess checkActive;
        checkActive.start("bash", QStringList() << "-c" << "systemctl is-active " + service);
        checkActive.waitForFinished();
        bool isActive = (checkActive.readAllStandardOutput().trimmed() == "active");

        if (!isActive || !isEnabled) {
            allServicesActive = false;
            break;
        }

    }

    return allServicesActive;
}

///////////////////////////////////////////////////
/// ADDONS:: CHECK VMWARE STATUS
//////////////////////////////////////////////////
int Widget::vmwareStatus() {
    QProcess checkVMware;

    // Check if the pkg is installed
    checkVMware.start("bash", QStringList() << "-c" << "pacman -Q vmware-workstation");
    checkVMware.waitForFinished();

    bool pkgInstalled = (checkVMware.exitCode() == 0);

    if (pkgInstalled && vmwareServiceStatus()) {
        return 0;
    }
    if (!pkgInstalled  && !vmwareServiceStatus())
    {
        return 1;
    }
    else {
        return 2;
    }
}

///////////////////////////////////////////////////
/// ADDONS:: ADD VMWARE SUPPORT
//////////////////////////////////////////////////
void Widget::addVMware(QPushButton *vmwButton) {

    QProcess vmInstalled;
    vmInstalled.start("bash", QStringList() << "-c" << "pacman -Q vmware-workstation");
    vmInstalled.waitForFinished();

    bool pkgInstalled = (vmInstalled.exitCode() == 0);

    if (!pkgInstalled) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "VMware Workstation is not installed", "Do you want to install it?");
        if (reply == QMessageBox::No) {
            return;
        }

        QProcess *installVMware = new QProcess(this);
        QTimer *monitorTimer = new QTimer(this);

        // Create the progress bar dynamically
        QProgressDialog *progress = new QProgressDialog("Installing VMware Workstation...", nullptr, 0, 100, this);
        progress->setWindowModality(Qt::WindowModal);
        progress->setCancelButton(nullptr);
        progress->show();

        int progressValue = 0;

        // **Real-Time progress update using process output**
        connect(installVMware, &QProcess::readyReadStandardOutput, this, [=]() mutable {
            progressValue += 5;
            progress->setValue(qMin(progressValue, 95));
            QCoreApplication::processEvents();
        });

        // **Update the button immediately hen installation is completed**
        connect(installVMware, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]()
                mutable {
                    QProcess checkInstalled;
                    checkInstalled.start("bash", QStringList() << "-c" << "pacman -Q vmware-workstation");
                    checkInstalled.waitForFinished();

                    if (checkInstalled.exitCode() == 0) {
                        progress->setValue(100);
                        QMessageBox::information(nullptr, "VMware Workstation Installed", "VMware Workstation was installed successfully!");

                        // ** Refresh the button text dynamically**
                        int updatedStatus = vmwareStatus();
                        vmwButton->setText(updatedStatus == 0 ? "Remove VMware Workstation" : "Install/Enable VMware Workstation");

                    } else {
                        progress->setValue(100);
                        QMessageBox::warning(nullptr, "Error", "Something went wrong installing VMware Workstation");

                    }

                    // Cleanup
                    progress->deleteLater();
                    installVMware->deleteLater();
                    monitorTimer->deleteLater();
                });

        // Start installation
        installVMware->start("pkexec", QStringList() << "bash" << "-c" << "pacman -S vmware-workstation --noconfirm");
        monitorTimer->start(250);
    }
    if (!vmwareServiceStatus()) {
        QProcess enableServices;
        enableServices.start("pkexec", QStringList() << "bash" << "-c" << "systemctl enable vmware-networks-configuration.service && pkexec systemctl start vmware-networks-configuration.service &&"
                                                                          "pkexec systemctl enable vmware-networks.service && pkexec systemctl start vmware-networks.service &&"
                                                                          "pkexec systemctl enable vmware-usbarbitrator.service && pkexec systemctl start vmware-usbarbitrator.service");
        enableServices.waitForFinished();
        QMessageBox::information(this, "Services Enabled and Active",
                                 "All VMware Workstation services has been Activated successfully");
        int updatedStatus = vmwareStatus();
        vmwButton->setText(updatedStatus == 0 ? "Remove VMware Workstation" : "Install/Enable VMware Workstation");
    }

}

///////////////////////////////////////////////////
/// ADDONS:: REMOVE VMWARE
//////////////////////////////////////////////////
void Widget::removeVMware(QPushButton *vmwButton) {
    QProcess *removeVMware = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this);

    QProgressDialog *progress = new QProgressDialog("Removing VMware Workstation...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    connect(removeVMware, &QProcess::readyReadStandardOutput, this, [=]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents();
    });

    connect(removeVMware, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]() mutable {
        QProcess checkRemoved;
        checkRemoved.start("bash", QStringList() << "-c" << "pacman -Q vmware-workstation");
        checkRemoved.waitForFinished();

        if (checkRemoved.exitCode() != 0) {
            progress->setValue(100);
            QMessageBox::information(nullptr, "VMware Workstation Removed",
                                     "VMware Workstation has been removed successfully");
            // **Update button dynamically**
            int updatedStatus = vmwareStatus();
            vmwButton->setText(updatedStatus == 0 ? "Remove VMware Workstation" : "Install/Enable VMware Workstation");
        } else {
            progress->setValue(100);
            QMessageBox::warning(nullptr, "Error", "Something went wrong removing VMware Workstation!");
        }

        progress->deleteLater();
        removeVMware->deleteLater();
        monitorTimer->deleteLater();
    });

    removeVMware->start("pkexec", QStringList() << "bash" << "-c" << "pacman -Rns vmware-workstation --noconfirm");
    monitorTimer->start(250);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE TERMINAL PAGE /////////////////////// /////////////////////// ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// TERMINAL: CHECK TERMINAL-THEMING STATUS
//////////////////////////////////////////////////
int Widget::checkTermThemingStatus(){
    QString termConfigFile = QDir::homePath() + "/.config/fish/config.fish";
    QFile configFile(termConfigFile);

    if (!configFile.exists()) {
        return 0; // File not found
    }
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0; // Treat as not found due to error
    }

    QString targetLine = "oh-my-posh init fish --config $HOME/.config/oh-my-posh-themes/ada-atomic.omp.json";
    QTextStream in(&configFile);

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains(targetLine)) {
            configFile.close();
            return line.startsWith("#") ? 2 : 1; // 2 if commented, 1 if uncommented
        }
    }

    configFile.close();
    return 0; // Line not found
}

////////////////////////////////////////////////////////
/// TERMINAL: DISABLE/ENABLE TERMINAL THEMING FUNCTION
//////////////////////////////////////////////////////
void Widget::disableTermTheme(QPushButton *terminalThemeButton) {
    QString termConfigFile = QDir::homePath() + "/.config/fish/config.fish";
    QFile configFile(termConfigFile);

    int status = checkTermThemingStatus(); // Get the current line status

    // Handle the case were the file or target line is missing
    if (status == 0) {
        terminalThemeButton->setText("No config.fish found");
        QMessageBox::warning(this, "No Oh-My-Posh theme Found!",
                                    "No current config.fish file was found in '$HOME/.config/fish'");
        return;
    }

    // If there is no actual ada-atomic.omp.json file
    if (!QFile::exists(QDir::homePath() + "/.config/oh-my-posh-themes/ada-atomic.omp.json")) {
        QMessageBox::warning(this, "ada-atomic.omp.json is Missing!",
                                   "ada-atomic.omp.json is missing from /.config/oh-my-posh-themes");
        return;
    }

    // Confirm action with the user
    QMessageBox msgBox;
    msgBox.setWindowTitle("Terminal Theming");
    msgBox.setIcon(QMessageBox::Information);

    if (status == 1) {
        msgBox.setText("Terminal theming is currently enabled.\nWould you like to disable it?");
    } else if (status == 2) {
        msgBox.setText("Terminal theming is currently disabled\nWould you like to enable it?");
    }

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    int userChoice = msgBox.exec();

    if (userChoice != QMessageBox::Yes) {
        return; // User canceled the action
    }

    // Attempt to open the file for reading
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to read configuration file.");
        return;
    }

    QStringList lines;
    QString targetLine = "oh-my-posh init fish --config $HOME/.config/oh-my-posh-themes/ada-atomic.omp.json";

    QTextStream in(&configFile);
    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.contains(targetLine)) {
            line = (status == 1) ? "#" + line : line.mid(1); // Comment or uncomment
        }

        lines.append(line);
    }
    configFile.close();

    // Attempt to open the file for writing
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Error", "Failed to modify configuration file.");
        return;
    }

    QTextStream out(&configFile);
    for (const QString &line : lines) {
        out << line << "\n";
    }
    configFile.close();

    // Update the button text after successful modification
    terminalThemeButton->setText(status == 1 ? "Enable Terminal Theming" : "Disable Terminal Theming");
    QMessageBox::information(this, "Success", "Terminal theming successfully updated");
}

// == I LOVE CPP ==================================
///////////////////////////////////////////////////
/// START MAIN FUNCTION
//////////////////////////////////////////////////
// == I LOVE C++ ==================================

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    coreFunctions = new CoreFunctions(this);

    setWindowTitle("Tolitica Ada Assistant");
    resize(800,600);
    setWindowIcon(QIcon(":/icons/resources/icons/tolitica-icon.png"));

    // Stacked widget to hold both layouts
    QStackedWidget *stackedWidget = new QStackedWidget(this);

    // ==== Main Page =====
    QWidget *mainPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainPage);

    // ==== Header and Description ==== //
    QLabel *headerLabel = new QLabel("<h1>Welcome to Tolitica Ada Assistant!</h1>", this);
        // == Description == //
    QLabel *greetingsLabel = new QLabel("<span style=\"font-size:11pt; font-weight:bold;\">Greetings from Angel!</span> - Owner & Maintainer of Ada", this);
    QLabel *descriptionLabel = new QLabel("With this helper application you can tweak several "
    "configurations from your system, please enjoy using "
    "Ada, break it, repair it or donate to me... Anyway have fun using my ArchLinux distro.", this);
    headerLabel->setContentsMargins(0,20,0,0);
    greetingsLabel->setContentsMargins(0,10,0,0);
    greetingsLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setContentsMargins(0,10,0,0);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(headerLabel, 0, Qt::AlignTop | Qt::AlignCenter);
    mainLayout->addWidget(greetingsLabel);
    mainLayout->addWidget(descriptionLabel, 1, Qt::AlignTop);
    // ==== End Header and Description ==== //

    // ==== Add Tolitica Icon ==== //
    QLabel *iconLabel = new QLabel(this);
    QPixmap toliticaIcon(":/icons/resources/icons/tolitica-icon.png");
    QPixmap scaledIcon = toliticaIcon.scaled(100, 100, Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
    iconLabel->setPixmap(scaledIcon);
    iconLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(iconLabel, 3, Qt::AlignCenter | Qt::AlignTop);

    //* ==== Button to Mount/Umount Hard Drives ==== *//
        // Create a container widget for the label and the button
    QWidget *mountDriveWidget = new QWidget(this);
    QVBoxLayout *mountDriveLayout = new QVBoxLayout(mountDriveWidget);
    mountDriveLayout->setContentsMargins(0, 0, 0, 0);
    mountDriveLayout->setSpacing(5); // Adjust as needed for vertical spacing

    QLabel *mountDriveLabel = new QLabel("Mount/Unmount Drives", this);
    mountDriveLabel->setAlignment(Qt::AlignLeft);

    QToolButton *mountDriveButton = new QToolButton(this);
    mountDriveButton->setIcon(QIcon(":/icons/resources/icons/hdd.png"));
    mountDriveButton->setIconSize(QSize(48, 48));
    mountDriveButton->setAutoRaise(true);
    mountDriveButton->setToolTip("Mount/Umount Drives");

        // Add the label and button to the container layout
    mountDriveLayout->addWidget(mountDriveLabel);
    mountDriveLayout->addWidget(mountDriveButton, 0, Qt::AlignCenter);
        // Adjust the container's width to match the label's sizeHint,
        // so the button's horizontal center is determine by the label's width.
    mountDriveWidget->setFixedWidth(mountDriveLabel->sizeHint().width());
    //mainLayout->addWidget(mountDriveWidget, 0, Qt::AlignLeft);

    /* === Navigation Buttons === */
    QVBoxLayout *buttonsLayout = new QVBoxLayout(); // Specific Layout for Buttons
    QPushButton *tweaksButton = new QPushButton("Configuration", this);
    QPushButton *addonsButton = new QPushButton("Addons", this);
    QPushButton *terminalButton = new QPushButton("Terminal", this);

    buttonsLayout->addWidget(mountDriveWidget, 0, Qt::AlignLeft);
    buttonsLayout->addWidget(tweaksButton);
    buttonsLayout->addWidget(addonsButton);
    buttonsLayout->addWidget(terminalButton);
    mainLayout->addLayout(buttonsLayout);
    mainPage->setLayout(mainLayout);

    // Horizontal layout for social media buttons
    QHBoxLayout *socialMediaLayout = new QHBoxLayout();

    //* === Social Media Icon Buttons === */
    // * == Discord
    QToolButton *discordButton = new QToolButton(this);
    discordButton->setIcon(QIcon(":/icons/resources/icons/discord.png"));
    discordButton->setIconSize(QSize(38, 38));
    discordButton->setAutoRaise(true);
    discordButton->setToolTip("Join my Discord!");

    // * == Twitter
    QToolButton *twitterButton = new QToolButton(this);
    twitterButton->setIcon(QIcon(":/icons/resources/icons/twitter.png"));
    twitterButton->setIconSize(QSize(48, 48));
    twitterButton->setAutoRaise(true);
    twitterButton->setToolTip("Follow me on Twitter!");

    // * == YouTube
    QToolButton *youtubeButton = new QToolButton(this);
    youtubeButton->setIcon(QIcon(":/icons/resources/icons/youtube.png"));
    youtubeButton->setIconSize(QSize(48, 48));
    youtubeButton->setAutoRaise(true);
    youtubeButton->setToolTip("Subscribe to my Channel!");

    // * == Patreon
    QToolButton *patreonButton = new QToolButton(this);
    patreonButton->setIcon(QIcon(":/icons/resources/icons/patreon.png"));
    patreonButton->setIconSize(QSize(48, 48));
    patreonButton->setAutoRaise(true);
    patreonButton->setToolTip("Become my patreon!");

    // * == Instagram
    // QToolButton *instagramButton = new QToolButton(this);
    // instagramButton->setIcon(QIcon(":/icons/resources/icons/instagram.png"));
    // instagramButton->setIconSize(QSize(48, 48));
    // instagramButton->setAutoRaise(true);
    // instagramButton->setToolTip("Follow me on Instagram!");

    // Insert social media buttons into the horizontal layout
    socialMediaLayout->addWidget(discordButton);
    socialMediaLayout->addWidget(twitterButton);
    socialMediaLayout->addWidget(youtubeButton);
    socialMediaLayout->addWidget(patreonButton);
    // socialMediaLayout->addWidget(instagramButton);
    socialMediaLayout->setSpacing(2);
    socialMediaLayout->setAlignment(Qt::AlignLeft);

    // Adding the horizontal layout to the main one
    mainLayout->addLayout(socialMediaLayout);

    // === Connect signals to the socialMedia function === //
    connect(discordButton, &QToolButton::clicked, this, [=](){
        coreFunctions->socialMedia("discord");
    });
    connect(twitterButton, &QToolButton::clicked, this, [=](){
        coreFunctions->socialMedia("twitter");
    });
    connect(youtubeButton, &QToolButton::clicked, this, [=](){
        coreFunctions->socialMedia("youtube");
    });
    connect(patreonButton, &QToolButton::clicked, this, [=](){
        coreFunctions->socialMedia("patreon");
    });

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== Tweaks Page =====
    ///////////////////////////////////////////////////////////////////////////////////////////////
    QWidget *tweaksPage = new QWidget();
    QGridLayout *tweaksLayout = new QGridLayout(tweaksPage);
    QPushButton *backButton = new QPushButton("Back", this);

    // Functional Buttons Tweaks Layout
    QPushButton *cleanOrphansButton = new QPushButton("Clean Unused Packages", this);
    QPushButton *cleanPkgCacheButton = new QPushButton("Clean Package Cache", this);
    QPushButton *updateSystemButton = new QPushButton("Update Ada", this);
    QPushButton *removeDBLockButton = new QPushButton("Remove DB Lock", this);
    QPushButton *rankMirrorsButton = new QPushButton("Rank Mirrors", this);

    // ** Bluetooth Toggle CheckBox ** //
    QCheckBox *bluetoothToggle = new QCheckBox("Enable Bluetooth", this);
    tweaksLayout->addWidget(bluetoothToggle, 3, 0, Qt::AlignLeft);
        // ** Bluetooth: set initial state based on Bluetooth status ** //
    bool bluetoothEnabled = (CoreFunctions::bluetoothStatus() == 0);
    bluetoothToggle->setChecked(bluetoothEnabled);
    bluetoothToggle->setText(bluetoothEnabled ? "Disable Bluetooth" : "Enable Bluetooth");
    // ** AppArmor Toggle CheckBox
    QCheckBox *apparmorToggle = new QCheckBox(this);
    tweaksLayout->addWidget(apparmorToggle, 3, 0, Qt::AlignRight);
        // ** AppArmor: set initial state base on AppArmor statud ** //
    int apparmorStatus = CoreFunctions::apparmorStatus();

    if(apparmorStatus == 0) {
        apparmorToggle->setText("Enable AppArmor");
    } else {
        bool appArmorEnabled = (CoreFunctions::apparmorStatus() == 0);
        apparmorToggle->setChecked(appArmorEnabled);
        apparmorToggle->setText(appArmorEnabled ? "Disable AppArmor" : "Enable AppArmor");
    }

    /* === Positioning Buttons === */
    tweaksLayout->addWidget(cleanOrphansButton, 1, 0, Qt::AlignLeft);
    tweaksLayout->addWidget(cleanPkgCacheButton, 1, 0, Qt::AlignRight);
    tweaksLayout->addWidget(rankMirrorsButton, 2, 0, Qt::AlignRight);
    tweaksLayout->addWidget(updateSystemButton, 2, 0, Qt::AlignCenter);
    tweaksLayout->addWidget(removeDBLockButton, 2, 0, Qt::AlignLeft);

    // Push Back-button to bottom dynamically
    tweaksLayout->setRowStretch(4, 1);
    tweaksLayout->addWidget(backButton, 5, 0, Qt::AlignLeft);
    tweaksPage->setLayout(tweaksLayout);

    tweaksSetupConnections(stackedWidget, tweaksButton, backButton, cleanOrphansButton,
                           cleanPkgCacheButton, updateSystemButton, removeDBLockButton, bluetoothToggle,
                           apparmorToggle, rankMirrorsButton);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== Terminal Page =====
    ///////////////////////////////////////////////////////////////////////////////////////////////
    QWidget *terminalPage = new QWidget();
    QGridLayout *terminalLayout = new QGridLayout(terminalPage);
    QPushButton *terminalBackButton = new QPushButton("Back", this);

    // ** Functional Buttons Terminal Layout ** //
    // *Enable/Disable Terminal Theming
    QPushButton *terminalThemeButton = new QPushButton("Disable Terminal Theming", this);
    int checkTermStatus = checkTermThemingStatus();

    if (checkTermStatus == 0) {
        terminalThemeButton->setText("No config.fish found");
    } else {
        terminalThemeButton->setText(checkTermStatus == 1 ? "Disable Terminal Theming" :
                                                            "Enable Terminal Theming");
    }
    // *Change the shell
    QPushButton *changeShellButton = new QPushButton("Change Shell", this);
    QComboBox *shellComboBox = new QComboBox(this); // Added for shell selection
    QStringList shells = CoreFunctions::getInstalledShells(); // Call to get installed shells
    QLabel *shellLabel = new QLabel("Current Shell: Unknown", this); // Add sehll label
    QGroupBox *shellGroupBox = new QGroupBox("Shell Options", this);
    QVBoxLayout *shellBoxGroupLayout = new QVBoxLayout(shellGroupBox);

    // Fetch and isplay the current shell
    QString currentShell = CoreFunctions::getCurrentShell();
    shellLabel->setText("Current Shell: " + currentShell);

    if (shells.isEmpty()) {
        shellComboBox->addItem("No shells detected");
        changeShellButton->setEnabled(false); // Disable button if no shells detected
    } else {
        shellComboBox->addItems(shells);
    }

    /* === Positioning Buttons === */
    terminalLayout->addWidget(terminalThemeButton);
    terminalLayout->addWidget(changeShellButton);
    /* === Boxes === */
    terminalLayout->addWidget(shellComboBox); // Dropdown for shell selection
        // Widgets for current shell-box
    shellBoxGroupLayout->addWidget(shellLabel);
    shellBoxGroupLayout->addWidget(shellComboBox);
    shellBoxGroupLayout->addWidget(changeShellButton);
    shellGroupBox->setLayout(shellBoxGroupLayout);
    shellGroupBox->setAlignment(Qt::AlignCenter);

    /* === Others === */
    terminalLayout->addWidget(shellGroupBox, 5, 0, Qt::AlignRight);

    // Push back button to bottom dynamically
    terminalLayout->setRowStretch(4, 1);
    terminalLayout->addWidget(terminalBackButton, 5, 0, Qt::AlignLeft);
    terminalPage->setLayout(terminalLayout);

    terminalSetupConnections(stackedWidget, terminalButton, terminalBackButton, terminalThemeButton, changeShellButton, shellComboBox, shellLabel);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== Addons Page =====
    ///////////////////////////////////////////////////////////////////////////////////////////////
    QWidget *addonsPage = new QWidget();
    QGridLayout *addonsLayout = new QGridLayout(addonsPage);
    QPushButton *addonsBackButton = new QPushButton("Back", this);

    // ** Functional Buttons Addons Layout ** //
    // *Ada Gaming Meta
    QPushButton *adaGamingMetaButton = new QPushButton(this);
    QProcess checkAGMinstalled;
    checkAGMinstalled.start("bash", QStringList() << "-c" << "pacman -Q ada-gaming-meta");
    checkAGMinstalled.waitForFinished();

    if (checkAGMinstalled.exitCode() == 0) {
        adaGamingMetaButton->setText("Remove Ada Gaming Meta");
    } else {
        adaGamingMetaButton->setText("Install Ada Gaming Meta");
    }
    // *Ada Development Meta
    QPushButton *adaDevelopmentMetaButton = new QPushButton(this);
    QProcess checkADMinstalled;
    checkADMinstalled.start("bash", QStringList() << "-c" << "pacman -Q ada-development-meta");
    checkADMinstalled.waitForFinished();

    if (checkADMinstalled.exitCode() == 0) {
        adaDevelopmentMetaButton->setText("Remove Ada Development Meta");
    } else {
        adaDevelopmentMetaButton->setText("Install Ada Development Meta");
    }
    // *ChaoticAUR Button
    QPushButton *chaoticAURButton = new QPushButton(this);

    int chaoticStatus = checkChaoticAURStatus();
    chaoticAURButton->setText(chaoticStatus == 1 ? "Remove Chaotic AUR" :
                              chaoticStatus == 2 ? "Add Chaotic AUR" :
                              "Repair Chaotic AUR");
    // **Add VMware Support**//
    QPushButton *vmwButton = new QPushButton(this);

    int vmStatus = vmwareStatus();
    if(vmStatus == 0) {
        vmwButton->setText("Remove VMware Workstation");
    } else {
        vmwButton->setText("Install/Enable VMware Workstation");
    }

    // ** Flatpak Toggle CheckBox ** //
    QCheckBox *flatpakToggle = new QCheckBox(this);
    addonsLayout->addWidget(flatpakToggle, 5, 0, Qt::AlignCenter);
        /* Flatpak set initial state based on Flatpak-Status */
    bool flatpakEnabled = (CoreFunctions::flatpakStatus() == 0);
    flatpakToggle->setChecked(flatpakEnabled);
    flatpakToggle->setText(flatpakEnabled ? "Disable/Remove Flatpak" : "Enable/Install Flatpak");

    // ** Snapd Toggle CheckBox ** //
    QCheckBox *snapdToggle = new QCheckBox(this);
    addonsLayout->addWidget(snapdToggle, 5, 0, Qt::AlignRight);
        /*Snapd set initial state based on Flatpak-status*/
    bool snapdEnabled = (coreFunctions->snapdStatus() == 0);
    snapdToggle->setChecked(snapdEnabled);
    snapdToggle->setText(snapdEnabled ? "Disable/Remove SNAPD" : "Enable/Install SNAPD");

    /* === Positioning Buttons === */
    addonsLayout->addWidget(adaGamingMetaButton, 1, 0, Qt::AlignLeft);
    addonsLayout->addWidget(adaDevelopmentMetaButton, 1, 0, Qt::AlignCenter);
    addonsLayout->addWidget(chaoticAURButton, 1, 0, Qt::AlignRight);
    addonsLayout->addWidget(vmwButton, 2, 0, Qt::AlignCenter);

    // Push Back-button to bottom dynamically
    addonsLayout->setRowStretch(4, 1);
    addonsLayout->addWidget(addonsBackButton, 5, 0, Qt::AlignLeft);
    addonsPage->setLayout(addonsLayout);

    /* === Connections === */
    addonsSetupConnections(stackedWidget, addonsButton, addonsBackButton, adaGamingMetaButton,
                           adaDevelopmentMetaButton, chaoticAURButton, vmwButton, flatpakToggle, snapdToggle);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== Mount Drives Page =====
    ///////////////////////////////////////////////////////////////////////////////////////////////

    mountDrivesSetupConnections(stackedWidget, mountDriveButton);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== End Pages =====
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Add pages to stacked widget
    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(tweaksPage);
    stackedWidget->addWidget(addonsPage);
    stackedWidget->addWidget(terminalPage);

    // Set the initial layout
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(this);
    mainWidgetLayout->addWidget(stackedWidget);
    setLayout(mainWidgetLayout);
}
// == I LOVE CPP ==================================
///////////////////////////////////////////////////
/// END MAIN FUNCTION
//////////////////////////////////////////////////
// == I LOVE C++ ==================================


///////////////////////////////////////////////////
/// TWEAK SETUP CONNECTIONS FUNCTION
//////////////////////////////////////////////////
void Widget::tweaksSetupConnections(QStackedWidget *stackedWidget, QPushButton *tweaksButton, QPushButton *backButton,
                                    QPushButton *cleanOrphansButton, QPushButton *cleanPkgCacheButton,
                                    QPushButton *updateSystemButton, QPushButton *removeDBLockButton,
                                    QCheckBox *bluetoothToggle, QCheckBox *appArmorToggle, QPushButton *rankMirrorsButton){
    // Navigation connections
    connect(tweaksButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(1); // Switch to Tweaks page
    });
    connect(backButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0); // Switch back to Main page
    });

    connect(cleanOrphansButton, &QPushButton::clicked, this, &Widget::cleanOrphans);
    connect(cleanPkgCacheButton, &QPushButton::clicked, this, &Widget::cleanPkgCache);
    connect(updateSystemButton, &QPushButton::clicked, this, &Widget::systemUpdate);
    connect(removeDBLockButton, &QPushButton::clicked, this, &Widget::removeDBLock);

    // ** Bluetooth Toggle Connection ** //
    connect(bluetoothToggle, &QCheckBox::toggled, this, [this, bluetoothToggle]() {
        CoreFunctions::enableBluetooth(this, bluetoothToggle);

        // ** Update Checkbox label based on new state ** //
        bluetoothToggle->setText(bluetoothToggle->isChecked() ? "Disable Bluetooth" : "Enable Bluetooth");
    });
        // ** AppArmor Toggle Connection ** //
        connect(appArmorToggle, &QCheckBox::toggled, this, [this, appArmorToggle]() {
        CoreFunctions::enableAppArmor(this, appArmorToggle);

        // ** Update CheckBox label based on new state ** //
        appArmorToggle->setText(appArmorToggle->isChecked() ? "Disable AppArmor" : "Enable AppArmor");
    });
        // ** Rank Mirrors ** //
        connect(rankMirrorsButton, &QPushButton::clicked, this, [this]() {
            int mirrorCount = coreFunctions->getMirrorCount(this);
            if (mirrorCount == -1) {
                // User cancel the dialog
                return;
            }
            coreFunctions->rankMirrors(this, mirrorCount);
        });
}

///////////////////////////////////////////////////
/// ADDONS SETUP CONNECTIONS FUNCTION
//////////////////////////////////////////////////
void Widget::addonsSetupConnections(QStackedWidget *stackedWidget, QPushButton *addonsButton, QPushButton *addonsBackButton,
                                    QPushButton *adaGamingMetaButton, QPushButton *adaDevelopmentButton, QPushButton *chaoticAURButton,
                                    QPushButton *vmwButton, QCheckBox *flatpakToggle, QCheckBox *snapdToggle) {
        // Navigation connections
        connect(addonsButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(2); // Switch to Addons page
    });
        connect(addonsBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0); // Switch back to Main page
    });

        // Connecting Buttons to their respective Functions
        //** Ada Gaming Meta **//
        connect(adaGamingMetaButton, &QPushButton::clicked, this, [=]() mutable {
        if(adaGamingMetaButton->text() == "Install Ada Gaming Meta") {
            adaGamingMetaButton->setText("Remove Ada Gaming Meta");
            adaGamingMeta();
        } else {
            adaGamingMetaButton->setText("Install Ada Gaming Meta");
            removeAdaGamingMeta();
        }
    });
        //** Ada Development Meta **//
        connect(adaDevelopmentButton, &QPushButton::clicked, this, [=]() mutable {
        if(adaDevelopmentButton->text() == "Install Ada Development Meta") {
            adaDevelopmentButton->setText("Remove Ada Development Meta");
            adaDevelopmentMeta();
        } else {
            adaDevelopmentButton->setText("Install Ada Development Meta");
            removeAdaDevelopmentMeta();
        }
    });
        //** Chaotic AUR **//
        connect(chaoticAURButton, &QPushButton::clicked, this, [=]() mutable {
        int chaoticStatus = checkChaoticAURStatus();

        if (chaoticStatus == 1) { // Fully installed
            chaoticAUR();
        } else if (chaoticStatus == 2) { // Not installed
            chaoticAUR();
        } else if (chaoticStatus == 3) { // Run repair
            chaoticAUR();

        }
        // **Recheck status after repair**
        chaoticStatus = checkChaoticAURStatus();
        chaoticAURButton->setText(chaoticStatus == 1 ? "Remove Chaotic AUR" :
                                      chaoticStatus == 2 ? "Add Chaotic AUR" :
                                      "Repair Chaotic AUR");
    });
        //** Add VMware Support **//
        connect(vmwButton, &QPushButton::clicked, this, [=]() mutable {
        int vmStatus = vmwareStatus();

        if(vmStatus == 0) { // Fully installed
            removeVMware(vmwButton);
        } else { // Not installed or partially installed
            addVMware(vmwButton);
        }
    });

        // ** Flatpak Toggle Connection ** //
        connect(flatpakToggle, &QCheckBox::toggled, this, [this, flatpakToggle]() {
        CoreFunctions::enableFlatpak(this, flatpakToggle);

        // ** Update checkBox label based on new state ** //
        flatpakToggle->setText(flatpakToggle->isChecked()
        ? "Disable/Remove Flatpak" : "Enable/Install Flatpak");
    });
        // ** Snapd Toggle Connection ** //
        connect(snapdToggle, &QCheckBox::toggled, this, [this, snapdToggle]() {
        coreFunctions->enableSnapd(this, snapdToggle);

        // ** Update checkBox label based on new state ** //
        snapdToggle->setText(snapdToggle->isChecked()
        ? "Disable/Remove SNAPD" : "Enable/Install SNAPD");
    });

}

///////////////////////////////////////////////////
/// TERMINAL SETUP CONNECTIONS FUNCTION
//////////////////////////////////////////////////
void Widget::terminalSetupConnections(QStackedWidget *stackedWidget, QPushButton *terminalButton, QPushButton *terminalBackButton,
                                      QPushButton *terminalThemeButton, QPushButton *changeShellButton, QComboBox *shellComboBox,
                                      QLabel *shellLabel) {
    // Navigation connections
    connect(terminalButton, &QPushButton::clicked, this, [stackedWidget] () {
        stackedWidget->setCurrentIndex(3); // Switch to Terminal page
    });
    connect(terminalBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0); // Switch back to Main page
    });

    //** Enable/Disable Terminal theming **//
    connect(terminalThemeButton, &QPushButton::clicked, this, [=]() mutable {
        int termStatus = checkTermThemingStatus();

        if(checkTermThemingStatus() == 0) {
            disableTermTheme(terminalThemeButton);
        } else {
            disableTermTheme(terminalThemeButton);
        }
    });

    connect(changeShellButton, &QPushButton::clicked, this, [=]() {
        QString selectedShell = shellComboBox->currentText(); // Get selected shell from QComboBox
        if (selectedShell.contains("No shells detected")) return;

        CoreFunctions::changeShell(this, selectedShell); // Call CoreFunctions::changeShell

        // Refresh label after shell change
        QString newShell = CoreFunctions::getCurrentShell();
        shellLabel->setText("Current Shell: " + newShell);
    });
}

///////////////////////////////////////////////////
/// MOUNT DRIVES SETUP CONNECTION
//////////////////////////////////////////////////
void Widget::mountDrivesSetupConnections(QStackedWidget *stackedWidget, QToolButton *mountDriveButton) {
    connect(mountDriveButton, &QPushButton::clicked, this, [this, stackedWidget]() {
       // Create the mount drives page container only once
        if (!mountDrivesPage) {
           mountDrivesPage = new QWidget(this);
            // Use a vertical layout for a natural top-to-bottom arrangement
           QVBoxLayout *mountDrivesLayout = new QVBoxLayout(mountDrivesPage);
           mountDrivesLayout->setContentsMargins(0, 0, 0, 0);

           // Create or refresh the drive list widget
           if (!drivesPage) {
               drivesPage = new drive_list_widget(mountDrivesPage);
           } else {
               drivesPage->refresh();
           }
           // Add the drive list widget with a stretch factor of 1 so it takes available space.
           mountDrivesLayout->addWidget(drivesPage, 1);

           // Create the back button using mountDrivesPage as its parent
           // ensuring it only appears on this page
           QPushButton *mountDrivesBackButton = new QPushButton("Back", mountDrivesPage);
           // Add the back button at the bottom. A stretch factor of 0 keeps it at it natural size.
           mountDrivesLayout->addWidget(mountDrivesBackButton, 0, Qt::AlignLeft);

           mountDrivesPage->setLayout(mountDrivesLayout);
           stackedWidget->insertWidget(4, mountDrivesPage);

           // Connect the back button to switch back to the main page (index 0).
           connect(mountDrivesBackButton, &QPushButton::clicked, this, [stackedWidget]() {
               stackedWidget->setCurrentIndex(0);
           });
        } else {
            drivesPage->refresh();
        }

        // Switch to the mount drives page (index 4).
        stackedWidget->setCurrentIndex(4);
    });
}


Widget::~Widget()
{
    delete ui;
}
