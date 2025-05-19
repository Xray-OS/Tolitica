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
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QWidget>

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

///////////////////////////////////////////////////
/// TWEAKS: CHECK APP-ARMOR STATUS
/////////////////////////////////////////////////
int CoreFunctions::apparmorStatus() {
    QProcess process;

    QStringList tests = {
        "lsmod | grep apparmor",
        "zgrep \"CONFIG_SECURITY_APPARMOR=y\" /proc/config.gz",
        "cat /sys/module/apparmor/parameters/enabled"
    };

    bool supportsApparmor = true;

    for (const QString &test : tests) {
        process.start("bash", QStringList() << "-c" << test);
        process.waitForFinished();
        QString result = process.readAllStandardOutput().trimmed();


        bool valid = test.contains("cat") ? (result == "Y") : !result.isEmpty();

        if(!valid) {
            supportsApparmor = false;
            break;
        }

    }
    if (!supportsApparmor) {
        return 0;
    }

    process.start("bash", QStringList() << "-c" << "pacman -Q apparmor");
    process.waitForFinished();
    bool pkgInstalled = (process.exitCode() == 0);

    process.start("bash", QStringList() << "-c" << "systemctl is-enabled apparmor.service");
    process.waitForFinished();
    bool isEnabled = (process.exitCode() == 0);

    // **Check if AppArmor modules exist in GRUB (Handle both Quote Types!)**
    QFile grubFile("/etc/default/grub");
    bool grubSet = false;

    if(grubFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString grubContent = QTextStream(&grubFile).readAll();
        grubFile.close();

        static const QRegularExpression grubRegex(R"(GRUB_CMDLINE_LINUX_DEFAULT=(['\"])(.*?)\1)");
        QRegularExpressionMatch match = grubRegex.match(grubContent);

        if (match.hasMatch()) {
            QString grubParams = match.captured(2); // Extract GRUB_CMDLINE_LINUX_DEFAULT contents
            grubSet = grubParams.contains("lsm=landlock lockdown yama integrity apparmor bpf");
        }
    }

    // **Determine final status**
    if (pkgInstalled && isEnabled && grubSet) {
        return 1;
    } else if (!pkgInstalled && !isEnabled && !grubSet) {
        return 2;
    }
    return 3;
}

///////////////////////////////////////////////////
/// TWEAKS::ENABLE/DISABLE APPARMOR
/////////////////////////////////////////////////

