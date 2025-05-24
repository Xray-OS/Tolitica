#include "calamares_page.h"
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QPixmap>
#include <QMessageBox>
#include <QDebug>
//#include <QtNetwork/QNetworkConfigurationManager>

// CUSTOM
#include "connectivityChecker.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// ==== INSTALLATION BUTTONS =====
///////////////////////////////////////////////////////////////////////////////////////////////
/*##*/
//////////////////////////////////////
// ==== ONLINE INSTALL BUTTON =====
/////////////////////////////////////
void calamares_page::onlineInstallation() {
    // Create a ConnectivityChecker instance.
    ConnectivityChecker *checker = new ConnectivityChecker(this);

    connect(checker, &ConnectivityChecker::connectivityChecked,
            this, [this, checker](bool isConnected) {
        checker->deleteLater();

        if(!isConnected) {
            QMessageBox::information(this, tr("Internet Connection"),
                        tr("It appears you are not connected to the internet. "
                        "Please connect to the internet before proceeding with the online installation"));
            return;
        }
    });

    // Starting asynchronous connectivity check.
    checker->checkConnectivity();
}

calamares_page::calamares_page(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    // ==== HEADER SECTION ====
    QLabel *headerLabel = new QLabel("<h1>Welcome to Tolitica Ada Assistant!</h1>", this);
    headerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(headerLabel);
    headerLabel->setContentsMargins(0, 20, 0, 0);

    // ==== GREETINGS SECTION ====
    QLabel *greetingsLabel = new QLabel(
        "<html><head><style>"
        " .greeting { font-size: 11pt; font-weight: bold; }"
        " .role { font-size: 10pt; font-weight: normal; }"
        "</style></head><body>"
        "<span class='greeting'>Greetings from Angel!</span> - "
        "<span class='role'>Owner & Maintainer of Ada</span>"
        "</body></html>", this);



    greetingsLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(greetingsLabel);
    greetingsLabel->setContentsMargins(0, 10, 0, 0);

    QWidget *descContainer = new QWidget(this);
    descContainer->setMaximumWidth(800);  // Maximum width, but it can shrink on smaller screens

    QVBoxLayout *descLayout = new QVBoxLayout(descContainer);
    descLayout->setContentsMargins(0, 10, 0, 0);

    QLabel *descriptionLabel = new QLabel(
        "<html><head><style>"
        "p { font-size: 10pt; text-align: justify; font-weight: bold }"
        "</style></head><body>"
        "</p>"
        "With this helper application you can install Ada on your system. "
        "Click on the buttons below to choose an offline or online installation."
        "</p>"
        "</body></html>", descContainer);

    descriptionLabel->setWordWrap(true);

    descLayout->addWidget(descriptionLabel);

    QHBoxLayout *centerDescLayout = new QHBoxLayout;
    centerDescLayout->addWidget(descContainer);
    mainLayout->addLayout(centerDescLayout);

    QLabel *iconLabel = new QLabel(this);
    QPixmap toliticaIcon(":/icons/resources/icons/tolitica-icon.png");
    QPixmap scaledIcon = toliticaIcon.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(scaledIcon);

    mainLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
    mainLayout->addWidget(iconLabel, 1, Qt::AlignCenter | Qt::AlignTop);

    setLayout(mainLayout);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // ==== INSTALLATION PAGE =====
    ///////////////////////////////////////////////////////////////////////////////////////////////
    QPushButton *onlineInstallationButton = new QPushButton("Online Installation", this);
    QPushButton *offlineInstallationButton = new QPushButton("Offline Installation", this);

    QHBoxLayout *installButtonsLayout = new QHBoxLayout;
    installButtonsLayout->addWidget(onlineInstallationButton);
    installButtonsLayout->addWidget(offlineInstallationButton);
    //installButtonsLayout->setAlignment(Qt::AlignCenter);

    QWidget *installButtonsWidget = new QWidget;
    installButtonsWidget->setLayout(installButtonsLayout);
    installButtonsWidget->setMaximumWidth(800);
    // installButtonsWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *installButtonsWrapper = new QHBoxLayout;
    installButtonsWrapper->addWidget(installButtonsWidget);

    mainLayout->addLayout(installButtonsWrapper, Qt::AlignTop);

    installOptionsSetupConnections(onlineInstallationButton, offlineInstallationButton);
}
///////////////////////////////////////////////////
/// INSTALL OPTIONS CONNECTIONS FUNCTION
//////////////////////////////////////////////////

void calamares_page::installOptionsSetupConnections(QPushButton *onlineInstallationButton, QPushButton *offlineInstallationButton) {
    connect(onlineInstallationButton, &QPushButton::clicked, this, &::calamares_page::onlineInstallation);

}
