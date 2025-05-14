#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include <QStringList>
#include <QWidget>
#include <QCheckBox>

class CoreFunctions {
public:

    // TERMINAL
    static QStringList getInstalledShells();
    static QString getCurrentShell();
    static void changeShell(QWidget *parent, const QString &selectedShell);

    //TWEAKS
    static void enableBluetooth(QWidget *parent, QCheckBox *bluetoothToggle);
    static int bluetoothStatus();
};

#endif // CORE_FUNCTIONS_H