void CoreFunctions::enableAppArmor(QWidget *parent, QCheckBox *apparmorToggle) {
   int status = apparmorStatus();
    if(status == 0) {
       apparmorToggle->blockSignals(true);
        apparmorToggle->setChecked(false);
       apparmorToggle->blockSignals(false);
        //apparmorToggle->setText("Enable AppArmor");
       QMessageBox::information(parent, "AppArmor not Supported", "Please Install a Kernel that supports AppArmor");
        return;
    }

    if (status == 1) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            parent,
            "Warning: Disable AppArmor",
            "Disabling AppArmor will remove security enforcement and system integrity protections.\n\nContinue?",
            QMessageBox::Yes | QMessageBox::No
            );
        if (reply == QMessageBox::No) {
            return;
        }
    }

    // Prepare asynchronous progress reporting (like in your Flatpak function)
    QProcess *process = new QProcess(parent);
    QTimer *monitorTimer = new QTimer(parent);
    QProgressDialog *progress = new QProgressDialog(
        (status == 1) ? "Disabling AppArmor..." : "Enabling AppArmor...",
        nullptr, 0, 100, parent
        );
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    connect(process, &QProcess::readyReadStandardOutput, parent, [progressValue, progress]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents();
    });

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            parent, [=]() mutable {
                progress->setValue(100);
                if (process->exitCode() == 0) {

                    bool newState = (status != 1);
                    QString msg = (status == 1) ?
                                      "AppArmor Disabled Successfully!" :
                                      "AppArmor Enabled Successfully!";
                    QMessageBox::information(parent, "AppArmor", msg);

                    // Update the checkbox according to the new state:
                    apparmorToggle->setChecked(newState);
                    apparmorToggle->setText(newState ? "Disable AppArmor" : "Enable AppArmor");
                } else {
                    QMessageBox::warning(parent, "Error",
                                         "Failed performing operations with AppArmor:\n" + process->readAllStandardError());
                }
                progress->deleteLater();
                process->deleteLater();
                monitorTimer->deleteLater();
            });

    // Our AppArmor parameter (note the leading space!)
    const QString param = " lsm=landlock lockdown yama integrity apparmor bpf";

    QString command;
    if (status == 1) {
        // Disabling AppArmor:
        command = QString(
                      "sed -Ei \"s/%1//g\" /etc/default/grub; "          // Remove duplicates from GRUB
                      "grub-mkconfig -o /boot/grub/grub.cfg; "             // Regenerate GRUB config
                      "systemctl disable apparmor.service; "             // Disable AppArmor service
                      "systemctl stop apparmor.service"                  // Stop AppArmor service
                      ).arg(param);
    } else {
        // Enabling AppArmor:
        command = QString(
                      "pacman -Q apparmor || pacman -S --noconfirm apparmor; " // Install if not present
                      "systemctl enable apparmor.service; "                   // Enable service
                      "systemctl start apparmor.service; "                    // Start service
                      "sed -Ei \"s/%1//g\" /etc/default/grub; "                // Remove duplicate parameters
                      // For double-quoted GRUB line:
                      "sed -Ei 's/^(GRUB_CMDLINE_LINUX_DEFAULT=\")(.*)\"/\\1\\2%1\"/' /etc/default/grub; "
                      // For single-quoted GRUB line:
                      "sed -Ei \"s/^(GRUB_CMDLINE_LINUX_DEFAULT=')([^']*)'/\\1\\2%1'/\" /etc/default/grub; "
                      "grub-mkconfig -o /boot/grub/grub.cfg"                  // Regenerate GRUB config
                      ).arg(param);
    }

    process->start("pkexec", QStringList() << "bash" << "-c" << command);
    monitorTimer->start(250);
}

///////////////////////////////////////////////////
/// TWEAKS: GET MIRROR COUNT
//////////////////////////////////////////////////
int CoreFunctions::getMirrorCount(QWidget *parent, int defaultValue, int minValue, int maxValue) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Rank Mirrors"));

    QFormLayout form(&dialog);

    // Create a label and a spinBox on the same row.
    QLabel *label = new QLabel(
        QObject::tr("It ranks all current mirrors by default. You can also set this from %1 up to %2").arg(minValue).arg(maxValue),
        &dialog);
    QSpinBox *spinBox = new QSpinBox(&dialog);
    spinBox->setRange(minValue, maxValue);
    spinBox->setValue(defaultValue);
    form.addRow(label, spinBox);

    // Add standard OK/Cancel buttons to a QDialogButtonBox.
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);

    // Create an extra button for "Rank All Mirrors"
    QPushButton *rankAllButton = new QPushButton(QObject::tr("Rank All Mirrors"), &dialog);
    buttonBox.addButton(rankAllButton, QDialogButtonBox::ActionRole);

    form.addRow(&buttonBox);

    // Standard connections for Ok and Cancel.
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Extra behaviour for Rank All Mirror button.
    QObject::connect(rankAllButton, &QPushButton::clicked, &dialog, [&dialog]() {
        dialog.setProperty("mirrorCountResult", 0);
        dialog.accept();
    });

    int result = defaultValue;

    // Execute the dialog.
    if (dialog.exec() == QDialog::Accepted) {
        QVariant prop = dialog.property("mirrorCountResult");
        if (prop.isValid() && prop.canConvert<int>() && prop.toInt() == 0)
            result = 0;
        else
            result = spinBox->value();
    } else {
        return -1;
    }

    return result;
}

