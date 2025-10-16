#ifndef WIDGET_INITIAL_H
#define WIDGET_INITIAL_H

#include "core_functions.h"
#include "core_initial.h"
#include "widget.h"
#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QStackedWidget>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QBoxLayout>
#include <QCloseEvent>

class Widget_Initial : public QWidget
{
    Q_OBJECT

public:
    Widget_Initial(QWidget *parent = nullptr);
    ~Widget_Initial();
    void markSetupComplete();

protected:
    void closeEvent(QCloseEvent *event) override;

// private slots:
//     void basicSetupConnections(QStackedWidget *stackedWidget,
//         QPushButton *flatpakToggleButton);

private:

CoreFunctions* coreFunctions;
Widget* widget;
CoreInitial* coreInitial;

};
#endif
