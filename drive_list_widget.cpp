#include "drive_list_widget.h"
#include <QVBoxLayout>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <utility> // for std::asconst
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>

drive_list_widget::drive_list_widget(QWidget *parent)
    : QWidget{parent}
{
    // Use a vertical layout that holds the QTreeWidget.
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setColumnCount(3);
    m_treeWidget->setHeaderLabels(QStringList() << "Device" << "Size" << "Action");
    layout->addWidget(m_treeWidget);
    setLayout(layout);

    // Populate initially
    refresh();
}

QSet<QString> drive_list_widget::loadManuallyEnabledDevices() const {
    QSet<QString> enabledSet;
    QString configPath = "/etc/ada/tolitica/automount/manually_enabled.conf";
    QFile file(configPath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("#"))
                continue;
            enabledSet.insert(line);
        }
        file.close();
    }
    return enabledSet;
}

void drive_list_widget::refresh() {
    // Load the current enabled device tokens from the configuration file.
    QSet<QString> enabledDevices = loadManuallyEnabledDevices();

    // Block selection-change signals during refresh.
    m_ignoreSelectionChanges = true;
    m_treeWidget->clear();

    // Run lsblk with JSON output including the UUID.
    QProcess process;
    process.start("lsblk", QStringList() << "--json"
                                         << "--output" << "NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT,LABEL,UUID");
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(output);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray devicesArray = jsonObj.value("blockdevices").toArray();

    // Iterate over each disk.
    for (const auto &value : std::as_const(devicesArray)) {
        QJsonObject deviceObj = value.toObject();
        QString type = deviceObj.value("type").toString();
        if (type != "disk")
            continue;

        // Extract the disk's UUID token.
        QString diskUUID = deviceObj.value("uuid").toString().trimmed();
        QString diskToken = diskUUID.isEmpty() ? "" : "UUID=" + diskUUID;

        // Create the parent tree item.
        QTreeWidgetItem *parentItem = new QTreeWidgetItem(m_treeWidget);
        QString driveName = "/dev/" + deviceObj.value("name").toString();
        QString driveSize = deviceObj.value("size").toString();
        parentItem->setText(0, driveName);
        parentItem->setText(1, driveSize);
        // Store the UUID token in the item's UserRole.
        parentItem->setData(0, Qt::UserRole, diskToken);

        // Check if this disk is partitioned.
        bool hasChildren = deviceObj.contains("children")
                           && deviceObj.value("children").isArray();

        if (!hasChildren) {
            // For unpartitioned drives, add a parent checkbox.
            QWidget *parentActionWidget = new QWidget();
            QHBoxLayout *parentLayout = new QHBoxLayout(parentActionWidget);
            parentLayout->setContentsMargins(0, 0, 0, 0);
            QCheckBox *parentCheckBox = new QCheckBox("Mount Drive", parentActionWidget);

            // Set checkbox state based on whether diskToken is in the config.
            if (!diskToken.isEmpty() && enabledDevices.contains(diskToken))
                parentCheckBox->setCheckState(Qt::Checked);
            else
                parentCheckBox->setCheckState(Qt::Unchecked);

            // Store default state.
            parentCheckBox->setProperty("defaultState", parentCheckBox->checkState());
            parentLayout->addWidget(parentCheckBox);
            parentActionWidget->setLayout(parentLayout);
            m_treeWidget->setItemWidget(parentItem, 2, parentActionWidget);

            // 🔵 CONNECT USING THE CORRECT SIGNAL 'stateChanged' WITH EXPLICIT CAPTURE AND PARAMETER
            connect(parentCheckBox, &QCheckBox::stateChanged, this,
                    [this, parentCheckBox](int state) {
                        if (!m_ignoreSelectionChanges)
                            emit selectionChanged();
                    });
        } else {
            // For partitioned drives, leave the parent's widget empty.
            QWidget *emptyWidget = new QWidget();
            m_treeWidget->setItemWidget(parentItem, 2, emptyWidget);
        }

        // Process children (partitions) if present.
        if (hasChildren) {
            QJsonArray partitionsArray = deviceObj.value("children").toArray();
            for (const auto &childVal : std::as_const(partitionsArray)) {
                QJsonObject partObj = childVal.toObject();
                if (partObj.value("type").toString() != "part")
                    continue;

                // Retrieve partition properties.
                QString fstype = partObj.value("fstype").toString().toLower();
                QString partMount = partObj.value("mountpoint").toString();
                QString label = partObj.value("label").toString().toLower();
                QString sizeStr = partObj.value("size").toString().toLower();

                // Extract partition UUID token.
                QString partUUID = partObj.value("uuid").toString().trimmed();
                QString partToken = partUUID.isEmpty() ? "" : "UUID=" + partUUID;

                // Determine whether to hide this partition.
                bool skip = false;
                if (fstype == "swap" && !m_showSwap)
                    skip = true;
                if (!m_showBoot) {
                    bool appearsBootlike = false;
                    if (partMount == "/boot" || partMount == "/boot/efi")
                        appearsBootlike = true;
                    if (label.contains("boot") || label.contains("efi") || label.contains("esp"))
                        appearsBootlike = true;
                    bool isVfat = (fstype == "vfat" || fstype == "fat32");
                    if (isVfat) {
                        bool sizeSmall = false;
                        if (sizeStr.endsWith("g")) {
                            bool ok = false;
                            double sizeVal = sizeStr.left(sizeStr.size() - 1).toDouble(&ok);
                            if (ok && sizeVal < 3.0)
                                sizeSmall = true;
                        } else if (sizeStr.endsWith("m")) {
                            bool ok = false;
                            double sizeVal = sizeStr.left(sizeStr.size() - 1).toDouble(&ok);
                            if (ok && sizeVal < 300.0)
                                sizeSmall = true;
                        }
                        if (sizeSmall)
                            appearsBootlike = true;
                    }
                    if (appearsBootlike)
                        skip = true;
                }
                if (skip)
                    continue;

                // Create a child item for the partition.
                QTreeWidgetItem *childItem = new QTreeWidgetItem(parentItem);
                QString partitionName = "/dev/" + partObj.value("name").toString();
                QString partitionSize = partObj.value("size").toString();
                childItem->setText(0, partitionName);
                childItem->setText(1, partitionSize);
                // Store the partition's UUID token.
                childItem->setData(0, Qt::UserRole, partToken);

                // Create widget with a checkbox for the partition.
                QWidget *childActionWidget = new QWidget();
                QHBoxLayout *childLayout = new QHBoxLayout(childActionWidget);
                childLayout->setContentsMargins(0, 0, 0, 0);
                QCheckBox *childCheckBox = new QCheckBox("Mount Drive", childActionWidget);

                // Set checkbox state based on whether partToken is in the config.
                if (!partToken.isEmpty() && enabledDevices.contains(partToken))
                    childCheckBox->setCheckState(Qt::Checked);
                else
                    childCheckBox->setCheckState(Qt::Unchecked);

                // Save the default state.
                childCheckBox->setProperty("defaultState", childCheckBox->checkState());

                // Mark as dangerous if this partition is swap/boot-like.
                bool isHarmful = (fstype == "swap" ||
                                  partMount == "/boot" || partMount == "/boot/efi" ||
                                  label.contains("boot") || label.contains("efi") || label.contains("esp"));
                childCheckBox->setProperty("isDangerous", isHarmful);

                childLayout->addWidget(childCheckBox);
                childActionWidget->setLayout(childLayout);
                m_treeWidget->setItemWidget(childItem, 2, childActionWidget);

                // 🔵 CONNECT USING 'stateChanged(int)' FOR CHILD CHECKBOX WITH EXPLICIT CAPTURE
                connect(childCheckBox, &QCheckBox::stateChanged, this,
                        [this, childCheckBox](int state) {
                            if (childCheckBox->property("isDangerous").toBool()) {
                                QMessageBox::StandardButton reply =
                                    QMessageBox::warning(this, "Warning",
                                                         "Changing the state of this partition can be harmful to your system.\n"
                                                         "Do you really want to continue?",
                                                         QMessageBox::Yes | QMessageBox::No);

                                if (reply == QMessageBox::No) {
                                    childCheckBox->blockSignals(true);
                                    int defState = childCheckBox->property("defaultState").toInt();
                                    childCheckBox->setCheckState(static_cast<Qt::CheckState>(defState));
                                    childCheckBox->blockSignals(false);
                                    return;
                                }
                            }
                            if (!m_ignoreSelectionChanges)
                                emit selectionChanged();
                        });
            }
        }
    }
    // Re-enable selection signals once refresh is complete.
    m_ignoreSelectionChanges = false;
}