///////////////////////////////////////////////////
/// TWEAKS: RANK MIRRORS
//////////////////////////////////////////////////
void CoreFunctions::rankMirrors(QWidget *parent, int mirrorCount) {
    // Creating a progress dialog with range 0-100
    QProgressDialog *progressDialog = new QProgressDialog(
        "Performing backup and ranking mirrors...", "Cancel", 0, 100, parent);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setAutoClose(true);
    progressDialog->setAutoReset(true);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);

    // Create and start the backup process asynchronously.
    QProcess *backupProcess = new QProcess(parent);
    connect(backupProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=]
    (int exitCode, QProcess::ExitStatus /*status*/) {
        if (progressDialog->wasCanceled()) {
            backupProcess->kill();
            backupProcess->deleteLater();
            progressDialog->deleteLater();
            return;
        }

        if (exitCode != 0) {
            QMessageBox::warning(parent, "Error", "Backup process encountered an error");
            backupProcess->deleteLater();
            progressDialog->cancel();
            progressDialog->deleteLater();
            return;
        }
        progressDialog->setValue(50);

        // Check if Reflector is installed by running "which reflector".
        QProcess whichProcess;
        whichProcess.start("which", QStringList() << "reflector");
        whichProcess.waitForFinished();
        bool hasReflector = (whichProcess.exitCode() == 0 &&
                             !QString(whichProcess.readAllStandardOutput()).trimmed().isEmpty());

        QString command;
        if (hasReflector) {
            // If mirrorCount is non-zero, run reflector with --latest, otherwise rank all mirrors
            if (mirrorCount != 0) {
                command = QString(
                "exec sudo reflector --latest %1 --sort rate --save /etc/pacman.d/mirrorlist")
                .arg(mirrorCount);
            } else {
                command = QString(
                "exec sudo reflector --sort rate --save /etc/pacman.d/mirrorlist");
            }
        } else {
            // Similarly, if using rankmirrors: include -n when mirrorCount is non-zero
            if (mirrorCount != 0) {
                command = QString("exec rankmirrors -n %1 /etc/pacman.d/mirrorlist").arg(mirrorCount);
            } else {
                command = QString("exec rankmirrors /etc/pacman.d/mirrorlist");
            }
        }

        //Create and start the ranking process asynchronously
        QProcess *rankProcess = new QProcess(parent);
        connect(rankProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus /*status*/) {
            if (progressDialog->wasCanceled()) {
                rankProcess->kill();
                rankProcess->deleteLater();
                progressDialog->deleteLater();
                return;
            }

            // Update progress to completed
            progressDialog->setValue(100);

            if (exitCode == 0) {
                if (mirrorCount != 0) {
                    QMessageBox::information(parent, "Mirrors Ranked",
                    QString("The mirrors have been ranked by the %1 fastest ones").arg(mirrorCount));
                }
                QMessageBox::information(parent, "Mirrors Ranked",
                                         QString("All the mirrors have been ranked to the fastest ones").arg(mirrorCount));
            } else {
                QMessageBox::warning(parent, "Error",
                "Something went wrong ranking the mirrors\n" +
                rankProcess->readAllStandardError());
            }
            rankProcess->deleteLater();
            progressDialog->deleteLater();
        });
        // Start the ranking process via pkexec in a bash shell
        rankProcess->start("pkexec", QStringList() << "bash" << "-c" << command);
        backupProcess->deleteLater();
    });

    // Connect the progress dialog's cancellation to the backup process.
    connect(progressDialog, &QProgressDialog::canceled, this, [=]() {
        if (backupProcess->state() != QProcess::NotRunning)
            backupProcess->kill();
    });

    // Start the backup process asynchronously via pkexec in a bash shell.
    backupProcess->start("pkexec", QStringList() << "bash" << "-c"
                        << "cp -r /etc/pacman.d/mirrorlist /etc/pacman.d/mirrorlist.backup");
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

