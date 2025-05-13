#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include <QStringList>
#include <QWidget>

class CoreFunctions {
public:

    // TERMINAL
    static QStringList getInstalledShells();
    static void changeShell(QWidget *parent, const QString &selectedShell);
};

#endif // CORE_FUNCTIONS_H
