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

void drive_list_widget::refresh() {
    // Block selection-change signals during refresh.
    m_ignoreSelectionChanges = true;
    m_treeWidget->clear();

    // Run lsblk with JSON output.
    QProcess process;
    process.start("lsblk", QStringList()
                               << "--json"
                               << "--output" << "NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT,LABEL");
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

        // Create the parent tree item.
        QTreeWidgetItem *parentItem = new QTreeWidgetItem(m_treeWidget);
        QString driveName = "/dev/" + deviceObj.value("name").toString();
        QString driveSize = deviceObj.value("size").toString();
        parentItem->setText(0, driveName);
        parentItem->setText(1, driveSize);
        QString parentMount = deviceObj.value("mountpoint").toString();
        parentItem->setData(0, Qt::UserRole, parentMount);

        // Check if this disk is partitioned.
        bool hasChildren = deviceObj.contains("children") && deviceObj.value("children").isArray();

        if (!hasChildren) {
            // For unpartitioned drives, add a parent checkbox.
            QWidget *parentActionWidget = new QWidget();
            QHBoxLayout *parentLayout = new QHBoxLayout(parentActionWidget);
            parentLayout->setContentsMargins(0, 0, 0, 0);
            QCheckBox *parentCheckBox = new QCheckBox("Mount Drive", parentActionWidget);

            if (!parentMount.isEmpty())
                parentCheckBox->setCheckState(Qt::Checked);
            else
                parentCheckBox->setCheckState(Qt::Unchecked);
            // Store default state.
            parentCheckBox->setProperty("defaultState", parentCheckBox->checkState());
            parentLayout->addWidget(parentCheckBox);
            parentActionWidget->setLayout(parentLayout);
            m_treeWidget->setItemWidget(parentItem, 2, parentActionWidget);

            // Emit selectionChanged only if not during refresh.
            connect(parentCheckBox, &QCheckBox::checkStateChanged, this, [this, parentCheckBox]() {
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

                // Determine whether to hide this partition.
                bool skip = false;
                if (fstype == "swap" && !m_showSwap)
                    skip = true;
                if (!m_showBoot) {
                    bool appearsBootlike = false;
                    if (partMount == "/boot" || partMount == "/boot/efi")
                        appearsBootlike = true;
                    if (label.contains("boot") || label.contains("efi") ||
                        label.contains("esp"))
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
                childItem->setData(0, Qt::UserRole, partMount);

                // Create widget with a checkbox for the partition.
                QWidget *childActionWidget = new QWidget();
                QHBoxLayout *childLayout = new QHBoxLayout(childActionWidget);
                childLayout->setContentsMargins(0, 0, 0, 0);
                QCheckBox *childCheckBox = new QCheckBox("Mount Drive", childActionWidget);
                if (!partMount.isEmpty())
                    childCheckBox->setCheckState(Qt::Checked);
                else
                    childCheckBox->setCheckState(Qt::Unchecked);
                // Save the default state.
                childCheckBox->setProperty("defaultState", childCheckBox->checkState());

                // Mark dangerous if this partition is swap or boot-like.
                bool isHarmful = (fstype == "swap" ||
                                  partMount == "/boot" || partMount == "/boot/efi" ||
                                  label.contains("boot") || label.contains("efi") || label.contains("esp"));
                childCheckBox->setProperty("isDangerous", isHarmful);

                childLayout->addWidget(childCheckBox);
                childActionWidget->setLayout(childLayout);
                m_treeWidget->setItemWidget(childItem, 2, childActionWidget);

                connect(childCheckBox, &QCheckBox::checkStateChanged, this,
                        [this, childCheckBox](int /*state*/) {
                            if (childCheckBox->property("isDangerous").toBool()) {
                                QMessageBox::StandardButton reply =
                                    QMessageBox::warning(this, "Warning",
                                                         "Changing the state of this partition can be harmful to your system.\n"
                                                         "Do you really want to continue?",
                                                         QMessageBox::Yes | QMessageBox::No);

                                if (reply == QMessageBox::No) {
                                    childCheckBox->blockSignals(true);
                                    int defState = childCheckBox->property("defaultState").toInt();
                                    childCheckBox->setCheckState(static_cast<Qt::CheckState>
                                                                 (defState));
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

bool drive_list_widget::applyMountSelection(const QString &username) {
    // Build lists of devices to enable (checked) and disable (unchecked).
    QStringList enabledDevices;
    QStringList disabledDevices;

    for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *parentItem = m_treeWidget->topLevelItem(i);
        QWidget *parentWidget = m_treeWidget->itemWidget(parentItem, 2);
        QCheckBox *parentCheckBox = parentWidget ? parentWidget->findChild<QCheckBox*>(): nullptr;

        if(parentCheckBox) {
            if (parentItem->childCount() == 0) {
                // No partitions: use parent's checkbox state.
                QString dev = parentItem->text(0);
                if (parentCheckBox->isChecked())
                    enabledDevices.append(dev);
                else
                    disabledDevices.append(dev);
            } else {
                // For drives with partitions, process each partition.
                for (int j = 0; j < parentItem->childCount(); j++) {
                    QTreeWidgetItem *childItem = parentItem->child(j);
                    QWidget *childWidget = m_treeWidget->itemWidget(childItem, 2);
                    QCheckBox *childCheckBox = childWidget ? childWidget->findChild<QCheckBox*>() : nullptr;

                    if (childCheckBox) {
                        QString dev = childItem->text(0);
                        if (childCheckBox->isChecked())
                            enabledDevices.append(dev);
                        else
                            disabledDevices.append(dev);
                    }
                }
            }
        }
    }

    // Write the configuration files.
    // (Note: these files are written as root; here we assume the ada external helper handles that.)
    QString enabledPath = "/etc/ada/tolitica/automount/enabled.conf";
    QString disabledPath = "/etc/ada/tolitica/automount/disabled.conf";
    bool success = true;

    QFile fileEnabled(enabledPath);
    if (fileEnabled.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&fileEnabled);
        for (const QString &line : enabledDevices)
            out << line << "\n";
        fileEnabled.close();
    } else {
        success = false;
    }
    QFile fileDisabled(disabledPath);
    if (fileDisabled.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&fileDisabled);
        for (const QString &line : disabledDevices)
            out << line << "\n";
        fileDisabled.close();
    } else {
        success = false;
    }

    // If file writes succedded, launch the external helper via pkexec
    if (success) {
        QProcess proc;
        QString helperBinary = "/etc/ada/tolitica/automount/ada-automount-helper";
        QStringList args;
        args << username; // Passing the username, so the helper can use the correct media path.
        proc.start("pkexec", QStringList() << helperBinary << args);
        proc.waitForFinished();
        if (proc.exitCode() == 0)
            return true;
        else
            return false;
    }

    return success;
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







