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
#include "calamares_page.h"

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
    checkYayProcess.start("bash", QStringList() << "-c" << "pacman -Q yay");
    checkYayProcess.waitForFinished();

    bool yayInstalled = (checkYayProcess.exitCode() == 0);

    // Create the command
    QStringList args;
    if (yayInstalled) {
        args << "bash" << "-c" << "yay -Yc --noconfirm && "
                "pkexec pacman -Rns $(pacman -Qtdq) --noconfirm";
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
    progress->setWindowModality(Qt::ApplicationModal); // Modality to prevent UI freezing
    progress->setCancelButton(nullptr);
    progress->setValue(0); // Ensure it starts at 0
    progress->show();

    QCoreApplication::processEvents(); // Forces immediate rendering before the process starts

    int progressValue = 0;

    // Updating the progress value when a new started output is available.
    connect(sysUp, &QProcess::readyReadStandardOutput, this, [=]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents(); // Ensure progress updates properly
    });

    // Using monitorTimer to simulate progress updates if process output is insufficient
    connect(monitorTimer, &QTimer::timeout, this, [=]() mutable {
        if (progressValue < 95) {
            progressValue += 2;
            progress->setValue(qMin(progressValue, 95));
        }
    });
    monitorTimer->start(250);

    // Detect when process finishes
    connect(sysUp, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=] (int exitCode,
    QProcess::ExitStatus status) {
        monitorTimer->stop();

        QString output = QString::fromUtf8(sysUp->readAllStandardOutput());
        if (output.contains("Nothing to do") && output.contains("there is nothing to do")) {
            QMessageBox::information(nullptr, "System Already Upto Date", "No updates were found");
        } else {
            progress->setValue(100);
            QMessageBox::information(nullptr, "Xray_OS Has Been Updated", "Your Operating System has been updated successfully");
            progress->setValue(100);
        }

        // Cleanup Memory
        progress->deleteLater();
        sysUp->deleteLater();
        monitorTimer->deleteLater(); // Stop aggressive monitoring

    });

    // Adding a little delay to ensure UI stability before starting the process
    QTimer::singleShot(150, this, [=]() {
        // Start update process
        sysUp->start("pkexec", QStringList() << "bash" << "-c" << "pacman -Syu --noconfirm && flatpak update --assumeyes");

        // Start aggressive monitoring
        monitorTimer->start(250); // Updates every 250ms
    });

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
/// ADDONS::INSTALL ARCH7Z-GAMING-META FUNCTION
//////////////////////////////////////////////////
void Widget::archZGamingMeta() {
    QProcess checkIssues;

    // Check DB sync
    checkIssues.start("bash", QStringList() << "-c" << "pacman -Sy --dbonly");
    checkIssues.waitForFinished();
    QString dbSyncErrors = checkIssues.readAllStandardError();

    bool dbNotSynced = !dbSyncErrors.isEmpty();

    QDir pkgCacheDir("/var/cache/pacman/pkg");
    bool cacheExists = pkgCacheDir.exists() && !pkgCacheDir.isEmpty();

    if (cacheExists) {
        // Check for PKG corruption
        checkIssues.start("bash", QStringList() << "-c" << "pacman -Qk");
        checkIssues.waitForFinished();
        QString corruptionErrors = checkIssues.readAllStandardError();
        bool corruptedPackages = !corruptionErrors.isEmpty();

        // cleanup

        if (dbNotSynced || (cacheExists && corruptedPackages)) {
            qDebug() << "Issues detected! Cleaning package cache.";
            QProcess cleanup;

            if (dbNotSynced) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "pacman -Sy");
                cleanup.waitForFinished();

                if (cleanup.exitCode() != 0) {
                    qDebug() << "Cleanup errors(pacman -Sy): " << cleanup.readAllStandardError();
                }
            }
            if (corruptedPackages) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "sudo rm -rf /var/cache/pacman/pkg/* && sudo pacman -Scc --noconfirm");
                cleanup.waitForFinished();
            }

            qDebug() << "Cleanup output:" << cleanup.readAllStandardOutput();
            qDebug() << "Cleanup erros:" << cleanup.readAllStandardError();

        } else {
            qDebug() << "System database and package cache are fine!";
        }
    }

    QProcess *installAGM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this); // High-frequency monitoring

    // Create the progress bar dynamically
    QProgressDialog *progress = new QProgressDialog("Installing Arch7z Gaming Meta...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // Fix: Real-time progress update using process output
    connect(installAGM, &QProcess::readyReadStandardOutput, this, [=]() mutable {
        // Adjust progress dynamically based on output
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents(); // Ensure UI refresh
    });

    // Combined finished signal handling
    connect(installAGM, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) mutable {
                // If installation failed, try to clean the cache and reinstall
                if (exitCode != 0) {
                    QProcess fix(this);
                    fix.start("pkexec", QStringList() << "pacman -Scc --noconfirm");
                    fix.waitForFinished();  // Ensure cache cleanup before retry
                    installAGM->start("pkexec", QStringList() << "bash" << "-c" << "pacman -S arch7z-gaming-meta --noconfirm");
                    installAGM->waitForFinished();
                }

                // After installation (or retry), check if the package is installed
                QProcess checkInstalled;
                checkInstalled.start("bash", QStringList() << "-c" << "pacman -Q arch7z-gaming-meta");
                checkInstalled.waitForFinished();

                progress->setValue(100); // Mark progress as complete

                if (checkInstalled.exitCode() == 0) {
                    QMessageBox::information(nullptr, "Arch7z Gaming Meta",
                                             "Arch7z Gaming Meta packages are installed successfully!");
                } else {
                    QMessageBox::warning(nullptr, "Arch7z Gaming Meta",
                                         "There was a problem installing Arch7z Gaming Meta packages.");
                }

                // Cleanup Memory
                progress->deleteLater();
                installAGM->deleteLater();
                monitorTimer->deleteLater(); // Stop aggressive monitoring
            });

    // Start aggressive monitoring
    monitorTimer->start(250); // Updates every 250ms

    // Start installing process
    installAGM->start("pkexec", QStringList() << "bash" << "-c" << "pacman -S arch7z-gaming-meta --noconfirm");
    installAGM->waitForFinished();
}


///////////////////////////////////////////////////
/// ADDONS::REMOVE ARCH7Z-GAMING-META FUNCTION
//////////////////////////////////////////////////
void Widget::removeArchZGamingMeta() {
    QProcess *removeAGM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this);

    QProgressDialog *progress = new QProgressDialog("Removing Arch7z Gaming Meta...", nullptr, 0, 100, this);
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
        "pacman -R arch7z-gaming-meta --noconfirm",
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
            QMessageBox::information(nullptr, "Arch7z Gaming Meta Removed",
                                     "Arch7z Gaming Meta packages have been successfully removed.");
            progress->deleteLater();
            removeAGM->deleteLater();
            monitorTimer->deleteLater();
        }
    });

    removeAGM->start("pkexec", QStringList() << "bash" << "-c" << removeCommands[currentStep]); // Start first command
    monitorTimer->start(250);
}

