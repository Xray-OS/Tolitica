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
#include <QFile>
#include <qdir.h>
#include <qprocess.h>
#include <QToolTip>
#include <qtoolbutton.h>
#include <QDesktopServices>
//#include <QtNetwork/QNetworkConfigurationManager>

// CUSTOM
#include "connectivityChecker.h"

///////////////////////////////////////////////////////////////////////////////////////////////
// ==== INSTALLATION BUTTONS =====
///////////////////////////////////////////////////////////////////////////////////////////////
/*##*/
// //////////////////////////////////////
// // ==== ONLINE INSTALL BUTTON =====
// /////////////////////////////////////
// void calamares_page::onlineInstallation() {
//     // Create a ConnectivityChecker instance.
//     ConnectivityChecker *checker = new ConnectivityChecker(this);

//     connect(checker, &ConnectivityChecker::connectivityChecked,
//             this, [this, checker](bool isConnected) {
//                 checker->deleteLater();

//                 if(!isConnected) {
//                     QMessageBox::information(this, tr("Internet Connection"),
//                                              tr("It appears you are not connected to the internet. "
//                                                 "Please connect to the internet before proceeding with the online installation"));
//                     return;
//                 } else {
//                     QString onlinePath = "/etc/calamares/settings-advanced.conf";
//                     QString sysUpdFileOnline = "/etc/calamares/modules/packages-system-update.conf";

//                     QFile settings_adv_file(onlinePath);
//                     QFile settings_sys_file(sysUpdFileOnline);

//                     QProcess proc;
//                     QString copyCommand = QString("cp -r %1 /etc/calamares/settings.conf && cp -r %2 /etc/calamares/packages.conf").arg(settings_adv_file.fileName(), settings_sys_file.fileName());
//                     proc.start("pkexec", QStringList() << "bash" << "-c" << copyCommand);
//                     proc.waitForFinished();

//                     QProcess *fireup = new QProcess(this);
//                     fireup->setProcessChannelMode(QProcess::ForwardedChannels);
//                     fireup->startDetached("bash", QStringList() << "-c" << "nice -n 10 sudo -S calamares");

//                     connect(fireup, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [fireup](int exitCode, QProcess::ExitStatus) {
//                         qDebug() << "Calamares finished with exit code:" << exitCode;
//                         fireup->deleteLater();
//                     });
//                 }
//             });

//     checker->checkConnectivity();

// }

//////////////////////////////////////
// ==== OFFLINE INSTALL BUTTON =====
/////////////////////////////////////
void calamares_page::offlineInstallation() {
    // QString offlinePath = "/etc/calamares/settings-beginner.conf";
    // QString sysFileOffline = "/etc/calamres/modules/packages-no-system-update.conf";

    // QFile settings_beg_file(offlinePath);
    // QFile settings_sys_file(sysFileOffline);

    // QProcess proc;
    // QString copyCommand = QString("cp -r %1  /etc/calamares/settings.conf && cp -r %2 /etc/calamares/packages.conf").arg(settings_beg_file.fileName(), settings_sys_file.fileName());
    // proc.start("pkexec", QStringList() << "bash" << "-c" << copyCommand);
    // proc.waitForFinished();

    QProcess *fireup = new QProcess(this);
    fireup->setProcessChannelMode(QProcess::ForwardedChannels);
    fireup->startDetached("bash", QStringList() << "-c" << "nice -n 10 sudo -S arch7z-installer");

    connect(fireup, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [fireup](int exitCode, QProcess::ExitStatus) {
        qDebug() << "Arch7z Installer finished with exit code:" << exitCode;
        fireup->deleteLater();
    });
}

//////////////////////////////////////
// ==== GPARTED BUTTON =====
/////////////////////////////////////
void calamares_page::gparted() {
    QProcess proc;
    proc.start("bash", QStringList() << "gparted");
    proc.waitForFinished();
}

//////////////////////////////////////
// ==== PARTITION-MANAGER BUTTON =====
/////////////////////////////////////
void calamares_page::partitionManager() {
    QProcess proc;
    proc.start("bash", QStringList() << "-c" << "partitionmanager");
    proc.waitForFinished();
}

