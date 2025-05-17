#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QStackedWidget>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include "core_functions.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    // TWEAKS
    void cleanOrphans(); // Function to clean orphans and other useful stuff to fix Arch
    void cleanPkgCache();
    void systemUpdate();
    void removeDBLock();
    void tweaksSetupConnections(QStackedWidget *stackedWidget, QPushButton *tweaksButton, QPushButton *backButton,
                                QPushButton *cleanOrphansButton, QPushButton *cleanPkgCacheButton,
                                QPushButton *updateSystemButton, QPushButton *removeDBLockButton, QCheckBox *bluetoothToggle,
                                QCheckBox *appArmorToggle, QPushButton *rankMirrorsButton);
    // ADDONS
    void adaGamingMeta();
    void removeAdaGamingMeta();
    void adaDevelopmentMeta();
    void removeAdaDevelopmentMeta();
    void chaoticAUR();
    void removeChaoticAUR();
    int checkChaoticAURStatus();
    void backupPacmanConfig();
    void addVMware(QPushButton *vmwButton);
    int vmwareStatus();
    bool vmwareServiceStatus();
    void removeVMware(QPushButton *vmwButton);
    void addonsSetupConnections(QStackedWidget *stackedWidget, QPushButton *addonsButton, QPushButton *addonsBackButton,
                                QPushButton *adaGamingMetaButton, QPushButton *adaDevelopmentButton, QPushButton *chaoticAURButton,
                                QPushButton *vmwButton, QCheckBox *flatpakToggle);
    // TERMINAL
    void terminalSetupConnections(QStackedWidget *stackedWidget, QPushButton *terminalButton, QPushButton *terminalBackButton,
                                  QPushButton *terminalThemeButton, QPushButton *changeShellButton, QComboBox *shellComboBox, QLabel *shellLabel);
    void disableTermTheme(QPushButton *terminalThemeButton);
    int checkTermThemingStatus();

private:
    Ui::Widget *ui;
    CoreFunctions* coreFunctions;
};
#endif // WIDGET_H
