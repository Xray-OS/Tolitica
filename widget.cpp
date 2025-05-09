#include "widget.h"
#include "./ui_widget.h"
#include <QStackedWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QProgressDialog>
#include <QTimer>
#include <QFile>
#include <QTextStream>

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
    setWindowTitle("Tolitica Ada Assistant");
    resize(800,600);
    setWindowIcon(QIcon(":/icons/resources/icons/tolitica-icon.png"));

    // Stacked widget to hold both layouts
    QStackedWidget *stackedWidget = new QStackedWidget(this);

    // ==== Main Page =====
    QWidget *mainPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainPage);

    /* === Navigation Buttons === */
    QVBoxLayout *buttonsLayout = new QVBoxLayout(); // Specific Layout for Buttons
    QPushButton *tweaksButton = new QPushButton("Configuration", this);
    QPushButton *addonsButton = new QPushButton("Addons", this);

    buttonsLayout->addWidget(tweaksButton);
    buttonsLayout->addWidget(addonsButton);
    mainLayout->addLayout(buttonsLayout);
    mainPage->setLayout(mainLayout);

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

    /* === Positioning Buttons === */
    tweaksLayout->addWidget(cleanOrphansButton, 1, 0, Qt::AlignLeft);
    tweaksLayout->addWidget(cleanPkgCacheButton, 1, 0, Qt::AlignRight);
    tweaksLayout->addWidget(updateSystemButton, 2, 0, Qt::AlignCenter);
    tweaksLayout->addWidget(removeDBLockButton, 2, 0, Qt::AlignLeft);

    // Push Back-button to bottom dynamically
    tweaksLayout->setRowStretch(4, 1);
    tweaksLayout->addWidget(backButton, 5, 0, Qt::AlignLeft);
    tweaksPage->setLayout(tweaksLayout);

    tweaksSetupConnections(stackedWidget, tweaksButton, backButton, cleanOrphansButton,
                           cleanPkgCacheButton, updateSystemButton, removeDBLockButton);

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

    /* === Positioning Buttons === */
    addonsLayout->addWidget(adaGamingMetaButton);
    addonsLayout->addWidget(adaDevelopmentMetaButton);
    addonsLayout->addWidget(chaoticAURButton);

    // Push Back-button to bottom dynamically
    addonsLayout->setRowStretch(4, 1);
    addonsLayout->addWidget(addonsBackButton, 5, 0, Qt::AlignLeft);
    addonsPage->setLayout(addonsLayout);

    /* === Connections === */
    addonsSetupConnections(stackedWidget, addonsButton, addonsBackButton, adaGamingMetaButton,
                           adaDevelopmentMetaButton, chaoticAURButton);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== End Pages =====
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Add pages to stacked widget
    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(tweaksPage);
    stackedWidget->addWidget(addonsPage);

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

void Widget::tweaksSetupConnections(QStackedWidget *stackedWidget, QPushButton *tweaksButton, QPushButton *backButton,
                                    QPushButton *cleanOrphansButton, QPushButton *cleanPkgCacheButton,
                                    QPushButton *updateSystemButton, QPushButton *removeDBLockButton){
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
}

void Widget::addonsSetupConnections(QStackedWidget *stackedWidget, QPushButton *addonsButton, QPushButton *addonsBackButton,
                                    QPushButton *adaGamingMetaButton, QPushButton *adaDevelopmentButton, QPushButton *chaoticAURButton) {
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

        if (chaoticStatus == 2) { // Not installed
            chaoticAUR();
            chaoticAURButton->setText("Remove Chaotic AUR");
        } else if (chaoticStatus == 1) { // Fully installed
            removeChaoticAUR();
            chaoticAURButton->setText("Add Chaotic AUR");
        } else if (chaoticStatus == 3) { // Run repair
            chaoticAUR();

            // **Recheck status after repair**
            chaoticStatus = checkChaoticAURStatus();
            chaoticAURButton->setText(chaoticStatus == 1 ? "Remove Chaotic AUR" :
                                      chaoticStatus == 2 ? "Add Chaotic AUR" :
                                        "Repair Chaotic AUR");
        }
    });
}

Widget::~Widget()
{
    delete ui;
}