///////////////////////////////////////////////////
/// ADDONS::INSTALL ARCH7Z-DEVELOPMENT-META FUNCTION
///////////////////////////////////////////////////
void Widget::archZDevelopmentMeta() {
    QProcess checkIssues;

    // Check DB sync
    checkIssues.start("bash", QStringList() << "-c" << "pacman -Sy --dbonly");
    checkIssues.waitForFinished();
    QString dbSyncErrors = checkIssues.readAllStandardError();

    bool dbNotSynced = !dbSyncErrors.isEmpty();

    QDir pkgCacheDir("/var/cache/pacman/pkg");
    bool cacheExists = pkgCacheDir.exists() && !pkgCacheDir.isEmpty();

    if (cacheExists) {
        // Check for PKG corruption
        checkIssues.start("bash", QStringList() << "-c" << "pacman -Qk");
        checkIssues.waitForFinished();
        QString corruptionErrors = checkIssues.readAllStandardError();
        bool corruptedPackages = !corruptionErrors.isEmpty();

        // cleanup

        if (dbNotSynced || (cacheExists && corruptedPackages)) {
            qDebug() << "Issues detected! Cleaning package cache.";
            QProcess cleanup;

            if (dbNotSynced) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "pacman -Sy");
                cleanup.waitForFinished();

                if (cleanup.exitCode() != 0) {
                    qDebug() << "Cleanup errors(pacman -Sy): " << cleanup.readAllStandardError();
                }
            }
            if (corruptedPackages) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "sudo rm -rf /var/cache/pacman/pkg/* && sudo pacman -Scc --noconfirm");
                cleanup.waitForFinished();
            }

            qDebug() << "Cleanup output:" << cleanup.readAllStandardOutput();
            qDebug() << "Cleanup erros:" << cleanup.readAllStandardError();

        } else {
            qDebug() << "System database and package cache are fine!";
        }
    }


    QProcess *installADM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this); // High-frequency monitoring

    // Create the progress bar dynamically
    QProgressDialog *progress = new QProgressDialog("Installing Arch7z Development Meta...", nullptr, 0, 100, this);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // Real-time progress update based on process output
    connect(installADM, &QProcess::readyReadStandardOutput, this, [=]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents(); // Ensure UI refresh
    });

    // Combined finished signal handling with cache cleanup on failure
    connect(installADM, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) mutable {
                // If the installation fails, clean up the pacman cache and retry installation
                // if (exitCode != 0) {
                //     QProcess fix(this);
                //     fix.start("pkexec", QStringList() << "bash" << "-c" << "pacman -Scc --noconfirm");
                //     fix.waitForFinished();  // Wait for cache cleanup to complete

                //     // Retry installation after cache cleanup
                //     installADM->start("pkexec", QStringList() << "bash" << "-c"
                //                                               << "pacman -S arch7z-development-meta --noconfirm");
                //     installADM->waitForFinished();
                // }

                // After installation (or after the retry), check if the package is installed
                QProcess checkInstalled;
                checkInstalled.start("bash", QStringList() << "-c" << "pacman -Q arch7z-development-meta");
                checkInstalled.waitForFinished();

                progress->setValue(100); // Mark progress as complete

                if (checkInstalled.exitCode() == 0) {
                    QMessageBox::information(nullptr, "Arch7z Development Meta",
                                             "Arch7z Development Meta packages are installed successfully!");
                } else {
                    QMessageBox::warning(nullptr, "Arch7z Development Meta",
                                         "There was a problem installing Arch7z Development Meta packages.");
                }

                // Cleanup dynamic objects
                progress->deleteLater();
                installADM->deleteLater();
                monitorTimer->deleteLater(); // Stop aggressive monitoring
            });

    // Start aggressive monitoring of progress updates
    monitorTimer->start(250); // Updates every 250ms

    // Begin the installation process
    installADM->start("pkexec", QStringList() << "bash" << "-c"
                                              << "pacman -S arch7z-development-meta --noconfirm");
    installADM->waitForFinished();
}


///////////////////////////////////////////////////
/// ADDONS:: REMOVE ARCH7Z-DEVELOPMENT-META FUNCTION
//////////////////////////////////////////////////
void Widget::removeArchZDevelopmentMeta() {
    QProcess *removeADM = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this);

    QProgressDialog *progress = new QProgressDialog("Removing Arch7z Development Meta...", nullptr, 0, 100,
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
        "pacman -R arch7z-development-meta --noconfirm",
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
            QMessageBox::information(nullptr, "Arhc7z Development Meta Removed",
                                     "Arhc7z Development Meta packages have been successfully removed");
            progress->deleteLater();
            removeADM->deleteLater();
            monitorTimer->deleteLater();
        }
    });

    removeADM->start("pkexec", QStringList() << "bash" << "-c" << removeCommands[CurrentStep]); // Start first commands
    monitorTimer->start(250);
}

///////////////////////////////////////////////////
/// ADDONS::HELPER: Run a shell command
//////////////////////////////////////////////////
bool Widget::runCommand(const QString &cmd) {
    QProcess process;
    process.start("bash", QStringList() << "-c" << cmd);
    process.waitForFinished();
    return (process.exitCode() == 0);
}

///////////////////////////////////////////////////
/// ADDONS:: CHECK CHAOTIC-AUR STATUS
//////////////////////////////////////////////////
int Widget::checkChaoticAURStatus() {
    // Check if key is imported
    bool keyExist = runCommand("pacman-key --list-keys 3056513887B78AEB");
    // Check if required packages are installed
    bool packagesInstalled = runCommand("pacman -Q chaotic-keyring chaotic-mirrorlist");
    // Check if repository header exists in pacman.conf
    bool repoHeaderExists = runCommand("grep -q '\\[chaotic-aur\\]' /etc/pacman.conf");

    // Check if Include line exists in pacman.conf
    bool includeLineExists = runCommand("grep -q 'Include = /etc/pacman.d/chaotic-mirrorlist' /etc/pacman.conf");

    if (keyExist && packagesInstalled && repoHeaderExists && includeLineExists)
        return 1; // Fully configured
    else if (!keyExist && !packagesInstalled && !repoHeaderExists && !includeLineExists)
        return 2; // Not set up
    else
        return 3; // Partial setup (repair needed)
}

///////////////////////////////////////////////////
/// ADDONS:: BACKUP PACMAN CONFIG FUNCTION
//////////////////////////////////////////////////
void Widget::backupPacmanConfig() {
    // Always backup before modifications
    QProcess createBackupDir;
    createBackupDir.start("pkexec", QStringList() << "bash" << "-c"
                                                  << "mkdir -p /etc/xray/tolitica/tolitica-settings/backups");
    createBackupDir.waitForFinished();
    QString errOut = createBackupDir.readAllStandardError();

    if (!errOut.isEmpty()) {
        QMessageBox::warning(this, "Backup Error", "Failed to create backup directory:\n" + errOut);
    }

    // Create backup if the current /etc/pacman.conf differs from the backup
    QProcess backupCheck;
    QString backupCmd = "cmp -s /etc/pacman.conf /etc/xray/tolitica/tolitica-settings/backups/pacman.conf || pkexec cp /etc/pacman.conf /etc/xray/tolitica/tolitica-settings/backups/";
    backupCheck.start("bash", QStringList() << "-c" << backupCmd);
    backupCheck.waitForFinished();
}

