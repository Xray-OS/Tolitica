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
/// SYSTEM UPDATE FUNCTION
//////////////////////////////////////////////////
void Widget::removeDBLock() {
    bool psAux = false;

    QProcess checkPacmanInUse;
    checkPacmanInUse->start("bash", QStringList() << "-c" << "ps aux | grep pacman");


}

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

    QPushButton *tweaksButton = new QPushButton("Configuration", this);
    mainLayout->addWidget(tweaksButton);

    // ==== Tweaks Page =====
    QWidget *tweaksPage = new QWidget();
    QVBoxLayout *tweaksLayout = new QVBoxLayout(tweaksPage);

    QPushButton *backButton = new QPushButton("Back", this);
    tweaksLayout->addWidget(backButton);

    // Clean orphan button
    QPushButton *cleanOrphansButton = new QPushButton("Clean Unused Packages", this);
    tweaksLayout->addWidget(cleanOrphansButton);
    // Clean Pkg Cache button
    QPushButton *cleanPkgCacheButton = new QPushButton("Clean Package Cache", this);
    tweaksLayout->addWidget(cleanPkgCacheButton);
    // Update system button
    QPushButton *updateSystemButton = new QPushButton("Update Ada", this);
    tweaksLayout->addWidget(updateSystemButton);
    // Remove DB Lock Button
    QPushButton *removeDBLockButton = new QPushButton("Remove DB Lock", this);
    tweaksLayout->addWidget(removeDBLockButton);

    // Add pages to stacked widget
    stackedWidget->addWidget(mainPage);
    stackedWidget->addWidget(tweaksPage);

    // Set the initial layout
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(this);
    mainWidgetLayout->addWidget(stackedWidget);
    setLayout(mainWidgetLayout);

    // ==== Connect Buttons =====
    connect(tweaksButton, &QPushButton::clicked, [stackedWidget]() {
        stackedWidget->setCurrentIndex(1); // Switch to tweaks page
    });

    connect(backButton, &QPushButton::clicked, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0); // Switch back to main page
    });

    connect(cleanOrphansButton, &QPushButton::clicked, this, &Widget::cleanOrphans);
    connect(cleanPkgCacheButton, &QPushButton::clicked, this, &Widget::cleanPkgCache);
    connect(updateSystemButton, &QPushButton::clicked, this, &Widget::systemUpdate);
    connect(removeDBLockButton, &QPushButton::clicked, this, &Widget::removeDbLock);
}

Widget::~Widget()
{
    delete ui;
}
