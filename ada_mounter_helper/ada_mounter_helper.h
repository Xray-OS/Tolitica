#ifndef ADA_MOUNTER_HELPER_H
#define ADA_MOUNTER_HELPER_H

#include <QString>
#include <QSet>

class AdaMounterHelper
{
public:
    // Public accessor for the configuration file path.
    static const QString& configPath() { return manuallyEnabledConfPath; }

    // Reads the list of partitions that the user explicitly wants mounted
    // from manually_enabled.conf.
    static QSet<QString> getManuallyEnabledPartitions();

    // Retrieves a set of currently mounted partitions device names.
    static QSet<QString> getMountedPartitions();

    // Enforces the user's desired state:
    // - Mounts partitions listed in manually_enabled.conf.
    // - Unmounts partitions not listed (but which are currently mounted)
    static void enforceMounts();
    static void enforceUnmounts();

private:
    static QString manuallyEnabledConfPath;
    static QString diskByUUIDPath;
};

#endif // ADA_MOUNTER_HELPER_H
