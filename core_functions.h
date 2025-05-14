#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include <QStringList>
#include <QWidget>
#include <QCheckBox>

class CoreFunctions : public QObject {
public:

    // TERMINAL
    static QStringList getInstalledShells();
    static QString getCurrentShell();
    static void changeShell(QWidget *parent, const QString &selectedShell);

    //TWEAKS
    static int bluetoothStatus();
    static void enableBluetooth(QWidget *parent, QCheckBox *bluetoothToggle);

    // ADDONS
    static int flatpakStatus();
    static void enableFlatpak(QWidget *parent, QCheckBox *flatpakToggle);
};

#endif // CORE_FUNCTIONS_H