///////////////////////////////////////////////////
/// ADDONS: SNAPD-STATUS
//////////////////////////////////////////////////
int CoreFunctions::snapdStatus() {
    QProcess process;
    process.start("bash", QStringList() << "-c" << "pacman -Q snapd");
    process.waitForFinished();
    bool pkgInstalled = (process.exitCode() == 0);

    process.start("bash", QStringList() << "-c" << "systemctl is-enabled snapd.socket");
    process.waitForFinished();
    bool isEnabled = (process.exitCode() == 0);

    process.start("bash", QStringList() << "-c" << "readlink /snap");
    process.waitForFinished();
    bool linkExist = (QString(process.readAllStandardOutput()).trimmed() == "/var/lib/snapd/snap");

    if (pkgInstalled && isEnabled && linkExist) {
        return 0;
    } else if (!pkgInstalled && !isEnabled && !linkExist) {
        return 1;
    } else {
        return 2;
    }
}

///////////////////////////////////////////////////
/// ADDONS: ENABLE/DISABLE SNAPD
//////////////////////////////////////////////////
void CoreFunctions::enableSnapd(QWidget *parent, QCheckBox *snapdToggle) {
    int status = snapdStatus();

    if (status == 0) {
        QString confirmationMsg = "Are you sure you want to disable/remove snapd support?";

        QMessageBox::StandardButton reply = QMessageBox::question(
            parent, "Confirmation: ", confirmationMsg, QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    QProcess *process = new QProcess(parent);
    QTimer *monitorTimer = new QTimer(parent);

    QProgressDialog *progress = new QProgressDialog(
        status == 0 ? "Removing Snapd..." : "Installing Snapd...", nullptr, 0, 100, parent);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->show();

    int progressValue = 0;

    // This connection updates the progress value when new standard output is available.
    connect(process, &QProcess::readyReadStandardOutput, parent, [=]() mutable {
        progressValue += 5;
        progress->setValue(qMin(progressValue, 95));
        QCoreApplication::processEvents();
    });

    // Using monitorTimer to simulate progress updates if process output is insufficient
    connect(monitorTimer, &QTimer::timeout, parent, [=]() mutable {
        if (progressValue < 95) {
            progressValue += 2;
            progress->setValue(qMin(progressValue, 95));
        }
    });
    monitorTimer->start(250);

    // Handling success/failure and clean up resources
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), parent, [=]()
        mutable {
        // Stop timer, no further updates needed
        monitorTimer->stop();

        if (process->exitCode() == 0){
            progress->setValue(100);

            QMessageBox::information(parent, "Snapd",
                                     status == 0 ? "Snapd Disabled Successfully!" :
                                         "Snapd Enabled Successfully!");
            snapdToggle->setChecked(status != 0);
            snapdToggle->setText(snapdToggle->isChecked() ?
                "Disable/Remove SNAPD" : "Enable/Install SNAPD");
        } else {
            QMessageBox::warning(parent, "Snapd", "An error occurred while processing");
        }

        progress->deleteLater();
        process->deleteLater();
        monitorTimer->deleteLater();
    });

    QString enableCommand = "pacman -S --noconfirm snapd && systemctl enable --now snapd.socket && "
                            "ln -s /var/lib/snapd/snap /snap";
    QString disableCommand = "pacman -Rns snapd && rm -rf /snap";
    QString command = (status == 0) ? disableCommand : enableCommand;

    if (status == 0) {
        if (QFile::exists("/var/lib/snapd")) {
            QString uninstallSnapdPkgs = "Would you also like to remove all SNAPD packages?";
            QMessageBox::StandardButton reply = QMessageBox::question(
                parent, "confirmation: ", uninstallSnapdPkgs, QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                disableCommand = "pacman -Rns --noconfirm snapd && rm -rf /snap && rm -rf /var/lib/snapd";
            }
        }
        command = disableCommand;
    } else {
        command = enableCommand;
    }

    process->start("pkexec", QStringList() << "bash" << "-c" << command);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS FOR THE SOCIAL MEDIA BUTTONS /////////////////////// /////////////////////// ///////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// SOCIAL MEDIA: SOCIAL MEDIA BUTTONS
//////////////////////////////////////////////////
void CoreFunctions::socialMedia(const QString &platform) {
    QString url;

    if (platform == "discord") {
        url = "https://discord.gg/dBR7wR3ABk";
    } else if (platform == "twitter") {
        url ="https://twitter.com/Ada_Linux";
    } else if (platform == "youtube") {
        url = "https://www.youtube.com/@Xray_OS-Linux";
    } else if (platform == "patreon") {
        url = "patreon.com/Ada_Linux";
    }

    if (!url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS TO MOUNT DRIVES              /////////////////////// /////////////////////// ///////
//////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////
/// FUNCTION TO LIST THE AVAILABLE DRIVES
//////////////////////////////////////////////////

// QWidget* CoreFunctions::listAvailableDrives() {
//     // Create a container widget and its layout
//     QWidget *driveWidget = new QWidget();
//     QVBoxLayout *layout = new QVBoxLayout(driveWidget);

//     // Create a tree widget to serve as our details-view list.
//     QTreeWidget *treeWidget = new QTreeWidget(driveWidget);
//     treeWidget->setColumnCount(3);
//     treeWidget->setHeaderLabels(QStringList() << "Device" << "Size" << "Action");

//     // Use QProcess to execute "lsblk" with JSON output.
//     QProcess process;
//     // We ask the for the following columns: NAME, SIZE, TYPE, MOUNTPOINT
//     process.start("lsblk", QStringList() << "--json" << "--output" << "NAME,SIZE,TYPE,MOUNTPOINT");
//     process.waitForFinished();
//     QByteArray output = process.readAllStandardOutput();

//     // Parse the JSON output.
//     QJsonDocument jsonDoc = QJsonDocument::fromJson(output);
//     QJsonObject jsonObj = jsonDoc.object();

//     // "blockdevices" array contains the drive information.
//     QJsonArray devicesArray = jsonObj.value("blockdevices").toArray();
//     for (const auto &value : std::as_const(devicesArray)) {
//         QJsonObject deviceObj = value.toObject();
//         QString type = deviceObj.value("type").toString();

//         // Filter: display only disks and partitions
//         if (type != "disk" && type != "part")
//             continue;

//         QString name = deviceObj.value("name").toString();
//         QString size = deviceObj.value("size").toString();
//         QString mountpoint = deviceObj.value("mountpoint").toString(); // May be empty if unmounted

//         // Create a new row in the tree widget for this drive.
//         QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
//         QString deviceName = "/dev/" + name;
//         item->setText(0, deviceName);
//         item->setText(1, size);

//         // --- Create a widget for the "Action" column ---
//         // This widget holds a checkbox with the label "Mount Drive".
//         QWidget *actionWidget = new QWidget();
//         QHBoxLayout *hlayout = new QHBoxLayout(actionWidget);
//         hlayout->setContentsMargins(0, 0, 0, 0);
//         QCheckBox *mountCheckBox = new QCheckBox("Mount Drive", actionWidget);
//         hlayout->addWidget(mountCheckBox);
//         actionWidget->setLayout(hlayout);

//         // Add the action widget to the third column of the current row.
//         treeWidget->setItemWidget(item, 2, actionWidget);

//         // Future note: is possible to store additional information (like mountpoint)
//         // into the item's data for use later (e.g., when handling the checkbox signal).
//         item->setData(0, Qt::UserRole, mountpoint);
//     }

//     // Add the tree widget to our layout.
//     layout->addWidget(treeWidget);
//     driveWidget->setLayout(layout);

//     // Return the widget containing the details view
//     return driveWidget;
// }

