void drive_list_widget::showAdditionalPartitionsDialog() {
    QDialog dialog(this);
    dialog.setWindowTitle("Additional Partitions Options");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QCheckBox *swapCheckBox = new QCheckBox("Show Swap Partitions", &dialog);
    swapCheckBox->setChecked(m_showSwap);
    QCheckBox *bootCheckBox = new QCheckBox("Show Boot Partitions", &dialog);
    bootCheckBox->setChecked(m_showBoot);
    QCheckBox *hiddenCheckBox = new QCheckBox("Show Hidden Partitions", &dialog);
    hiddenCheckBox->setChecked(m_showHidden);

    layout->addWidget(swapCheckBox);
    layout->addWidget(bootCheckBox);
    layout->addWidget(hiddenCheckBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        m_showSwap = swapCheckBox->isChecked();
        m_showBoot = bootCheckBox->isChecked();
        m_showHidden = hiddenCheckBox->isChecked();
        refresh();
    }
}

// Helper: returns true if any checkbox (parent or child) differs from its default state.
bool drive_list_widget::isModified() const {
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
        QWidget *w = m_treeWidget->itemWidget(item, 2);
        if (w) {
            QCheckBox *cb = w->findChild<QCheckBox*>();
            if (cb) {
                if (cb->checkState() != cb->property("defaultState").toInt())
                    return true;
            }
        }
        // Check children.
        for (int j = 0; j < item->childCount(); j++) {
            QWidget *cw = m_treeWidget->itemWidget(item->child(j), 2);
            if (cw) {
                QCheckBox *cb = cw->findChild<QCheckBox*>();
                if (cb) {
                    if (cb->checkState() != cb->property("defaultState").toInt())
                        return true;
                }
            }
        }
    }
    return false;
}

