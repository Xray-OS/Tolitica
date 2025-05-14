#include "core_functions.h"
#include <QMessageBox>
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
#include <QComboBox>
#include <unistd.h>
#include <QInputDialog>
#include <QApplication>
#include <QObject>

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE TERMINAL PAGE /////////////////////// /////////////////////// ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// TERMINAL: GET THE CURRENT SHELL
//////////////////////////////////////////////////
QString CoreFunctions::getCurrentShell() {
    QProcess process;
    process.start("getent", QStringList() << "passwd" << qgetenv("USER"));
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();

    if (output.isEmpty()) {
        qDebug() << "Failed to fetch current shell";
        return "Unknown";
    }

    QStringList fields = output.split(":");
    return fields.last(); // Extracts shell from passwd entry
}

///////////////////////////////////////////////////
/// TERMINAL: GET THE SHELLS INSTALLED FUNCTION
//////////////////////////////////////////////////
QStringList CoreFunctions::getInstalledShells() {
    QStringList availableShells;
    QFile file("/etc/shells");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error opening /etc/shells";
        return availableShells;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("/bin")) { // Ensures we're parsing valid shell paths
            availableShells.append(line);
        }
    }
    file.close();
    return availableShells;
}

///////////////////////////////////////////////////
/// TERMINAL: CHANGE THE SHELL FUNCTION
//////////////////////////////////////////////////
void CoreFunctions::changeShell(QWidget *parent, const QString &selectedShell) {
    if (selectedShell.isEmpty()) {
        QMessageBox::warning(parent, "Shell Change", "No shell selected.");
        return;
    }
    QString username = qgetenv("USER");  // Get actual username

    qDebug() << "Attempting to change shell to:" << selectedShell;

    QProcess process;
    process.start("pkexec", QStringList() << "chsh" << "-s" << selectedShell << username); // Use `pkexec` for permissions
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QMessageBox::information(parent, "Shell Change", "Shell changed successfully to: " + selectedShell);
    } else {
        QMessageBox::warning(parent, "Shell Change", "Failed to change shell. Error:\n" + process.readAllStandardError());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE TWEAKS PAGE /////////////////////// /////////////////////// ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// TWEAKS: BLUETOOTH STATUS
/////////////////////////////////////////////////
int CoreFunctions::bluetoothStatus() {
    QProcess bluetoothService;
    bluetoothService.start("bash", QStringList() << "-c" << "systemctl is-enabled bluetooth.service");
    bluetoothService.waitForFinished();
    bool bluetoothEnabled = (bluetoothService.exitCode() == 0);

    bluetoothService.start("bash", QStringList() << "-c" << "systemctl is-active bluetooth.service");
    bluetoothService.waitForFinished();
    bool bluetoothActive = (bluetoothService.exitCode() == 0);

    if (bluetoothEnabled && bluetoothActive) {
        return 0;
    } else if (!bluetoothEnabled && !bluetoothActive) {
        return 1;
    } else {
        return 2;
    }
}



///////////////////////////////////////////////////
/// TWEAKS: ENABLE/DISABLE BLUETOOTH
//////////////////////////////////////////////////
void CoreFunctions::enableBluetooth(QWidget *parent, QCheckBox *bluetoothToggle) {
    int status = bluetoothStatus();

    QProcess process;
    QString command = (status == 0) ? "systemctl disable --now bluetooth.service" : "systemctl enable --now bluetooth.service";

    process.start("pkexec", QStringList() << "bash" << "-c" << command);
    process.waitForFinished();

    // ** Check process success or failure ** //
    if (process.exitCode() == 0) {
        bluetoothToggle->setChecked(status != 0);
        bluetoothToggle->setText(status != 0 ? "Disable Bluetooth" : "Enable Bluetooth");
        QMessageBox::information(parent, "Bluetooth",
                                 status == 0 ? "Bluetooth Disabled Successfully" :
                                 "Bluetooth Enabled Successfully");
    } else {
        bluetoothToggle->setChecked(status == 0);
        bluetoothToggle->setText(status == 0 ? "Disable Bluetooth" : "Enable Bluetooth");
        QMessageBox::warning(nullptr, "Error", "Failed to change Bluetooth state:\n" +
                                                   process.readAllStandardError());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE ADDONS PAGE /////////////////////// /////////////////////// ////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// ADDONS: CHECK FLATPAK STATUS
//////////////////////////////////////////////////
int CoreFunctions::flatpakStatus() {
    QProcess flatpakStatus;
    flatpakStatus.start("bash", QStringList() << "-c" << "pacman -Q flatpak");
    flatpakStatus.waitForFinished();
    bool pkgInstalled = (flatpakStatus.exitCode() == 0);

    flatpakStatus.start("bash", QStringList() << "-c" << "flatpak remotes | grep -q flathub");
    flatpakStatus.waitForFinished();
    bool repoSet = (flatpakStatus.exitCode() == 0);

    if (pkgInstalled && repoSet) {
        return 0;
    } else if (!pkgInstalled && !repoSet) {
        return 1;
    } else {
        return 2;
    }
}

///////////////////////////////////////////////////
/// ADDONS: ENABLE/DISABLE FLATPAK
//////////////////////////////////////////////////
void CoreFunctions::enableFlatpak(QWidget *parent, QCheckBox *flatpakToggle) {
    int status = flatpakStatus();

    QProcess dependencyCheck;
    dependencyCheck.start("bash", QStringList() << "-c" << "pacman -Qi flatpak | grep 'Required by' | cut -d':' -f2 | tr '\n' ' '");
    dependencyCheck.waitForFinished();
    QString dependencies = dependencyCheck.readAllStandardOutput().trimmed();

    // ** Warn user before removing Flatpak and its dependent packages ** //
    if (status == 0) {
        QString warningMessage = "Removing Flatpak will also uninstall:\n\n";
        warningMessage += dependencies.isEmpty() ? "All Flatpak Apps" : "All Flatpak Apps\n" + dependencies;
        warningMessage += "\n\nDo you want to continue?";

        QMessageBox::StandardButton reply = QMessageBox::question(
            parent, "Warning: Flatpak Removal", warningMessage, QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    QProcess *process = new QProcess(parent);
    QTimer *monitorTimer = new QTimer(parent);

    QProgressDialog *progress = new QProgressDialog(
        status == 0 ? "Removing Flatpak..." : "Installing Flatpak...", nullptr, 0, 100, parent);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    connect(process, &QProcess::readyReadStandardOutput, parent, [=]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents();
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    parent, [=]() mutable {
        if (process->exitCode() == 0){
            progress->setValue(100);

            QMessageBox::information(parent, "Flatpak",
                                     status == 0 ? "Flatpak Disabled Successfully!" :
                                     "Flatpak Enabled Successfully!");
            flatpakToggle->setChecked(status != 0);
            flatpakToggle->setText(flatpakToggle->isChecked() ? "Disable/Remove Flatpak"
                                                              : "Enable/Install Flatpak");
        } else {
            progress->setValue(100);
            QMessageBox::warning(parent, "Error", "Failed performing operations with Flatpak:\n"
                                 + process->readAllStandardError());
        }

        progress->deleteLater();
        process->deleteLater();
        monitorTimer->deleteLater();
    });

    QString enableCommand = "pacman -S --noconfirm flatpak && flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo";
    QString disableCommand = "flatpak uninstall --all && flatpak remotes | grep -q flathub && flatpak remote-delete flathub && sudo pacman -Rcns --noconfirm flatpak";
    QString command = (status == 0) ? disableCommand : enableCommand;

    process->start("pkexec", QStringList() << "bash" << "-c" << command);
    monitorTimer->start(250);
}










