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

///////////////////////////////////////////////////
/// CLEAN ORPHANS FUCNTION
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
/// CLEAN PACKAGE CACHE FUNCTION
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
/// SYSTEM UPDATE FUNCTION
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
/// REMOVE db.lck FUNCTION
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

// == I LOVE CPP ==================================
///////////////////////////////////////////////////
/// MAIN FUNCTION
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
    QPushButton *AddonsButton = new QPushButton("Addons", this);

    buttonsLayout->addWidget(tweaksButton);
    buttonsLayout->addWidget(AddonsButton);
    mainLayout->addLayout(buttonsLayout);
    mainPage->setLayout(mainLayout);

    // ==== Tweaks Page =====
    QWidget *tweaksPage = new QWidget();
    QGridLayout *tweaksLayout = new QGridLayout(tweaksPage);
    QPushButton *backButton = new QPushButton("Back", this);
    tweaksLayout->addWidget(backButton);

    // Functional Buttons
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

    // ==== Addons Page === //
    QWidget *addonsPage = new QWidget();
    QGridLayout *addonsLayout = new QGridLayout(addonsPage);
    addonsPage->setLayout(addonsLayout);

    /* === Connections === */
    setupConnections(stackedWidget, tweaksButton, AddonsButton, backButton, cleanOrphansButton,
                     cleanPkgCacheButton, updateSystemButton, removeDBLockButton);

    // Add pages to stacked widget
    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(tweaksPage);
    stackedWidget->addWidget(addonsPage);

    // Set the initial layout
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(this);
    mainWidgetLayout->addWidget(stackedWidget);
    setLayout(mainWidgetLayout);
}

void Widget::setupConnections(QStackedWidget *stackedWidget, QPushButton *tweaksButton, QPushButton *addonsButton,
                              QPushButton *backButton, QPushButton *cleanOrphansButton, QPushButton *cleanPkgCacheButton,
                              QPushButton *updateSystemButton, QPushButton *removeDBLockButton){
    // Navigation connections
    connect(tweaksButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(1); // Switch to Tweaks page
    });
    connect(addonsButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(2); // Switch to Addons page
    });
    connect(backButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0); // Switch back to Main page
    });

    connect(cleanOrphansButton, &QPushButton::clicked, this, &Widget::cleanOrphans);
    connect(cleanPkgCacheButton, &QPushButton::clicked, this, &Widget::cleanPkgCache);
    connect(updateSystemButton, &QPushButton::clicked, this, &Widget::systemUpdate);
    connect(removeDBLockButton, &QPushButton::clicked, this, &Widget::removeDBLock);
}

Widget::~Widget()
{
    delete ui;
}