///////////////////////////////////////////////////
/// SOCIAL MEDIA: SOCIAL MEDIA BUTTONS
//////////////////////////////////////////////////
void calamares_page::socialMedia(const QString &platform) {
    QString url;

    if (platform == "discord") {
        url = "https://discord.gg/dBR7wR3ABk";
    } else if (platform == "twitter") {
        url = "https://x.com/xray_os";
    } else if (platform == "youtube") {
        url = "https://www.youtube.com/@Xray_OS";
    }

    if (!url.isEmpty()) {
        QDesktopServices::openUrl(QUrl(url));
    }
}

// == I LOVE CPP ==================================
///////////////////////////////////////////////////
/// START MAIN FUNCTION
//////////////////////////////////////////////////
// == I LOVE C++ ==================================

calamares_page::calamares_page(QWidget *parent)
    : QWidget(parent)
{
    // ---------- Main Layout for the Whole Page ----------
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // ---------- Content Container (Max 800px) ----------
    QWidget *contentContainer = new QWidget(this);
    contentContainer->setMaximumWidth(800);
    contentContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setSpacing(10);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // ---------- Header Section ----------
    QLabel *headerLabel = new QLabel("<h1>Welcome to Tolitica Xray_OS Assistant!</h1>", contentContainer);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setContentsMargins(0, 20, 0, 0);
    contentLayout->addWidget(headerLabel);

    // ---------- Greetings Section ----------
    QLabel *greetingsLabel = new QLabel(
        "<html><head><style>"
        " .greeting { font-size: 11pt; font-weight: bold; }"
        " .role { font-size: 10pt; font-weight: normal; }"
        "</style></head><body>"
        "<span class='greeting'>Greetings from Angel!</span> - "
        "<span class='role'>Owner & Maintainer of Xray_OS.</span>"
        "</body></html>", contentContainer);
    greetingsLabel->setAlignment(Qt::AlignCenter);
    greetingsLabel->setContentsMargins(0, 10, 0, 0);
    contentLayout->addWidget(greetingsLabel);

    // ---------- Description Section ----------
    QWidget *descContainer = new QWidget(contentContainer);
    descContainer->setMaximumWidth(800);
    QVBoxLayout *descLayout = new QVBoxLayout(descContainer);
    descLayout->setContentsMargins(0, 10, 0, 0);
    QLabel *descriptionLabel = new QLabel(
        "<html><head><style>"
        "p { font-size: 10pt; text-align: justify }"
        "</style></head><body>"
        "<p>With this helper application you can install Xray_OS on your system. "
        "Click on the button below to start the installation.</p>"
        "</body></html>", descContainer);
    descriptionLabel->setWordWrap(true);
    descLayout->addWidget(descriptionLabel);
    QHBoxLayout *centerDescLayout = new QHBoxLayout;
    centerDescLayout->addWidget(descContainer);
    contentLayout->addLayout(centerDescLayout);

    // ---------- Icon Section ----------
    QLabel *iconLabel = new QLabel(contentContainer);
    QPixmap toliticaIcon(":/icons/resources/icons/tolitica-icon.png");
    QPixmap scaledIcon = toliticaIcon.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    iconLabel->setPixmap(scaledIcon);
    contentLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));
    contentLayout->addWidget(iconLabel, 1, Qt::AlignCenter | Qt::AlignTop);

    // ---------- Buttons Section ----------
    QWidget *buttonsContainer = new QWidget(contentContainer);
    buttonsContainer->setMaximumWidth(800);
    QGridLayout *gridLayout = new QGridLayout(buttonsContainer);
    gridLayout->setContentsMargins(20, 20, 20, 20);
    gridLayout->setSpacing(10);

    // --- Row 0: Installation Buttons Section ---
    // QPushButton *onlineInstallationButton = new QPushButton("Online Installation", contentContainer);
    QPushButton *offlineInstallationButton = new QPushButton("Start Installation", contentContainer);

    // Set expanding horizontal size policies so they fill available width.
    // onlineInstallationButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    //onlineInstallationButton->setFixedSize(200, 40);
    offlineInstallationButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    //offlineInstallationButton->setFixedSize(200, 40);

    QWidget *installButtonsWidget = new QWidget(buttonsContainer);
    QHBoxLayout *installLayout = new QHBoxLayout(installButtonsWidget);
    installLayout->setContentsMargins(0, 0, 0, 0);
    installLayout->setSpacing(10);
    // installLayout->addWidget(onlineInstallationButton);
    installLayout->addWidget(offlineInstallationButton);
    installButtonsWidget->setLayout(installLayout);
    gridLayout->addWidget(installButtonsWidget, 0, 0);

    // --- Row 1: Partition Buttons Section ---
    QPushButton *gpartedButton = new QPushButton("GParted", contentContainer);
    QPushButton *partitionManagerButton = new QPushButton("Partition Manager", contentContainer);
    QWidget *partitionButtonsWidget = new QWidget(buttonsContainer);
    QVBoxLayout *partLayout = new QVBoxLayout(partitionButtonsWidget);
    partLayout->setContentsMargins(0, 100, 0, 0);
    partLayout->setSpacing(5);
    partLayout->addWidget(gpartedButton);
    partLayout->addWidget(partitionManagerButton);
    partitionButtonsWidget->setLayout(partLayout);
    gridLayout->addWidget(partitionButtonsWidget, 1, 0, Qt::AlignLeft | Qt::AlignTop);

    // --- Row 1: Social Media Buttons ---
    QWidget *socialMediaButtonsWidget = new QWidget(buttonsContainer);
    QHBoxLayout *socialMediaLayout = new QHBoxLayout(socialMediaButtonsWidget);

    QToolButton *discordButton = new QToolButton(contentContainer);
    discordButton->setIcon(QIcon(":/icons/resources/icons/discord.png"));
    discordButton->setIconSize(QSize(38, 38));
    discordButton->setAutoRaise(true);
    discordButton->setToolTip("Join my Discord!");

    // *== Twitter
    QToolButton *twitterButton = new QToolButton(contentContainer);
    twitterButton->setIcon(QIcon(":/icons/resources/icons/twitter.png"));
    twitterButton->setIconSize(QSize(48, 48));
    twitterButton->setAutoRaise(true);
    twitterButton->setToolTip("Follow me on Twitter");

    // *== YouTube
    QToolButton *youtubeButton = new QToolButton(contentContainer);
    youtubeButton->setIcon(QIcon(":/icons/resources/icons/youtube.png"));
    youtubeButton->setIconSize(QSize(48, 48));
    youtubeButton->setAutoRaise(true);
    youtubeButton->setToolTip("Subscribe to my Channel");

    socialMediaLayout->setContentsMargins(0, 100, 0, 0);
    socialMediaLayout->setSpacing(5);
    socialMediaLayout->addWidget(discordButton);
    socialMediaLayout->addWidget(twitterButton);
    socialMediaLayout->addWidget(youtubeButton);
    socialMediaButtonsWidget->setLayout(socialMediaLayout);
    gridLayout->addWidget(socialMediaButtonsWidget, 1, 0, Qt::AlignCenter);

    // === Connect signals to the social media function === //
    connect(discordButton, &QToolButton::clicked, this, [=](){
        socialMedia("discord");
    });
    connect(twitterButton, &QToolButton::clicked, this, [=](){
        socialMedia("twitter");
    });
    connect(youtubeButton, &QToolButton::clicked, this, [=](){
        socialMedia("youtube");
    });

    // Adjust vertical distribution:
    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 2);
    contentLayout->addWidget(buttonsContainer);

    // ---------- Center the Content Container in the Window ----------
    QHBoxLayout *wrapperLayout = new QHBoxLayout;
    wrapperLayout->addWidget(contentContainer);
    mainLayout->addLayout(wrapperLayout);

    setLayout(mainLayout);

    // ---------- Signal-Slot Connections ----------
    installOptionsSetupConnections(nullptr, offlineInstallationButton,
                                   gpartedButton, partitionManagerButton);
}

// == I LOVE CPP ==================================
///////////////////////////////////////////////////
/// END MAIN FUNCTION
//////////////////////////////////////////////////
// == I LOVE C++ ==================================

///////////////////////////////////////////////////
/// INSTALL OPTIONS CONNECTIONS FUNCTION
//////////////////////////////////////////////////

void calamares_page::installOptionsSetupConnections(QPushButton *onlineInstallationButton, QPushButton *offlineInstallationButton, QPushButton *gpartedButton,
                                                    QPushButton *partitionManagerButton) {
    // Only connect if the button exists
    if (onlineInstallationButton) {
        // connect(onlineInstallationButton, &QPushButton::clicked, this, &::calamares_page::onlineInstallation);
    }
    connect(offlineInstallationButton, &QPushButton::clicked, this, &::calamares_page::offlineInstallation);

    // * == GPARTED/PARTITIONMANAGER == * //
    connect(gpartedButton, &QPushButton::clicked, this, &::calamares_page::gparted);
    connect(partitionManagerButton, &QPushButton::clicked, this, &::calamares_page::partitionManager);
}
