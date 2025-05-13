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
    QString username = qgetenv("USER");  // ✅ Get actual username

    qDebug() << "Attempting to change shell to:" << selectedShell;

    QProcess process;
    process.start("pkexec", QStringList() << "chsh" << "-s" << selectedShell << username); // ✅ Use `pkexec` for permissions
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QMessageBox::information(parent, "Shell Change", "Shell changed successfully to: " + selectedShell);
    } else {
        QMessageBox::warning(parent, "Shell Change", "Failed to change shell. Error:\n" + process.readAllStandardError());
    }
}

// void CoreFunctions::changeShell(QWidget *parent, const QString &selectedShell) {
//     if (selectedShell.isEmpty()) {
//         QMessageBox::warning(parent, "Shell Change", "No shell selected.");
//         return;
//     }

//     QString username = qgetenv("USER");  // ✅ Get actual username
//     qDebug() << "Attempting to change shell for user:" << username << " to:" << selectedShell;

//     // Prompt user for password
//     bool ok;
//     QString password = QInputDialog::getText(parent, "Authentication Required",
//                                              "Enter your password:", QLineEdit::Password, "", &ok);

//     if (!ok || password.isEmpty()) {
//         QMessageBox::warning(parent, "Shell Change", "Password is required to proceed.");
//         return;
//     }

//     // Start process with password input
//     QProcess process;
//     process.start("sudo", QStringList() << "-S" << "chsh" << "-s" << selectedShell << username);  // ✅ Use actual username
//     process.write(password.toUtf8() + "\n");  // ✅ Send password to sudo
//     process.waitForFinished();

//     QString errorOutput = process.readAllStandardError();
//     qDebug() << "chsh command output:" << errorOutput;

//     if (process.exitCode() == 0) {
//         QMessageBox::information(parent, "Shell Change", "Shell changed successfully to: " + selectedShell);
//     } else {
//         QMessageBox::warning(parent, "Shell Change", "Failed to change shell. Error:\n" + errorOutput);
//     }
// }













