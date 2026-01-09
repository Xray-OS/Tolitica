#ifndef WIDGET_INITIAL_H
#define WIDGET_INITIAL_H

#include "core_functions.h"
#include "core_initial.h"
#include "widget.h"
#include "connectivityChecker.h"
#include <QWidget>
#include <QIcon>
#include <QPushButton>
#include <QStackedWidget>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QBoxLayout>
#include <QCloseEvent>
#include <functional>

class Widget_Initial : public QWidget
{
    Q_OBJECT

public:
    Widget_Initial(QWidget *parent = nullptr);
    ~Widget_Initial();
    void markSetupComplete();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void checkConnectivityAndExecute(std::function<void()> callback, const QString &errorMessage = "Internet connection required");

    CoreFunctions* coreFunctions;
    Widget* widget;
    CoreInitial* coreInitial;
    ConnectivityChecker* connectivityChecker;
};
#endif
