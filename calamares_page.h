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
    void offlineInstallation();
    void gparted();
    void partitionManager();
    void installOptionsSetupConnections(QPushButton *onlineInstallationButton,
                                        QPushButton *offlineInstallationButton,
                                        QPushButton *gpartedButton,
                                        QPushButton *partitionManagerButton);
    void socialMedia(const QString &platform);
};

#endif // CALAMARES_PAGE_H
