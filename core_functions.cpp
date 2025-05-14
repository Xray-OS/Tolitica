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








