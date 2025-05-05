#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPushButton>

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

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
