#ifndef CALAMARES_PAGE_H
#define CALAMARES_PAGE_H

#include <QWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QObject>

class calamares_page : public QWidget
{
    Q_OBJECT

public:
    explicit calamares_page(QWidget *parent = nullptr);

    void onlineInstallation();
    void installOptionsSetupConnections(QPushButton *onlineInstallationButton,
                                        QPushButton *offlineInstallationButton);
};

#endif // CALAMARES_PAGE_H
