#include "ada_mounter_helper.h"
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QSet>
#include <QDebug>

// Define the path to manually_enabled.conf
QString AdaMounterHelper::manuallyEnabledConfPath = "/etc/ada/tolitica/automount/manually_enabled.conf";

// Define the base path for device resolution by UUID
QString AdaMounterHelper::diskByUUIDPath = "/dev/disk/by-uuid/";

// ----------------------------------------------------------------------------------------------
// Reads the list of partitions that should be mounted from the config.
// Expected format: one partition device per line (e.g., "/dev/sda1").
// ----------------------------------------------------------------------------------------------
QSet<QString> AdaMounterHelper::getManuallyEnabledPartitions() {
    QSet<QString> enabledSet;
    QFile file(manuallyEnabledConfPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << manuallyEnabledConfPath;
        return enabledSet;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        // Remove BOM if it's present (UTF-8 BOM character U+FEFF)
        if (!line.isEmpty() && line.at(0) == QChar(0xFEFF)) {
            line.remove(0, 1);
        }

        if(line.isEmpty() || line.startsWith("#")) {
            continue; // Skip empty lines or comments
        }

        // Expect the line to be in the format "UUID=<uuid>"
        if (line.startsWith("UUID=")) {
            enabledSet.insert(line);
        } else {
            qWarning() << "Invalid configuration format, expected line to start with \"UUID=\": " << line;
        }
    }
    file.close();
    return enabledSet;
}
// ----------------------------------------------------------------------------------------------
// Uses lsblk to retrieve a set of currently mounted partitions based on UUID.
// We ask lsblk to output the "UUID", "TYPE", and "MOUNTPOINT" fields.
// For each partition (type "part") that has a non-empty mountpoint, we store it.
// - Use "--output" "UUID,TYPE,MOUNTPOINT"
// - Store mounted partitions as "UUID=<uuid>" for consistency with config.
// ----------------------------------------------------------------------------------------------
QSet<QString> AdaMounterHelper::getMountedPartitions() {
    QSet<QString> mountedSet;
    QProcess process;

    // Use the overload, provide QIODevice::ReadWrite as the third argument.
    process.start("mount", QStringList(), QIODevice::ReadWrite);
    process.waitForFinished();
    QString out = process.readAllStandardOutput();

    // Use the split variant with Qt::SkipEmptyParts
    QStringList lines = out.split("\n", Qt::SkipEmptyParts);

    for (const QString &line : qAsConst(lines)) {
        if (!line.contains("/mnt/"))
            continue;
        // Expected line format: <device> on <mountpoint> type <fstype> (options)
        QStringList tokens = line.split(" ", Qt::SkipEmptyParts);
        if (tokens.size() < 3)
            continue;
        QString mountPoint = tokens.at(2);
        if (mountPoint.startsWith("/mnt/")) {
            QString rawUUID = mountPoint.section('/', 2, 2);
            if (!rawUUID.isEmpty()) {
                mountedSet.insert("UUID=" + rawUUID);
            }
        }
    }
    return mountedSet;
}



// ----------------------------------------------------------------------------------------------
// Enforces the mounting of partitions using UUID.
// - For each UUID listed in manually_enabled.conf (e.g., "UUID=<uuid>"):
//     * If the partition is not mounted, create a mount directory and mount it.
// - The mount command now dynamically detects the filesystem type using blkid.
// ----------------------------------------------------------------------------------------------
void AdaMounterHelper::enforceMounts() {
    QSet<QString> enabled = getManuallyEnabledPartitions();
    QSet<QString> mounted = getMountedPartitions();

    qDebug() << "Enabled partitions:" << enabled;
    qDebug() << "Mounted partitions:" << mounted;

    // Iterate over enabled partitions: if not mounted, mount them.
    for (const QString &uuidLine : qAsConst(enabled)) {
        if (!mounted.contains(uuidLine)) {
            // Define a mount directory, e.g., /mnt/ followed by partition identifier.
            QString uuid = uuidLine.section('=', 1, 1);

            QString mountDir = "/mnt/" + uuid;
            QProcess mkdirProc, mountProc;

            qDebug() << "Mounting partition with UUID:" << uuidLine << "to" << mountDir;

            // Create the mount directory if it doesn't exist.
            mkdirProc.start("mkdir", QStringList() << "-p" << mountDir);
            mkdirProc.waitForFinished();

            // Resolve the actual device path from the UUID using the diskByUUID variable.
            // Build the symlink path (e.g., "/dev/disk/by-uuid/uuid/dca4b9d7-a0a4-485c-857f-e0b2f2da0206")
            // Use readlink to resolve the real device path from symlink
            QString uuidSymlink = diskByUUIDPath + uuid;
            QProcess readLinkProc;
            readLinkProc.start("readlink", QStringList() << "-f" << uuidSymlink);
            readLinkProc.waitForFinished();
            QString resolvedDevice = QString(readLinkProc.readAllStandardOutput()).trimmed();
            qDebug() << "Resolved device path:" << resolvedDevice;

            // Detect the filesystem type using blkid:
            QProcess blkidProc;
            blkidProc.start("blkid", QStringList() << "-o" << "value" << "-s" << "TYPE" << resolvedDevice);
            blkidProc.waitForFinished();
            QString fsType = QString(blkidProc.readAllStandardOutput()).trimmed();
            if (fsType.isEmpty())
                fsType = "auto"; // Fallback if detection fails.
            qDebug() << "Detected filesystem type:" << fsType;

            // Perform the mount using the detected filesystem type.
            // Note: "UUID=" + uuid produces the proper argument, e.g., "UUID=dca4b9d7"
            mountProc.start("mount", QStringList() << "-t" << fsType << "UUID=" + uuid << mountDir);
            mountProc.waitForFinished();

            // Debugging output
            qDebug() << "Mount command exit code:" << mountProc.exitCode();
            qDebug() << "Mount stdout:" << mountProc.readAllStandardOutput();
            qDebug() << "Mount stderr:" << mountProc.readAllStandardError();
        }
    }
}

void AdaMounterHelper::enforceUnmounts() {
    // Read the current configuration and mounted partitions.

    QSet<QString> enabled = getManuallyEnabledPartitions();
    QSet<QString> mounted = getMountedPartitions();

    qDebug() << "enforceUnmounts: Enabled partitions:" << enabled;
    qDebug() << "enforceUnmounts: Mounted partitions" << mounted;

    // For each mounted partition that is no longer enabled, unmount its mount point.
    for (const QString &part : qAsConst(mounted)) {
        if (!enabled.contains(part)) {
            // Extract the raw UUID from the token (which is stored as "UUID-<rawUUID>")
            QString rawUUID = part.section('=', 1, 1);
            // Construct the mount directory (which the mount command uses)
            QString mountDir = "/mnt/" + rawUUID;

            qDebug() << "Enforcing unmount for:" << mountDir;

            QProcess umountProc;
            umountProc.start("umount", QStringList() << mountDir);
            umountProc.waitForFinished();
            int exitCode = umountProc.exitCode();

            if (exitCode != 0) {
                qWarning() << "Failed to unmount" << mountDir << "with exit code:" << exitCode;
            } else {
                qDebug() << "Successfully unmounted" << mountDir;
            }
        }
    }
}

























