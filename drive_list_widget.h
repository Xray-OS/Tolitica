#ifndef DRIVE_LIST_WIDGET_H
#define DRIVE_LIST_WIDGET_H

#include <QWidget>
#include <QTreeWidget>

class drive_list_widget : public QWidget
{
    Q_OBJECT
public:
    explicit drive_list_widget(QWidget *parent = nullptr);

    // Public method to refresh the drive list.
    void refresh();

private:
    QTreeWidget *m_treeWidget;

signals:
};

#endif // DRIVE_LIST_WIDGET_H
