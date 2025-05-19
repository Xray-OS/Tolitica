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

drive_list_widget::drive_list_widget(QWidget *parent)
    : QWidget{parent}
{
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
    // Clear the olf items from the tree widget.
    m_treeWidget->clear();

    // Execute lsblk with JSON output
    QProcess process;
    process.start("lsblk", QStringList() << "--json" << "--output" << "NAME,SIZE,TYPE,MOUNTPOINT");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();

    // Parse the JSON output.
    QJsonDocument jsonDoc = QJsonDocument::fromJson(output);
    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray devicesArray = jsonObj.value("blockdevices").toArray();

    // Iterate over each top-level device (using std::as_const to avoid detach warnings).
    for (const auto &value : std::as_const(devicesArray)) {
        QJsonObject deviceObj = value.toObject();
        QString type = deviceObj.value("type").toString();

        // Process only disks as top-level items.
        if (type != "disk")
            continue;

        // Create a top-level item for the disk.
        QTreeWidgetItem *parentItem = new QTreeWidgetItem(m_treeWidget);
        QString driveName = "/dev/" + deviceObj.value("name").toString();
        QString driveSize = deviceObj.value("size").toString();
        parentItem->setText(0, driveName);
        parentItem->setText(1, driveSize);
        parentItem->setData(0, Qt::UserRole, deviceObj.value("mountpoint").toString());

        // --- Create the parent's "Action" widget with a QCheckBox ---
        QWidget *parentActionWidget = new QWidget();
        QHBoxLayout *parentLayout = new QHBoxLayout(parentActionWidget);
        parentLayout->setContentsMargins(0, 0, 0, 0);
        QCheckBox *parentCheckBox = new QCheckBox("Mount Drive", parentActionWidget);
        parentLayout->addWidget(parentCheckBox);
        parentActionWidget->setLayout(parentLayout);
        m_treeWidget->setItemWidget(parentItem, 2, parentActionWidget);

        // Check if the disk object contains a "children" array (i.e. partitions).
        if (deviceObj.contains("children") && deviceObj.value("children").isArray()) {
            QJsonArray partitionsArray = deviceObj.value("children").toArray();

            // Process each partition
            for (const auto &childVal : std::as_const(partitionsArray)) {
                QJsonObject partObj = childVal.toObject();
                // Confirm that the type is "part"
                if (partObj.value("type").toString() != "part")
                    continue;

                // Create a child item under the disk parent.
                QTreeWidgetItem *childItem = new QTreeWidgetItem(parentItem);
                QString partitionName = "/dev/" + partObj.value("name").toString();
                QString partitionSize = partObj.value("size").toString();
                childItem->setText(0, partitionName);
                childItem->setText(1, partitionSize);
                childItem->setData(0, Qt::UserRole, partObj.value("mountpoint").toString());

                // --- Create the partition's "Action" widget with a QCheckBox, ---
                QWidget *childActionWidget = new QWidget();
                QHBoxLayout *childLayout = new QHBoxLayout(childActionWidget);
                childLayout->setContentsMargins(0, 0, 0, 0);
                QCheckBox *childCheckBox = new QCheckBox("Mount Drive", childActionWidget);
                childLayout->addWidget(childCheckBox);
                childActionWidget->setLayout(childLayout);
                m_treeWidget->setItemWidget(childItem, 2, childActionWidget);

                // Connect the child's checkbox signal.
                // When a partition checkbox is unchecked, force the parent's checkbox to uncheck
                connect(childCheckBox, &QCheckBox::checkStateChanged, this, [this, parentItem, parentCheckBox]
                (int state) {
                    if (state == Qt::Unchecked) {
                        parentCheckBox->blockSignals(true);
                        parentCheckBox->setCheckState(Qt::Unchecked);
                        parentCheckBox->blockSignals(false);
                    } else {
                        // If the child is being checked, check if all sibling partitions are now checked.
                        bool allChecked = true;
                        int childCount = parentItem->childCount();
                        for (int i = 0; i < childCount; ++i) {
                            QWidget *w = m_treeWidget->itemWidget(parentItem->child(i), 2);
                            if (w) {
                                QCheckBox *cb = w->findChild<QCheckBox*>();
                                if (!cb || cb->checkState() != Qt::Checked) {
                                    allChecked = false;
                                    break;
                                }
                            }
                        }
                        if (allChecked) {
                            parentCheckBox->blockSignals(true);
                            parentCheckBox->setCheckState(Qt::Checked);
                            parentCheckBox->blockSignals(false);
                        }
                    }
                });
            } // end for each partition

            // Connect the parent's checkbox so that toggling it sets all child partitions accordingly.
            connect(parentCheckBox, &QCheckBox::checkStateChanged, this, [this, parentItem](int state) {
                int childCount = parentItem->childCount();
                for(int i = 0; i < childCount; ++i) {
                    QWidget *childWidget = m_treeWidget->itemWidget(parentItem->child(i), 2);
                    if (childWidget) {
                        QCheckBox *childCB = childWidget->findChild<QCheckBox*>();
                        if (childCB) {
                            childCB->blockSignals(true);
                            childCB->setCheckState(static_cast<Qt::CheckState>(state));
                            childCB->blockSignals(false);
                        }
                    }
                }
            });
        }
        // if a disk not have any partitions (no "children" array),
        // the parent's checkbox will remain independent
        // end for each disk
    }
}









