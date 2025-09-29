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
#include "drive_list_widget.h"
#include <QToolButton>
#include <QBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT
    friend class Widget_Initial;

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void cleanCache();

private slots:
    // TERMINAL
    // void autostartStatus();
    ////////////////////////////////////////////////////////
    /// GENERAL: DISABLE/ENABLE AUTOSTART
    //////////////////////////////////////////////////////
    bool autostart();

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
    void archZGamingMeta();
    void removeArchZGamingMeta();
    void arch7zDevelopmentMeta();
    void removeArch7zDevelopmentMeta();
    void chaoticAUR();
    void removeChaoticAUR();
    bool runCommand(const QString &cmd);
    int checkChaoticAURStatus();
    void backupPacmanConfig();
    void addVMware(QPushButton *vmwButton);
    bool vmwareStatus();
    bool vmwareServiceStatus();
    void addonsSetupConnections(QStackedWidget *stackedWidget, QPushButton *addonsButton, QPushButton *addonsBackButton,
                                QPushButton *archZGamingMetaButton, QPushButton *archZDevelopmentButton, QPushButton *chaoticAURButton,
                                QPushButton *vmwButton, QCheckBox *flatpakToggle, QCheckBox *snapdToggle);
    // TERMINAL
    void terminalSetupConnections(QStackedWidget *stackedWidget, QPushButton *terminalButton, QPushButton *terminalBackButton,
                                  QPushButton *terminalThemeButton, QPushButton *changeShellButton, QComboBox *shellComboBox, QLabel *shellLabel);
    void disableTermTheme(QPushButton *terminalThemeButton);
    int checkTermThemingStatus();
    int asciiLogoStatus();
    void enableAsciiLogo();
    // MOUNT/UNMOUNT DRIVES
    void mountDrivesSetupConnections(QStackedWidget *stackedWidget, QToolButton *mountDriveButton);

private:
    Ui::Widget *ui;

    CoreFunctions* coreFunctions;
    QWidget *mountDrivesPage = nullptr;
    drive_list_widget* drivesPage = nullptr;

    void setupMountDrivesButtons(QWidget *parent, QHBoxLayout *layout);
    void resetMountUnmountButtonState();
};
#endif // WIDGET_H
