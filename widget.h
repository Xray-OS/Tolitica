#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QStackedWidget>

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
                                QPushButton *updateSystemButton, QPushButton *removeDBLockButton);
    // ADDONS
    void adaGamingMeta();
    void removeAdaGamingMeta();
    void adaDevelopmentMeta();
    void removeAdaDevelopmentMeta();
    void chaoticAUR();
    void removeChaoticAUR();
    int checkChaoticAURStatus();
    void backupPacmanConfig();
    void addonsSetupConnections(QStackedWidget *stackedWidget, QPushButton *addonsButton, QPushButton *addonsBackButton,
                                QPushButton *adaGamingMetaButton, QPushButton *adaDevelopmentButton, QPushButton *chaoticAURButton);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