bool drive_list_widget::isDangerousModified() const {
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = m_treeWidget->topLevelItem(i);
        QWidget *w = m_treeWidget->itemWidget(item, 2);
        if (w) {
            QCheckBox *cb = w->findChild<QCheckBox*>();
            if (cb && cb->property("isDangerous").toBool()) {
                if (cb->checkState() != cb->property("defaultState").toInt())
                    return true;
            }
        }
        for (int j = 0; j < item->childCount(); j++) {
            QWidget *cw = m_treeWidget->itemWidget(item->child(j), 2);
            if (cw) {
                QCheckBox *cb = cw->findChild<QCheckBox*>();
                if (cb && cb->property("isDangerous").toBool()) {
                    if (cb->checkState() != cb->property("defaultState").toInt())
                        return true;
                }
            }
        }
    }
    return false;
}

bool drive_list_widget::applyMountSelection() {
    QString configPath = "/etc/ada/tolitica/automount/manually_enabled.conf";

    // Reset cancellation flag at the beginning.
    m_operationCancelled = false;

    // Build a string containing the new configuration content.
    QStringList enabledTokens;
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *parentItem = m_treeWidget->topLevelItem(i);
        QWidget *parentWidget = m_treeWidget->itemWidget(parentItem, 2);
        QCheckBox *parentCheckBox = parentWidget ? parentWidget->findChild<QCheckBox*>() : nullptr;

        if (parentCheckBox && parentItem->childCount() == 0) {
            QString token = parentItem->data(0, Qt::UserRole).toString();
            if (parentCheckBox->isChecked() && !token.isEmpty())
                enabledTokens.append(token);
        } else if (parentItem->childCount() > 0) {
            for (int j = 0; j < parentItem->childCount(); j++) {
                QTreeWidgetItem *childItem = parentItem->child(j);
                QWidget *childWidget = m_treeWidget->itemWidget(childItem, 2);
                QCheckBox *childCheckBox = childWidget ? childWidget->findChild<QCheckBox*>() : nullptr;
                if (childCheckBox) {
                    QString token = childItem->data(0, Qt::UserRole).toString();
                    if (childCheckBox->isChecked() && !token.isEmpty())
                        enabledTokens.append(token);
                }
            }
        }
    }

    // Construct the shell command to write the data.
    QString command = "echo '# list of enabled automount partitions' | pkexec tee " + configPath;
    for (const QString &token : enabledTokens) {
        command += " && echo '" + token + "' | pkexec tee -a " + configPath;
    }
    command += " && pkexec touch " + configPath; // Force the system to update timestamp.

    // Run the command using QProcess.
    QProcess process;
    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished();

    // Retrieve and trim standard error output to check for cancellation.
    QString errorOutput = process.readAllStandardError().trimmed();

    // (Optional) Debug output:
    // qDebug() << "applyMountSelection errorOutput:" << errorOutput << "exitCode:" << process.exitCode();

    // Check for success.
    if (process.exitCode() != 0) {
        // If errorOutput is empty, unusually short, contains cancellation text, or the exit code is 126,
        // assume a cancel occurred—and do not warn.
        if (errorOutput.isEmpty() ||
            errorOutput.length() < 15 ||
            errorOutput.contains("cancel", Qt::CaseInsensitive) ||
            errorOutput.contains("canceled", Qt::CaseInsensitive) ||
            errorOutput.contains("Authentication canceled", Qt::CaseInsensitive) ||
            process.exitCode() == 126) {
            m_operationCancelled = true;
            return true; // Treat cancellation as non-error.
        }
        // Instead of issuing a warning, we silently return false.
        return false;
    }

    return true;
}































