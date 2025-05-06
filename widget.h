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
    void cleanOrphans(); // Function to clean orphans and other useful stuff to fix Arch
    void cleanPkgCache();
    void systemUpdate();
    void removeDBLock();
    void setupConnections(QStackedWidget *QStackedWidget, QPushButton *tweaksButton, QPushButton *addonsButton,
                          QPushButton *backButton, QPushButton *cleanOrphansButton, QPushButton *cleanPkgCacheButton,
                          QPushButton *updateSystemButton, QPushButton *removeDBLockButton);

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