///////////////////////////////////////////////////
/// ADDONS:: REMOVE-CHAOTIC-AUR
//////////////////////////////////////////////////
void Widget::removeChaoticAUR() {
    QProcess removeProc;
    removeProc.start("pkexec", QStringList() << "bash" << "-c" << "sed -i '/\\[chaotic-aur\\]/,+1d' /etc/pacman.conf && "
                                                                  "pkexec pacman -Rns chaotic-keyring chaotic-mirrorlist --noconfirm && "
                                                                  "pkexec pacman-key --delete 3056513887B78AEB");
    removeProc.waitForFinished();

    // Optionally delete an outdated backup file if it exists
    if (QFile::exists("/etc/xray/tolitica/tolitica-settings/backups/pacman.conf")) {
        QProcess delProc;
        delProc.start("pkexec", QStringList() << "bash" << "-c" << "rm -r /etc/xray/tolitica/tolitica-settings/backups/pacman.conf");
        delProc.waitForFinished();
    }

    QString errorOutput = removeProc.readAllStandardError();
    if (removeProc.exitCode() == 0) {
        QMessageBox::information(this, "Chaotic AUR Removed",
                                 "Chaotic AUR repositories have been removed successfully");
    } else {
        QMessageBox::warning(this, "Error", "Something went wrong removing Chaotic AUR repositories:\n" + errorOutput);
    }
}

///////////////////////////////////////////////////
/// ADDONS:: CHAOTIC-AUR
//////////////////////////////////////////////////
void Widget::chaoticAUR() {
    QProcess checkIssues;

    // Check DB sync
    checkIssues.start("bash", QStringList() << "-c" << "pacman -Sy --dbonly");
    checkIssues.waitForFinished();
    QString dbSyncErrors = checkIssues.readAllStandardError();

    bool dbNotSynced = !dbSyncErrors.isEmpty();

    QDir pkgCacheDir("/var/cache/pacman/pkg");
    bool cacheExists = pkgCacheDir.exists() && !pkgCacheDir.isEmpty();

    if (cacheExists) {
        // Check for PKG corruption
        checkIssues.start("bash", QStringList() << "-c" << "pacman -Qk");
        checkIssues.waitForFinished();
        QString corruptionErrors = checkIssues.readAllStandardError();
        bool corruptedPackages = !corruptionErrors.isEmpty();

        // cleanup

        if (dbNotSynced || (cacheExists && corruptedPackages)) {
            qDebug() << "Issues detected! Cleaning package cache.";
            QProcess cleanup;

            if (dbNotSynced) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "pacman -Sy");
                cleanup.waitForFinished();

                if (cleanup.exitCode() != 0) {
                    qDebug() << "Cleanup errors(pacman -Sy): " << cleanup.readAllStandardError();
                }
            }
            if (corruptedPackages) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "sudo rm -rf /var/cache/pacman/pkg/* && sudo pacman -Scc --noconfirm");
                cleanup.waitForFinished();
            }

            qDebug() << "Cleanup output:" << cleanup.readAllStandardOutput();
            qDebug() << "Cleanup erros:" << cleanup.readAllStandardError();

        } else {
            qDebug() << "System database and package cache are fine!";
        }
    }

    QString Stringstatus = QString::number(checkChaoticAURStatus());
    qDebug() << "status: " << Stringstatus;

    int status = checkChaoticAURStatus();
    // Always back up before making any modifications
    backupPacmanConfig();

    // When fully configured, that is, repository is active, removal is the desired action.
    if (status == 1) {
        removeChaoticAUR();
        return;
    }

    // For initial installation, modify pacman.conf via C++.
    else if (status == 2) {

        // Proceed with package and key setup.
        QProcess addProc;
        QString addCmd =
            "pkexec pacman-key --recv-key 3056513887B78AEB --keyserver keyserver.ubuntu.com && "
            "pkexec pacman-key --lsign-key 3056513887B78AEB && "
            "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-keyring.pkg.tar.zst --noconfirm &&"
            "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-mirrorlist.pkg.tar.zst --noconfirm";
        addProc.start("bash", QStringList() << "-c" << addCmd);
        addProc.waitForFinished();
        qDebug() << "addProcFunction output:" << addProc.readAllStandardOutput();
        qDebug() << "addProcFunction errors:" << addProc.readAllStandardError();

        QString workingDir = QDir::homePath() + "/tolitica-home-settings/backups/current-use";
        if (!QDir().mkpath(workingDir)) {
            qDebug() << "Failed to create backup directory:" << workingDir;
        }
        QString configPath = workingDir + "/pacman.conf";
        QString originalConfig = "/etc/pacman.conf";

        qDebug() << "workingDir:" << workingDir;
        qDebug() << "configPath:" << configPath;
        qDebug() << "originalConfig:" << originalConfig;

        // Copy the original config into our working directory
        QProcess copyProc;
        copyProc.start("pkexec", QStringList() << "cp" << originalConfig << configPath);
        copyProc.waitForFinished();
        qDebug() << "Copy exit code:" << copyProc.exitCode();
        qDebug() << "Copy process output:" << copyProc.readAllStandardOutput();
        qDebug() << "Copy process errors:" << copyProc.readAllStandardError();

        if (copyProc.exitCode() != 0) {
            QMessageBox::warning(this, "Error", "Failed to copy pacman.conf to working directory:\n" +
                                                    copyProc.readAllStandardError());
            return;
        }

        // Immediately fix ownership so the file is writeable by the current user.
        QProcess fixPermissions;
        // Use the current user from the environment (e.g., "angel")
        QString user = qgetenv("USER");
        qDebug() << "Fixing permissions for user:" << user;
        fixPermissions.start("pkexec", QStringList() << "chown" << user + ":" + user << configPath);
        fixPermissions.waitForFinished();
        qDebug() << "chown exit code:" << fixPermissions.exitCode();
        qDebug() << "chown output:" << fixPermissions.readAllStandardOutput();
        qDebug() << "chown errors:" << fixPermissions.readAllStandardError();

        fixPermissions.start("pkexec", QStringList() << "chmod" << "644" << configPath);
        fixPermissions.waitForFinished();
        qDebug() << "chmod exit code:" << fixPermissions.exitCode();
        qDebug() << "chmod output:" << fixPermissions.readAllStandardOutput();
        qDebug() << "chmod errors:" << fixPermissions.readAllStandardError();

        // Open the working copy in C++ for precise modification.
        QFile configFile(configPath);
        if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Failed to open working copy for reading.");
            return;
        }
        QString content = configFile.readAll();
        configFile.close();

        // Append the Chaotic AUR repository block if it's not already there.
        if (!content.contains("[chaotic-aur]")) {
            content.append("\n[chaotic-aur]\nInclude = /etc/pacman.d/chaotic-mirrorlist\n");
        }

        // Write back the modified configuration.
        if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QMessageBox::warning(this, "Error", "Failed to open working copy for writing.");
            return;
        }
        QTextStream out(&configFile);
        out << content;
        configFile.close();

        // Copy the modified configuration back to /etc/pacman.conf using pkexec.
        QProcess installConfig;
        installConfig.start("pkexec", QStringList() << "cp" << configPath << originalConfig);
        installConfig.waitForFinished();
        qDebug() << "InstallConfig exit code:" << installConfig.exitCode();
        qDebug() << "InstallConfig output:" << installConfig.readAllStandardOutput();
        qDebug() << "InstallConfig errors:" << installConfig.readAllStandardError();

        if (installConfig.exitCode() != 0) {
            QMessageBox::warning(this, "Error", "Failed to update /etc/pacman.conf:\n" +
                                                    installConfig.readAllStandardError());
            return;
        }

        if (installConfig.exitCode() == 0) {
            QMessageBox::information(this, "Chaotic AUR Added",
                                     "Chaotic AUR repositories have been successfully set up in Xray_OS");
        } else {
            QMessageBox::warning(this, "Error", "Error adding Chaotic AUR:\n" + addProc.readAllStandardError());
        }

        return;
    }

    // For a partial/failed setup, attempt to repair.
    else if (status == 3) {
        // If the local chaotic-mirrorlist does not exist, clean any broken entries.
        if (!QFile::exists("/etc/pacman.d/chaotic-mirrorlist")) {
            QProcess removeRepo;
            qDebug() << "Executing: pkexec sed -i '/[chaotic-aur]/,+1d' /etc/pacman.conf'";

            removeRepo.start("pkexec", QStringList() << "bash" << "-c" << QString("sed -i '/\\[chaotic-aur\\]/,+1d' /etc/pacman.conf"));
            removeRepo.waitForFinished();

            qDebug() << "removeRepo output:" << removeRepo.readAllStandardOutput();
            qDebug() << "removeRepo errors:" << removeRepo.readAllStandardError();
        }

        // Reinstall packages and re-import the key.
        QProcess repairProc;
        QString repairCmd =
            "pkexec pacman-key --recv-key 3056513887B78AEB --keyserver keyserver.ubuntu.com && "
            "pkexec pacman-key --lsign-key 3056513887B78AEB &&"
            "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-keyring.pkg.tar.zst --noconfirm && "
            "pkexec pacman -U https://cdn-mirror.chaotic.cx/chaotic-aur/chaotic-mirrorlist.pkg.tar.zst --noconfirm";
        repairProc.start("bash", QStringList() << "-c" << repairCmd);
        repairProc.waitForFinished();
        qDebug() << "repairProc output:" << repairProc.readAllStandardOutput();
        qDebug() << "repairProc errors:" << repairProc.readAllStandardError();

        QProcess checkConf;
        checkConf.start("bash", QStringList() << "-c" << "grep '[chaotic-aur]' /etc/pacman.conf");
        checkConf.waitForFinished();
        qDebug() << "Pacman.conf after removeRepo:" << checkConf.readAllStandardOutput();

        // Restore the pacman.conf from your backup store.
        QProcess restoreProc;
        QString homePath = QDir::homePath();
        // Build the restore command as a single string to avoid splitting issues.
        QString restoreCmd = QString("cp -r %1/tolitica-home-settings/backups/current-use/pacman.conf /etc/pacman.conf").arg(homePath);
        qDebug() << "Restore command:" << restoreCmd;
        restoreProc.start("pkexec", QStringList() << "bash" << "-c" << restoreCmd);
        restoreProc.waitForFinished();
        qDebug() << "restoreProc exit code:" << restoreProc.exitCode();
        qDebug() << "restoreProc output:" << restoreProc.readAllStandardOutput();
        qDebug() << "restoreProc errors:" << restoreProc.readAllStandardError();

        if (restoreProc.exitCode() == 0) {
            QMessageBox::information(this, "Restore Complete", "Chaotic AUR has been repaired!");
        } else {
            QMessageBox::warning(this, "Error", "Something went wrong restoring pacman.conf:\n" +
                                                    restoreProc.readAllStandardError());
        }
    }
}



