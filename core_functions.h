#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include <QStringList>
#include <QWidget>
#include <QCheckBox>
#include <QObject>

class CoreFunctions : public QObject {
public:
    // Constructor that takes an optional parent pointer
    explicit CoreFunctions(QObject *parent = nullptr) : QObject(parent) {}

    // TERMINAL
    static QStringList getInstalledShells();
    static QString getCurrentShell();
    static void changeShell(QWidget *parent, const QString &selectedShell);

    //TWEAKS
    static int bluetoothStatus();
    static void enableBluetooth(QWidget *parent, QCheckBox *bluetoothToggle);
    static int apparmorStatus();
    static void enableAppArmor(QWidget *parent, QCheckBox *apparmorToggle);
    int getMirrorCount(QWidget *parent, int defaultValue = 10, int minValue = 1, int maxValue = 100);
    void rankMirrors(QWidget *parent, int mirrorCount);

    // ADDONS
    static int flatpakStatus();
    static void enableFlatpak(QWidget *parent, QCheckBox *flatpakToggle);
    int snapdStatus();
    void enableSnapd(QWidget *parent, QCheckBox *snapdToggle);

    // SOCIAL MEDIA
    void socialMedia(const QString &platform);

    // MOUNT/UNMOUNT DRIVES
    // QWidget* listAvailableDrives();
};

#endif // CORE_FUNCTIONS_H