///////////////////////////////////////////////////
/// ADDONS:: CHECK VMWARE SERVICES STATUS
//////////////////////////////////////////////////
bool Widget::vmwareServiceStatus() {

    bool allServicesActive = true;

    QProcess checkEnabled;
    checkEnabled.start("bash", QStringList() << "-c" << "systemctl is-enabled vmware-usbarbitrator.service");
    checkEnabled.waitForFinished();
    bool isEnabled = (checkEnabled.readAllStandardOutput().trimmed() == "enabled");

    QProcess checkActive;
    checkActive.start("bash", QStringList() << "-c" << "systemctl is-active vmware-usbarbitrator.service");
    checkActive.waitForFinished();
    bool isActive = (checkActive.readAllStandardOutput().trimmed() == "active");

    if (!isActive || !isEnabled) {
        allServicesActive = false;
    }

    return allServicesActive;
}

///////////////////////////////////////////////////
/// ADDONS:: CHECK VMWARE STATUS
//////////////////////////////////////////////////
bool Widget::vmwareStatus() {
    QProcess checkVMware;
    checkVMware.start("bash", QStringList() << "-c" << "pacman -Q vmware-workstation");
    checkVMware.waitForFinished();

    if (checkVMware.exitCode() == 0) {
        return true;
    }
    else {
        return false;
    }
}

///////////////////////////////////////////////////
/// ADDONS:: ADD VMWARE SUPPORT
//////////////////////////////////////////////////
void Widget::addVMware(QPushButton *vmwButton) {
    QProcess checkIssues;

    // Check DB sync
    checkIssues.start("bash", QStringList() << "-c" << "pacman -Sy --dbonly");
    checkIssues.waitForFinished();
    QString dbSyncErrors = checkIssues.readAllStandardError();

    bool dbNotSynced = !dbSyncErrors.isEmpty();

    QDir pkgCacheDir("/var/cache/pacman/pkg");
    bool cacheExists = pkgCacheDir.exists() && !pkgCacheDir.isEmpty();

    if (cacheExists) {
        // Check for PKG corruption
        checkIssues.start("bash", QStringList() << "-c" << "pacman -Qk");
        checkIssues.waitForFinished();
        QString corruptionErrors = checkIssues.readAllStandardError();
        bool corruptedPackages = !corruptionErrors.isEmpty();

        // cleanup

        if (dbNotSynced || (cacheExists && corruptedPackages)) {
            qDebug() << "Issues detected! Cleaning package cache.";
            QProcess cleanup;

            if (dbNotSynced) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "pacman -Sy");
                cleanup.waitForFinished();

                if (cleanup.exitCode() != 0) {
                    qDebug() << "Cleanup errors(pacman -Sy): " << cleanup.readAllStandardError();
                }
            }
            if (corruptedPackages) {
                cleanup.start("pkexec", QStringList() << "bash" << "-c" << "sudo rm -rf /var/cache/pacman/pkg/* && sudo pacman -Scc --noconfirm");
                cleanup.waitForFinished();
            }

            qDebug() << "Cleanup output:" << cleanup.readAllStandardOutput();
            qDebug() << "Cleanup erros:" << cleanup.readAllStandardError();

        } else {
            qDebug() << "System database and package cache are fine!";
        }
    }

    bool status = vmwareStatus();
    bool servicesStatus = vmwareServiceStatus();

    QMessageBox::StandardButton reply;

    if (status && servicesStatus) {
        reply = QMessageBox::question(this, "Remove VMware Workstation",
                                      "Are you sure you want to remove VMware Workstation",
                                    QMessageBox::Yes | QMessageBox::No);
    }
    else if (!status && !servicesStatus) {
        reply = QMessageBox::question(this, "Install VMware Workstation", "VMware Workstation is not installed, do you want to install it?",
                                      QMessageBox::Yes | QMessageBox::No);
    }
    else if (!status || !servicesStatus) {
        if (!status) {
            reply = QMessageBox::question(this, "Install VMware Workstation", "VMware Workstation is not installed, do you want to install it?",
            QMessageBox::Yes | QMessageBox::No);
        } else {
            reply = QMessageBox::question(this, "VMware services", "VMware Workstation services are not enabled, do you want to enable them?",
                                          QMessageBox::Yes | QMessageBox::No);
        }
    }


    if (reply == QMessageBox::No)
        return;

    // Create process objects and a timer to monitor installation progress
    QProcess *installVMware = new QProcess(this);
    QTimer *monitorTimer = new QTimer(this);
    QProgressDialog *progress = nullptr;

    // Display progress dialog with the message
    if (status && servicesStatus) {
        progress = new QProgressDialog("Removing VMware Workstation...", nullptr, 0, 100, this);
    }
    else if (!status && !servicesStatus) {
        progress = new QProgressDialog("Installing VMware Workstation...", nullptr, 0, 100, this);
    }

    // one or the other... then adjust according
    else if (!status || !servicesStatus) {
        if (!status) {
            progress = new QProgressDialog("Installing VMware Workstation...", nullptr, 0, 100, this);
        } else {
            progress = new QProgressDialog("Enabling VMware Workstation services...", nullptr, 0, 100, this);
        }
    }

    progress->setWindowModality(Qt::ApplicationModal);
    progress->setCancelButton(nullptr);
    progress->setValue(0);
    progress->show();
    QCoreApplication::processEvents(); // Force immediate rendering

    int progressValue = 0;

    // Increase the progress bar as process output comes in.
    connect(installVMware, &QProcess::readyReadStandardOutput, this, [=]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents();
    });

    // Use a timer to simulate progress in case output is sparse.
    connect(monitorTimer, &QTimer::timeout, this, [=]() mutable {
        if (progressValue < 95) {
            progressValue += 2;
            progress->setValue(qMin(progressValue, 95));
        }
    });

    // When the installation process finishes;
    connect(installVMware, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    this, [=](int exitCode, QProcess::ExitStatus status) mutable {
        progress->setValue(100);
        QCoreApplication::processEvents(); // Force UI to process the update before displayin the message
        progress->close();

        // Cleanup dynamic objects.
        progress->deleteLater();
        installVMware->deleteLater();
        monitorTimer->deleteLater();

        bool updatedStatus = vmwareStatus() && vmwareServiceStatus();
        vmwButton->setText(updatedStatus ? "Remove VMware Workstation"
                                         : "Install/Enable VMware Workstation");

        if (installVMware->exitCode() == 0) {
            QMessageBox::information(nullptr, "Operation successful!",
                                     "VMware Workstation operations completed successfully");
        } else {
            QMessageBox::warning(nullptr, "Error", "Something went wrong installing VMware Workstation");
        }
    });

    if (!status && !servicesStatus) {

        installVMware->start("pkexec", QStringList() << "bash" << "-c" <<
                                           "pacman -S vmware-workstation --noconfirm &&"
                                           "pkexec systemctl enable vmware-networks-configuration.service && "
                                           "pkexec systemctl start vmware-networks-configuration.service && "
                                           //"pkexec systemctl enable vmware-networks.service && "
                                           //"pkexec systemctl start vmware-networks.service && "
                                           "pkexec systemctl enable vmware-usbarbitrator.service && "
                                           "pkexec systemctl start vmware-usbarbitrator.service");
        installVMware->waitForFinished();
        qDebug() << "OUTPUT: " << installVMware->readAllStandardOutput();
        qDebug() << "ERROR: " << installVMware->readAllStandardError();
    }
    else if (status && !servicesStatus) {

        installVMware->start("pkexec", QStringList() << "bash" << "-c" <<
                                           "pkexec systemctl enable vmware-networks-configuration.service && "
                                           "pkexec systemctl start vmware-networks-configuration.service && "
                                           //"pkexec systemctl enable vmware-networks.service && "
                                           //"pkexec systemctl start vmware-networks.service && "
                                           "pkexec systemctl enable vmware-usbarbitrator.service && "
                                           "pkexec systemctl start vmware-usbarbitrator.service");
        installVMware->waitForFinished();
        qDebug() << "OUTPUT: " << installVMware->readAllStandardOutput();
        qDebug() << "ERROR: " << installVMware->readAllStandardError();

    } else {
        // Removing the package also disable the services.. (no need for manual adjustment)
        installVMware->start("pkexec", QStringList() << "bash" << "-c" <<
                                           "pacman -Rns vmware-workstation --noconfirm && "
                                           //"systemctl stop vmware-networks.service && "
                                           "systemctl stop vmware-usbarbitrator.service && "
                                           "rm -rf /etc/systemd/system/vmware-networks.service && "
                                           "rm -rf /etc/systemd/system/vmware-usbarbitrator.service && "
                                           "rm -rf /etc/vmware && "
                                           "rm -rf /usr/lib/vmware && "
                                           // "sudo mkinitcpio -P"
                                           "systemctl daemon-reload && "
                                           "systemctl reset-failed");
        installVMware->waitForFinished();
        qDebug() << "OUTPUT: " << installVMware->readAllStandardOutput();
        qDebug() << "ERROR: " << installVMware->readAllStandardError();

    }

    // bool newStatus = (vmwareStatus() && vmwareServiceStatus());
    // vmwButton->setText(newStatus ? "Remove VMware Workstation"
    //                              : "Install/Enable VMware Workstation");

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

    QString targetLine = "oh-my-posh init fish --config $HOME/.config/oh-my-posh-themes/arch-atomic.omp.json";
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

    // If there is no actual arch-atomic.omp.json file
    if (!QFile::exists(QDir::homePath() + "/.config/oh-my-posh-themes/arch-atomic.omp.json")) {
        QMessageBox::warning(this, "arch-atomic.omp.json is Missing!",
                                   "arch-atomic.omp.json is missing from /.config/oh-my-posh-themes");
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
    QString targetLine = "oh-my-posh init fish --config $HOME/.config/oh-my-posh-themes/arch-atomic.omp.json";

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
    terminalThemeButton->setText(status == 1 ? "Enable Terminal Theming (Fish ONLY)" : "Disable Terminal Theming (Fish ONLY)");
    QMessageBox::information(this, "Success", "Terminal theming successfully updated");
}

////////////////////////////////////////////////////////
/// GENERAL: DISABLE/ENABLE AUTOSTART
//////////////////////////////////////////////////////
bool Widget::autostart() {
    QString desktopFilePath = QDir::homePath() + "/.config/autostart/tolitica.desktop";
    QFile desktopFile(desktopFilePath);

    bool autoS = desktopFile.exists();
    qDebug() << "Checking autostart file:" << desktopFilePath << "exists:" << autoS;

    if (!autoS) {
        if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to create .desktop file:" << desktopFilePath;
            return false;
        }

        QTextStream out(&desktopFile);
        out << "[Desktop Entry]\n";
        out << "Type=Application\n";
        out << "Exec=/opt/tolitica/tolitica\n";
        out << "Hidden=false\n";
        out << "NoDisplay=false\n";
        out << "X-GNOME-Autostart-enabled=true\n";
        out << "Name=Tolitica\n";
        out << "Comment=Tolitica Xray_OS Assistant\n";
        out << "Icon=/usr/share/icons/Arch7z-Custom-Icons/tolitica-icon.png\n";
        desktopFile.close();

        qDebug() << "Created .desktop file at" << desktopFilePath;

        return true;
    }
    else {
        if (desktopFile.remove()) {
            qDebug() << "Deleted .desktop file successfully: " << desktopFilePath;
            return false;
        } else {
            qWarning() << "Failed to delete .desktop file: " << desktopFilePath;
            return true;
        }
    }
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

    setWindowTitle("Tolitica Xray_OS Assistant");
    resize(800,600);
    setWindowIcon(QIcon(":/icons/resources/icons/tolitica-icon.png"));

    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(this);

    //////////////////////////////////////
    /// CALAMARES UI ---------////////////
    //////////////////////////////////////
    QProcess process;
    process.start("bash", QStringList() << "-c" << "grep -q '/cow' /proc/mounts || [ -f /run/live/medium ]");
    process.waitForFinished();

    bool isLiveEnv = (process.exitCode() == 0);
    QString word = "tolitica";

    if (isLiveEnv && word == "tolitica") {
        QStackedWidget *calStackedWidget = new QStackedWidget();
        mainWidgetLayout->addWidget(calStackedWidget);

        QWidget *calamaresContainer = new QWidget();
        QVBoxLayout *calamaresContainerLayout = new QVBoxLayout(calamaresContainer);
        calamares_page *calamaresPage = new calamares_page(nullptr);

        calamaresContainerLayout->addWidget(calamaresPage);
        calamaresContainer->setLayout(calamaresContainerLayout);

        calStackedWidget->addWidget(calamaresContainer);
        calStackedWidget->setCurrentIndex(0);

        // Redirects to the calamares page
    } else {
        //////////////////////////////////////
        /// MAIN UI ---------/////////////////
        //////////////////////////////////////

        // Create a stacked widget to hold the different pages
        QStackedWidget *stackedWidget = new QStackedWidget(this);
        // Setting initial index to 0 (regular UI)
        stackedWidget->setCurrentIndex(0);

        // Create all pages
        // Use fixed width for mainPage to force 800px width.
        QWidget *mainPage = new QWidget();
        mainPage->setFixedWidth(800);
        QWidget *tweaksPage = new QWidget();
        QWidget *addonsPage = new QWidget();
        QWidget *terminalPage = new QWidget();

        // Add pages to the stacked widget
        stackedWidget->addWidget(mainPage);
        stackedWidget->addWidget(tweaksPage);
        stackedWidget->addWidget(addonsPage);
        stackedWidget->addWidget(terminalPage);

        // Wrap the stacked widget in a centering container
        QWidget *centerContainer = new QWidget(this);
        QHBoxLayout *centerLayout = new QHBoxLayout(centerContainer);
        centerLayout->setContentsMargins(0, 0, 0, 0);
        centerLayout->addStretch(); // Left spacer
        centerLayout->addWidget(stackedWidget);
        centerLayout->addStretch(); // Right spacer

        // Add the centering container to the main layout
        mainWidgetLayout->addWidget(centerContainer, 0, Qt::AlignCenter);
        setLayout(mainWidgetLayout);

        // ==== Main Page Content ====
        QVBoxLayout *mainLayout = new QVBoxLayout(mainPage);

        // ==== Header and Description ==== //
        QLabel *headerLabel = new QLabel("<h1>Welcome to Tolitica Xray_OS Assistant!</h1>", this);
        // == Description == //
        QLabel *greetingsLabel = new QLabel("<span style=\"font-size:11pt; font-weight:bold;\">Greetings from Angel!</span> - Owner & Maintainer of Xray_OS", this);
        QLabel *descriptionLabel = new QLabel("With this helper application you can tweak several "
                                              "configurations from your system, please enjoy using "
                                              "Xray_OS, break it, repair it or donate to me... Anyway have fun using my ArchLinux distro.", this);
        headerLabel->setContentsMargins(0, 0, 0, 0);
        greetingsLabel->setContentsMargins(0, 5, 0, 0);
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
        // mainLayout->addWidget(mountDriveWidget, 0, Qt::AlignLeft);

        /* === Navigation Buttons === */
        QVBoxLayout *buttonsLayout = new QVBoxLayout(); // Specific layout for buttons
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
        // socialMediaLayout->addWidget(instagramButton);
        socialMediaLayout->setSpacing(2);
        socialMediaLayout->setAlignment(Qt::AlignLeft);

        // Adding the horizontal layout to the main one
        mainLayout->addLayout(socialMediaLayout);

        // === Connect signals for social media buttons === //
        connect(discordButton, &QToolButton::clicked, this, [=](){
            coreFunctions->socialMedia("discord");
        });
        connect(twitterButton, &QToolButton::clicked, this, [=](){
            coreFunctions->socialMedia("twitter");
        });
        connect(youtubeButton, &QToolButton::clicked, this, [=](){
            coreFunctions->socialMedia("youtube");
        });

        // Disable/Enable at Startup
        QCheckBox *disabledStartup = new QCheckBox(this);
        mainLayout->addWidget(disabledStartup, 3, Qt::AlignRight);
        // Determine the current autostart state by checking if the .desktop file exists.
        QString desktopFilePath = QDir::homePath() + "/.config/autostart/tolitica.desktop";
        QFile desktopFile(desktopFilePath);
        bool currentAutoState = desktopFile.exists();

        disabledStartup->setText(currentAutoState ? "Disable the app at startup" :
                                     "Fire the app at startup");
        disabledStartup->setChecked(currentAutoState);

        // === Connect the toggled signal from the QCheckBox === //
        connect(disabledStartup, &QCheckBox::toggled, this, [this, disabledStartup](bool /*checked*/){
            bool newState = autostart();
            disabledStartup->setText(newState ? "Disable the app at startup" : "Fire the app at startup");
            disabledStartup->setChecked(newState);
        });

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // ==== Tweaks Page =====
        ///////////////////////////////////////////////////////////////////////////////////////////////
        QGridLayout *tweaksLayout = new QGridLayout(tweaksPage);
        QPushButton *backButton = new QPushButton("Back", this);

        // Functional Buttons Tweaks Layout
        QPushButton *cleanOrphansButton = new QPushButton("Clean Unused Packages", this);
        QPushButton *cleanPkgCacheButton = new QPushButton("Clean Package Cache", this);
        QPushButton *updateSystemButton = new QPushButton("Update Xray_OS", this);
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
        int aaStatus = CoreFunctions::apparmorStatus();
        qDebug() << "Initial AppArmor status:" << aaStatus;

        if (aaStatus == 0) {
            // Not supported – leave unchecked and prompt text "Enable AppArmor"
            apparmorToggle->setChecked(false);
            apparmorToggle->setText("Enable AppArmor");
        } else if (aaStatus == 1) {
            // AppArmor is enabled
            apparmorToggle->setChecked(true);
            apparmorToggle->setText("Disable AppArmor");
        } else if (aaStatus == 2) {
            // AppArmor is disabled
            apparmorToggle->setChecked(false);
            apparmorToggle->setText("Enable AppArmor");
        } else {
            // For any other status (e.g., 3 means partially enabled or custom) – decide on a default:
            apparmorToggle->setChecked(false);
            apparmorToggle->setText("Enable AppArmor");
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
        QGridLayout *terminalLayout = new QGridLayout(terminalPage);
        QPushButton *terminalBackButton = new QPushButton("Back", this);

        // ** Functional Buttons Terminal Layout ** //
        // *Enable/Disable Terminal Theming

        QString shell = CoreFunctions::getCurrentShell();
        bool isFsh = (shell == "/bin/fish");

        QPushButton *terminalThemeButton = new QPushButton("Disable Terminal Theming (Fish ONLY)", this);
        terminalThemeButton->setEnabled(isFsh);

        if (isFsh) {
            int checkTermStatus = checkTermThemingStatus();

            if (checkTermStatus == 0) {
                terminalThemeButton->setText("No config.fish found");
            } else {
                terminalThemeButton->setText(checkTermStatus == 1 ? "Disable Terminal Theming (Fish ONLY)" :
                                             "Enable Terminal Theming");
            }
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
        QGridLayout *addonsLayout = new QGridLayout(addonsPage);
        QPushButton *addonsBackButton = new QPushButton("Back", this);

        // ** Functional Buttons Addons Layout ** //
        // *Arch7 Gaming Meta
        QPushButton *archZGamingMetaButton = new QPushButton(this);
        QProcess checkAGMinstalled;
        checkAGMinstalled.start("bash", QStringList() << "-c" << "pacman -Q arch7z-gaming-meta");
        checkAGMinstalled.waitForFinished();

        if (checkAGMinstalled.exitCode() == 0) {
            archZGamingMetaButton->setText("Remove Arch7z Gaming Meta");
        } else {
            archZGamingMetaButton->setText("Install Arch7z Gaming Meta");
        }
        // *Arch7z Development Meta
        QPushButton *archZDevelopmentMetaButton = new QPushButton(this);

        QProcess checkADMinstalled;
        checkADMinstalled.start("bash", QStringList() << "-c" << "pacman -Q arch7z-development-meta");
        checkADMinstalled.waitForFinished();

        if (checkADMinstalled.exitCode() == 0) {
            archZDevelopmentMetaButton->setText("Remove Arch7z Development Meta");
        } else {
            archZDevelopmentMetaButton->setText("Install Arhc7z Development Meta");
        }
        // *ChaoticAUR Button
        QPushButton *chaoticAURbutton = new QPushButton(this);
        int chaoticStatus = checkChaoticAURStatus();
        chaoticAURbutton->setText(chaoticStatus == 1 ? "Remove Chaotic AUR" :
                                      chaoticStatus == 2 ? "Add Chaotic AUR" :
                                      "Repair Chaotic AUR");
        // **Add VMware Support**//
        QPushButton *vmwButton = new QPushButton(this);
        bool vmStatus = vmwareStatus() && vmwareServiceStatus();

        vmwButton->setText((vmStatus) ? "Remove VMware Workstation" : "Install/Enable VMware Workstation");

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
        addonsLayout->addWidget(archZGamingMetaButton, 1, 0, Qt::AlignLeft);
        addonsLayout->addWidget(archZDevelopmentMetaButton, 1, 0, Qt::AlignCenter);
        addonsLayout->addWidget(chaoticAURbutton, 1, 0, Qt::AlignRight);
        addonsLayout->addWidget(vmwButton, 2, 0, Qt::AlignCenter);

        // Push Back-button to bottom dynamically
        addonsLayout->setRowStretch(4, 1);
        addonsLayout->addWidget(addonsBackButton, 5, 0, Qt::AlignLeft);
        addonsPage->setLayout(addonsLayout);

        /* === Connections === */
        addonsSetupConnections(stackedWidget, addonsButton, addonsBackButton, archZGamingMetaButton,
                               archZDevelopmentMetaButton, chaoticAURbutton, vmwButton, flatpakToggle, snapdToggle);

        ///////////////////////////////////////////////////////////////////////////////////////////////
        // ==== Mount Drives Page =====
        ///////////////////////////////////////////////////////////////////////////////////////////////

        mountDrivesSetupConnections(stackedWidget, mountDriveButton);
    }

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
        // Block signals so that changes made during the process don’t trigger unwanted updates
        appArmorToggle->blockSignals(true);
        CoreFunctions::enableAppArmor(this, appArmorToggle);
        appArmorToggle->blockSignals(false);
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
                                    QPushButton *archZGamingMetaButton, QPushButton *archZDevelopmentButton, QPushButton *chaoticAURbutton,
                                    QPushButton *vmwButton, QCheckBox *flatpakToggle, QCheckBox *snapdToggle) {
        // Navigation connections
        connect(addonsButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(2); // Switch to Addons page
    });
        connect(addonsBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0); // Switch back to Main page
    });

        // Connecting Buttons to their respective Functions
        //** Arch7z Gaming Meta **//
        connect(archZGamingMetaButton, &QPushButton::clicked, this, [=]() mutable {
        if(archZGamingMetaButton->text() == "Install Arch7z Gaming Meta") {
            archZGamingMetaButton->setText("Remove Arch7z Gaming Meta");
            archZGamingMeta();
        } else {
            archZGamingMetaButton->setText("Install Arch7z Gaming Meta");
            removeArchZGamingMeta();
        }
    });
        //** Arch7z Development Meta **//
        connect(archZDevelopmentButton, &QPushButton::clicked, this, [=]() mutable {
        if(archZDevelopmentButton->text() == "Install Arch7z Development Meta") {
            archZDevelopmentButton->setText("Remove Arch7z Development Meta");
            archZDevelopmentMeta();
        } else {
            archZDevelopmentButton->setText("Install Arch7z Development Meta");
            removeArchZDevelopmentMeta();
        }
    });

        //** Chaotic AUR **//
        connect(chaoticAURbutton, &QPushButton::clicked, this, [=]() mutable {
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
            chaoticAURbutton->setText(chaoticStatus == 1 ? "Remove Chaotic AUR" :
                                          chaoticStatus == 2 ? "Add Chaotic AUR" :
                                          "Repair Chaotic AUR");
        });
        //** Add VMware Support **//
        connect(vmwButton, &QPushButton::clicked, this, [=]() mutable {
        int vmStatus = vmwareStatus();

            if (vmStatus == 0) {
                addVMware(vmwButton);
            } else {
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
void Widget::mountDrivesSetupConnections(QStackedWidget *stackedWidget,
                                         QToolButton *mountDriveButton) {
    connect(mountDriveButton, &QToolButton::clicked, this, [this, stackedWidget]() {
        if (!mountDrivesPage) {
            mountDrivesPage = new QWidget(this);
            QVBoxLayout *mainLayout = new QVBoxLayout(mountDrivesPage);
            mainLayout->setContentsMargins(0, 0, 0, 0);

            // INITIALIZE OR REFRESH THE DRIVES PAGE
            if (!drivesPage) {
                drivesPage = new drive_list_widget(mountDrivesPage);
            } else {
                drivesPage->refresh();
            }
            mainLayout->addWidget(drivesPage, 1);

            QHBoxLayout *bottomLayout = new QHBoxLayout();
            QPushButton *mountDrivesBackButton = new QPushButton("Back", mountDrivesPage);
            bottomLayout->addWidget(mountDrivesBackButton, 0, Qt::AlignLeft);
            bottomLayout->addStretch();
            QPushButton *showAdditionalButton = new QPushButton("Show Additional Partitions", mountDrivesPage);
            bottomLayout->addWidget(showAdditionalButton, 0, Qt::AlignCenter);
            bottomLayout->addStretch();
            QPushButton *mountUnmountButton = new QPushButton("Mount/Unmount", mountDrivesPage);
            mountUnmountButton->setObjectName("mountUnmountButton");
            mountUnmountButton->setEnabled(false);
            bottomLayout->addWidget(mountUnmountButton, 0, Qt::AlignRight);

            mainLayout->addLayout(bottomLayout);
            mountDrivesPage->setLayout(mainLayout);
            stackedWidget->insertWidget(4, mountDrivesPage);

            connect(mountDrivesBackButton, &QPushButton::clicked, this, [stackedWidget]() {
                stackedWidget->setCurrentIndex(0);
            });

            // CONNECT SELECTION CHANGED SIGNAL TO UPDATE THE MOUNT/UNMOUNT BUTTON
            connect(drivesPage, &drive_list_widget::selectionChanged, this, [this, mountUnmountButton]() {
                bool mod = drivesPage->isModified();
                qDebug() << "SELECTION CHANGED CALLED - isModified():" << mod;
                bool dangerous = drivesPage->isDangerousModified();
                qDebug() << "DANGEROUS STATE:" << dangerous;
                if (mod) {
                    mountUnmountButton->setEnabled(true);
                    mountUnmountButton->setProperty("dangerousState", dangerous);
                } else {
                    mountUnmountButton->setEnabled(false);
                    mountUnmountButton->setProperty("dangerousState", false);
                }
                // APPLY STYLING BASED ON DANGEROUS STATE
                if (mountUnmountButton->property("dangerousState").toBool())
                    mountUnmountButton->setStyleSheet("background-color: red;");
                else
                    mountUnmountButton->setStyleSheet("");
                qDebug() << "MOUNT/UNMOUNT BUTTON ENABLED:" << mountUnmountButton->isEnabled();
            });

            connect(showAdditionalButton, &QPushButton::clicked, this, [this]() {
                drivesPage->showAdditionalPartitionsDialog();
            });

            // MOUNT/UNMOUNT BUTTON CLICK HANDLER
            connect(mountUnmountButton, &QPushButton::clicked, this, [this, mountUnmountButton]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, "Mount/Unmount", "Are you sure you want to proceed?",
                                              QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    if (drivesPage->applyMountSelection()) {
                        // If the operation was canceled (for example, pkexec cancelled),
                        // simply refresh and exit silently.
                        if (drivesPage->operationCancelled()) {
                            drivesPage->refresh();
                            return;
                        }
                        drivesPage->refresh();

                        bool mod = drivesPage->isModified();
                        mountUnmountButton->setEnabled(mod);
                        bool dangerous = drivesPage->isDangerousModified();
                        mountUnmountButton->setProperty("dangerousState", dangerous);
                        mountUnmountButton->setStyleSheet(dangerous ? "background-color: red;" : "");

                        QMessageBox::information(this, "Mount/Unmount",
                                                 "The process has been successfully completed!");
                    } else {
                        // Instead of displaying error dialogs, just refresh the drive page.
                        drivesPage->refresh();
                    }
                }
            });

        } else {
            // IF THE PAGE ALREADY EXISTS, REFRESH THE DRIVE PAGE
            drivesPage->refresh();

            // UPDATE MOUNT/UNMOUNT BUTTON STATE
            QPushButton *mountUnmountButton = mountDrivesPage->findChild<QPushButton*>("mountUnmountButton");
            if (mountUnmountButton) {
                bool mod = drivesPage->isModified();
                qDebug() << "PAGE EXISTS - isModified():" << mod;
                if (mod) {
                    mountUnmountButton->setEnabled(true);
                    mountUnmountButton->setProperty("dangerousState", drivesPage->isDangerousModified());
                    if (mountUnmountButton->property("dangerousState").toBool())
                        mountUnmountButton->setStyleSheet("background-color: red;");
                    else
                        mountUnmountButton->setStyleSheet("");
                } else {
                    mountUnmountButton->setEnabled(false);
                    mountUnmountButton->setStyleSheet("");
                }
                qDebug() << "MOUNT/UNMOUNT BUTTON (EXISTING PAGE) ENABLED:" << mountUnmountButton->isEnabled();
            }
        }
        stackedWidget->setCurrentIndex(4);
    });
}





Widget::~Widget()
{
    delete ui;
}
