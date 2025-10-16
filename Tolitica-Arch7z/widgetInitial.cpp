#include "widgetInitial.h"
#include "core_functions.h"
#include "core_initial.h"
#include "widget.h"
#include <QDir>
#include <QDebug>
#include <QProgressDialog>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include <QPropertyAnimation>
#include <QPoint>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <QImageReader>

// for links to work
#include <QDesktopServices>
#include <QUrl>

/// START OF MAIN FUNCTION
Widget_Initial::Widget_Initial(QWidget *parent)
    : QWidget(parent)
{
    // ui->setupUi(this); // Commented out - no UI file

    coreFunctions = new CoreFunctions(this);
    widget = new Widget(this);
    widget->hide();
    coreInitial = new CoreInitial();

    setWindowTitle("Tolitica Xray_OS Assistant");
    resize(800,600);
    setWindowIcon(QIcon(":/icons/resources/icons/tolitica-icon.png"));

    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(this);
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->setCurrentIndex(0);

    QWidget *introPage = new QWidget();
    QWidget *basicPage = new QWidget();
    QWidget *appearancePage = new QWidget();
    QWidget *AURhelpersPage = new QWidget();
    QWidget *appStoresPage = new QWidget();
    QWidget *gamingPage = new QWidget();
    QWidget *supportPage = new QWidget();

    stackedWidget->addWidget(introPage);
    stackedWidget->addWidget(basicPage);
    stackedWidget->addWidget(appearancePage);
    stackedWidget->addWidget(AURhelpersPage);
    stackedWidget->addWidget(appStoresPage);
    stackedWidget->addWidget(gamingPage);
    stackedWidget->addWidget(supportPage);

    mainWidgetLayout->addWidget(stackedWidget);

    // === IntroPage ===
    QVBoxLayout *introLayout = new QVBoxLayout(introPage);
    introLayout->setContentsMargins(0, 0, 0, 20);

    introLayout->addStretch(1);

    QLabel *xrayIconLabel = new QLabel(this);
    QPixmap xrayIconPixmap(":/icons/resources/icons/xray.svg"); // Using existing icon
    QPixmap xrayScaledIcon = xrayIconPixmap.scaled(100, 100, Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation);
    xrayIconLabel->setPixmap(xrayScaledIcon);
    introLayout->addWidget(xrayIconLabel, 0, Qt::AlignCenter);

    QLabel *introHeader = new QLabel("<h1>Welcome to Xray_OS</h1>", this);
    introHeader->setAlignment(Qt::AlignCenter);
    introLayout->addWidget(introHeader, 0, Qt::AlignCenter);

    QLabel *introDescription = new QLabel("This is the initial setup for your system "
                                          "\nplease select your preferred options", this);
    introDescription->setWordWrap(true);
    introDescription->setAlignment(Qt::AlignCenter);
    introLayout->addWidget(introDescription, 0, Qt::AlignCenter);
    introLayout->addStretch(1);

    //* == Navigation Buttons
    QPushButton *continueButton = new QPushButton("Continue", this);
    continueButton->setCursor(Qt::PointingHandCursor);
    continueButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #007ACC;"
        "    color: white;"
        "    border: none;"
        "    padding: 10px 20px;"
        "    border-radius: 5px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #004080;"
        "}"
    );

    introLayout->addWidget(continueButton, 0, Qt::AlignCenter);

    connect(continueButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(1);
    });

    // === BASIC PAGE === //
    QGridLayout *basicLayout = new QGridLayout(basicPage);
    basicLayout->setContentsMargins(20, 20, 20, 20);
    basicLayout->setSpacing(15);

    basicLayout->setRowStretch(0, 1);

    QLabel *basicHeader = new QLabel("Basic Setup", this);
    basicHeader->setStyleSheet("font-size: 18px; color: #888; margin-bottom: 30px;");
    basicHeader->setAlignment(Qt::AlignCenter);

    basicLayout->addWidget(basicHeader, 1, 0, Qt::AlignCenter);

    // Flatpak Container
    QWidget *flatpakContainer = new QWidget(this);
    QHBoxLayout *flatpakLayout = new QHBoxLayout(flatpakContainer);
    flatpakLayout->setSpacing(50);

    QLabel *flatpakIconLabel = new QLabel(this);
    QPixmap flatpakIcon(":/icons/resources/icons/flatpak.svg");
    QPixmap flatpakIconScaled = flatpakIcon.scaled(60, 60, Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation);
    flatpakIconLabel->setPixmap(flatpakIconScaled);

    QLabel *flatpakLabel = new QLabel("Enable/Disable Flatpak", this);
    //flatpakLabel->setStyleSheet("font-size: 14px; color: white;");

    // Toggle switch
    bool isEnabled = (CoreFunctions::flatpakStatus() == 0);

    QWidget *toggleSwitch = new QWidget(this);
    toggleSwitch->setFixedSize(60, 30);

    if (isEnabled) {
        toggleSwitch->setStyleSheet("QWidget { background-color: #4a9eff; border-radius: 15px; }");
    } else {
        toggleSwitch->setStyleSheet("QWidget { background-color: #666666; border-radius: 15px; }");
    }

    QPushButton *toggleButton = new QPushButton(toggleSwitch);
    toggleButton->setFixedSize(26, 26);
    if (isEnabled)
    {
        toggleButton->move(32, 2); // Start in enabled position
    } else {
        toggleButton->move(2, 2);
    }

    toggleButton->setStyleSheet("QPushButton { background-color: white; border-radius: 13px; border: none; }");

    flatpakLabel->setText(isEnabled ? "Disable/Remove Flatpak" : "Enable/Install Flatpak");

    QPropertyAnimation *toggleAnimation = new QPropertyAnimation(toggleButton, "pos");
    toggleAnimation->setDuration(500);
    toggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *flatpakToggleAnim = new QPropertyAnimation(toggleButton, "iconSize");
    flatpakToggleAnim->setDuration(200);
    flatpakToggleAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bunce effect on press
    connect(toggleButton, &QPushButton::pressed, [=]() {
      flatpakToggleAnim->setStartValue(QSize(60, 60));
      flatpakToggleAnim->setEndValue(QSize(70, 70));
      flatpakToggleAnim->start();
    });
    connect(toggleButton, &QPushButton::released, [=]() {
        flatpakToggleAnim->setStartValue(QSize(70, 70));
        flatpakToggleAnim->setEndValue(QSize(60, 60));
        flatpakToggleAnim->start();
    });

    // Toggle effect on click and Logic
    connect(toggleButton, &QPushButton::clicked, this, [=]() mutable {
        // Temporary QCheckBox that mimics the toggle state
        QCheckBox *tempCheckBox = new QCheckBox();
        tempCheckBox->setChecked(isEnabled);

        // Pass callback to handle UI updates when async operation completes
        CoreFunctions::enableFlatpak(this, tempCheckBox, [=](bool success) mutable {
            if (success) {
                isEnabled = tempCheckBox->isChecked();

                if (isEnabled) {
                    toggleAnimation->setEndValue(QPoint(32, 2));
                    toggleSwitch->setStyleSheet("QWidget { background-color: #4a9eff; border-radius: 15px; }");
                } else {
                    toggleAnimation->setEndValue(QPoint(2, 2));
                    toggleSwitch->setStyleSheet("QWidget { background-color: #666666; border-radius: 15px; }");
                }
                flatpakLabel->setText(isEnabled ? "Disable/Remove Flatpak" : "Enable/Install Flatpak");
                toggleAnimation->start();
            }

            // Now it's safe to delete
            tempCheckBox->deleteLater();
        });
    });

    flatpakLabel->setText(isEnabled ? "Disable/Remove Flatpak" : "Enable/Install Flatpak");

    flatpakContainer->setFixedWidth(500);

    flatpakLayout->addWidget(flatpakIconLabel);
    flatpakLayout->addWidget(flatpakLabel);
    flatpakLayout->addStretch();
    flatpakLayout->addWidget(toggleSwitch);

    basicLayout->addWidget(flatpakContainer, 2, 0, Qt::AlignCenter);

    // Snapd Container
    QWidget *snapdContainer = new QWidget(this);
    QHBoxLayout *snapdContainerLayout = new QHBoxLayout(snapdContainer);
    snapdContainerLayout->setSpacing(50);

    QLabel *snapdIconLabel = new QLabel(this);
    QPixmap snapdIcon(":/icons/resources/icons/snapcraft.svg");
    QPixmap snapdIconScaled = snapdIcon.scaled(60, 60, Qt::KeepAspectRatio,
                                                    Qt::SmoothTransformation);
    snapdIconLabel->setPixmap(snapdIconScaled);

    QLabel *snapdLabel = new QLabel("Enable/Disable Snapd", this);
    // snapdLabel->setStyleSheet("font-size: 14px; color: white");

    bool isSnapdEnabled = (coreFunctions->snapdStatus() == 0);

    QWidget *snapdToggleSwitch = new QWidget(this);
    snapdToggleSwitch->setFixedSize(60, 30);

    if (isSnapdEnabled) {
        snapdToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }");
    }
    else {
        snapdToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }");
    }

    QPushButton *snapdToggleButton = new QPushButton(snapdToggleSwitch);
    snapdToggleButton->setFixedSize(26, 26);
    if (isSnapdEnabled) {
        snapdToggleButton->move(32, 2);
    } else {
        snapdToggleButton->move(2, 2);
    }

    snapdToggleButton->setStyleSheet("QPushButton { background-color: white; border-radius: 13px; border: none; }");
    snapdLabel->setText(isSnapdEnabled ? "Disable/Remove Snapd" : "Enable/Install Snapd");

    QPropertyAnimation *snapdToggleAnim = new QPropertyAnimation(snapdToggleButton, "pos");
    snapdToggleAnim->setDuration(500);
    snapdToggleAnim->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *snapdIconAnim = new QPropertyAnimation(snapdToggleButton, "iconSize");
    snapdIconAnim->setDuration(200);
    snapdIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(snapdToggleButton, &QPushButton::pressed, [=]() {
        snapdIconAnim->setStartValue(QSize(60, 60));
        snapdIconAnim->setEndValue(QSize(70, 70));
        snapdIconAnim->start();
    });
    connect(snapdToggleButton, &QPushButton::released, [=]() {
        snapdIconAnim->setStartValue(QSize(70, 70));
        snapdIconAnim->setEndValue(QSize(60, 60));
        snapdIconAnim->start();
    });

    // Toggle effect on click and Logic
    connect(snapdToggleButton, &QPushButton::clicked, this, [=]() mutable {
        QCheckBox *snapdTempCheckBox = new QCheckBox();
        snapdTempCheckBox->setChecked(isSnapdEnabled);

        coreFunctions->enableSnapd(this, snapdTempCheckBox, [=](bool snapdSuccess) mutable {
            if (snapdSuccess) {
                isSnapdEnabled = snapdTempCheckBox->isChecked();

                if (isSnapdEnabled) {
                    snapdToggleAnim->setEndValue(QPoint(32, 2));
                    snapdToggleSwitch->setStyleSheet("QWidget { background-color: #4a9eff; border-radius: 15px; }");
                } else {
                    snapdToggleAnim->setEndValue(QPoint(2, 2));
                    snapdToggleSwitch->setStyleSheet("QWidget { background-color: #666666; border-radius: 15px; }");
                }
                snapdLabel->setText(isSnapdEnabled ? "Disable/Remove Snapd" : "Enable/Install Snapd");
                snapdToggleAnim->start();
            }
            snapdTempCheckBox->deleteLater();
        });
    });

    snapdLabel->setText(isSnapdEnabled ? "Disable/Remove Snapd" : "Enable/Install Snapd");
    snapdContainer->setFixedWidth(500);

    snapdContainerLayout->addWidget(snapdIconLabel);
    snapdContainerLayout->addWidget(snapdLabel);
    snapdContainerLayout->addStretch();
    snapdContainerLayout->addWidget(snapdToggleSwitch);

    basicLayout->addWidget(snapdContainer, 3, 0, Qt::AlignCenter);

    // Chaotic-AUR Container
    QWidget *chaoticContainer = new QWidget(this);
    QHBoxLayout *chaoticLayout = new QHBoxLayout(chaoticContainer);
    chaoticLayout->setSpacing(50);

    QLabel *chaoticIconLabel = new QLabel(this);
    QImageReader::setAllocationLimit(512);
    QPixmap chaoticIcon(":/icons/resources/icons/chaotic.svg");
    QPixmap chaoticIconScaled = chaoticIcon.scaled(60, 60, Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation);
    chaoticIconLabel->setPixmap(chaoticIconScaled);

    QLabel *chaoticLabel = new QLabel("Enable/Disable Chaotic-AUR", this);

    bool isChaoticEnabled = (widget->checkChaoticAURStatus() == 0);

    QWidget *chaoticToggleSwitch = new QWidget(this);
    chaoticToggleSwitch->setFixedSize(60, 30);

    if (isChaoticEnabled) {
        chaoticToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }");
    } else {
        chaoticToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }");
    }

    QPushButton *chaoticToggleButton = new QPushButton(chaoticToggleSwitch);
    chaoticToggleButton->setFixedSize(26, 26);
    if (isChaoticEnabled) {
        chaoticToggleButton->move(32, 2);
    } else {
        chaoticToggleButton->move(2, 2);
    }

    chaoticToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none }");
    chaoticLabel->setText(isChaoticEnabled ?
        "Disable/Remove Chaotic AUR" : "Enable/Install Chaotic AUR");

    QPropertyAnimation *chaoticToggleAnim = new QPropertyAnimation(chaoticToggleButton, "pos");
    chaoticToggleAnim->setDuration(500);
    chaoticToggleAnim->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *chaoticIconAnim = new QPropertyAnimation(chaoticToggleButton, "iconSize");
    chaoticIconAnim->setDuration(200);
    chaoticIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(chaoticToggleButton, &QPushButton::pressed, [=]() {
        chaoticIconAnim->setStartValue(QSize(60, 60));
        chaoticIconAnim->setEndValue(QSize(70, 70));
        chaoticIconAnim->start();
    });
    connect(chaoticToggleButton, &QPushButton::released, this, [=]() {
        chaoticIconAnim->setStartValue(QSize(70, 70));
        chaoticIconAnim->setEndValue(QSize(60, 60));
        chaoticIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(chaoticToggleButton, &QPushButton::clicked, this, [=]() mutable {
    widget->chaoticAUR();

    // Update UI after operation
    isChaoticEnabled = (widget->checkChaoticAURStatus() == 0);
    if (isChaoticEnabled) {
        chaoticToggleAnim->setEndValue(QPoint(32, 2));
        chaoticToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }");
    } else {
        chaoticToggleAnim->setEndValue(QPoint(2, 2));
        chaoticToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }");
    }
    chaoticLabel->setText(isChaoticEnabled ? "Disable/Remove Chaotic-AUR" :
        "Enable/Install Chaotic-AUR");
    chaoticToggleAnim->start();
    });

    chaoticLabel->setText(isChaoticEnabled ? "Disable/Remove Chaotic-AUR" :
        "Enable/Install Chaotic-AUR");
    chaoticContainer->setFixedWidth(500);

    chaoticLayout->addWidget(chaoticIconLabel);
    chaoticLayout->addWidget(chaoticLabel);
    chaoticLayout->addStretch();
    chaoticLayout->addWidget(chaoticToggleSwitch);

    basicLayout->addWidget(chaoticContainer, 4, 0, Qt::AlignCenter);
    basicPage->setLayout(basicLayout);

    // * == Navigation Buttons
    // * -continue-
    QPushButton *basicContinueButton = new QPushButton("Continue", this);
    basicContinueButton->setCursor(Qt::PointingHandCursor);
    basicContinueButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );
    basicLayout->addWidget(basicContinueButton, 6, 0, Qt::AlignRight);
    basicLayout->setRowStretch(5, 1);

    connect(basicContinueButton, &QPushButton::clicked, this, [stackedWidget]() {
       stackedWidget->setCurrentIndex(2);
    });
    // * -go back-
    QPushButton *basicBackButton = new QPushButton("Back", this);
    basicBackButton->setCursor(Qt::PointingHandCursor);
    basicBackButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );
    basicLayout->addWidget(basicBackButton, 6, 0, Qt::AlignLeft);
    basicLayout->setRowStretch(5, 1);

    connect(basicBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(0);
    });

    // === APPEARANCE PAGE === //
    QGridLayout *appearanceLayout = new QGridLayout(appearancePage);
    appearanceLayout->setContentsMargins(20, 20, 20, 20);
    appearanceLayout->setSpacing(15);

    // Add stretch before content
    appearanceLayout->setRowStretch(0, 1);

    QLabel *appearanceHader = new QLabel("Appearance Settings", this);
    appearanceHader->setStyleSheet("font-size: 18px; color: #888; margin-bottom: 30px;");
    appearanceHader->setAlignment(Qt::AlignCenter);

    appearanceLayout->addWidget(appearanceHader, 1, 0, Qt::AlignCenter);

    // Xray Theming Container
    QWidget *xrayThemingContainer = new QWidget(this);
    QHBoxLayout *xrayThemingLayout = new QHBoxLayout(xrayThemingContainer);
    xrayThemingLayout->setSpacing(50);

    QLabel *xrayThemingIconLabel = new QLabel(this);
    QPixmap xrayThemingIcon(":/icons/resources/icons/xray-theming.svg");
    QPixmap xrayThemingIconScaled = xrayThemingIcon.scaled(60, 60, Qt::KeepAspectRatio,
                                                                Qt::SmoothTransformation);
    xrayThemingIconLabel->setPixmap(xrayThemingIconScaled);
    QLabel *xrayThemingLabel = new QLabel("Enable/Disable Xray Theming", this);

    bool isXrayThemingEnabled = coreInitial->xrayThemeStatus();

    // Toggle switch
    QWidget *xrayThemingToggleSwitch = new QWidget(this);
    xrayThemingToggleSwitch->setFixedSize(60, 30);

    if (isXrayThemingEnabled) {
        xrayThemingToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }"
        );
    } else {
        xrayThemingToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
    }

    QPushButton *xrayThemingToggleButton = new QPushButton(xrayThemingToggleSwitch);
    xrayThemingToggleButton->setFixedSize(26, 26);
    if (isXrayThemingEnabled) {
        xrayThemingToggleButton->move(32, 2);
    } else {
        xrayThemingToggleButton->move(2, 2);
    }

    xrayThemingToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );
    xrayThemingLabel->setText(isXrayThemingEnabled ? "Go back default Breeze Dark" :
        "Switch to Xray_OS theming");

    QPropertyAnimation *xrayThemingToggleAnim = new QPropertyAnimation(xrayThemingToggleButton, "pos");
    xrayThemingToggleAnim->setDuration(500);
    xrayThemingToggleAnim->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *xrayThemingIconAnim = new QPropertyAnimation(xrayThemingToggleButton,
        "iconSize");
    xrayThemingIconAnim->setDuration(200);
    xrayThemingIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(xrayThemingToggleButton, &QPushButton::pressed, [=]() {
        xrayThemingIconAnim->setStartValue(QSize(60, 60));
        xrayThemingIconAnim->setEndValue(QSize(70, 70));
        xrayThemingIconAnim->start();
    });
    connect(xrayThemingToggleButton, &QPushButton::released, [=]() {
        xrayThemingIconAnim->setStartValue(QSize(70, 70));
        xrayThemingIconAnim->setEndValue(QSize(60, 60));
        xrayThemingIconAnim->start();
    });


    connect(xrayThemingToggleButton, &QPushButton::clicked, this, [=]() mutable {
        coreInitial->applyGlobalTheme(isXrayThemingEnabled ? "org.kde.breezedark.desktop" : "XRAY-DARK.desktop");

        QTimer::singleShot(1000, [=]() {
            coreInitial->reloadPlasmaByReplace();
        });

        isXrayThemingEnabled = !isXrayThemingEnabled;
        if (isXrayThemingEnabled) {
            xrayThemingToggleAnim->setEndValue(QPoint(32, 2));
            xrayThemingToggleSwitch->setStyleSheet("QWidget { background-color: #4a9eff; border-radius: 15px; }");
        } else {
            xrayThemingToggleAnim->setEndValue(QPoint(2, 2));
            xrayThemingToggleSwitch->setStyleSheet("QWidget { background-color: #666666; border-radius: 15px; }");
        }
        xrayThemingLabel->setText(isXrayThemingEnabled ? "Go back default Breeze Dark" :
            "Switch to Xray_OS theming");
        xrayThemingToggleAnim->start();
    });
    xrayThemingContainer->setFixedWidth(500);

    xrayThemingLayout->addWidget(xrayThemingIconLabel);
    xrayThemingLayout->addWidget(xrayThemingLabel);
    xrayThemingLayout->addStretch();
    xrayThemingLayout->addWidget(xrayThemingToggleSwitch);

    appearanceLayout->addWidget(xrayThemingContainer, 2, 0, Qt::AlignCenter);

    // Terminal Theming
    QWidget *terminalThemingContainer = new QWidget(this);
    QHBoxLayout *terminalThemingLayout = new QHBoxLayout(terminalThemingContainer);
    terminalThemingLayout->setSpacing(50);

    QLabel *terminalThemingIconLabel = new QLabel(this);
    QPixmap terminalThemingIcon(":/icons/resources/icons/terminal-theming.svg");
    QPixmap terminalThemingIconScaled = terminalThemingIcon.scaled(60, 60,
        Qt::KeepAspectRatio, Qt::SmoothTransformation);

    terminalThemingIconLabel->setPixmap(terminalThemingIconScaled);
    QLabel *terminalThemingLabel = new QLabel("Enable/Disable Terminal Theming", this);
    bool osReleaseStatus = coreInitial->osreleaseStatus();
    bool konsoleProfStatus = coreInitial->konsoleProfStatus();
    int termThemStatus = widget->checkTermThemingStatus();

    bool isTerminalThemingEnabled = (osReleaseStatus &&
        konsoleProfStatus && termThemStatus == 1);

    qDebug() << "isTerminalThemingEnabled: " << isTerminalThemingEnabled;

    QWidget *terminalThemingToggleSwitch = new QWidget(this);
    terminalThemingToggleSwitch->setFixedSize(60, 30);

    if (isTerminalThemingEnabled) {
        terminalThemingToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }"
        );
    } else {
        terminalThemingToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
    }

    QPushButton *terminalThemingToggleButton = new QPushButton(terminalThemingToggleSwitch);
    terminalThemingToggleButton->setFixedSize(26, 26);
    if (isTerminalThemingEnabled) {
        terminalThemingToggleButton->move(32, 2);
    } else {
        terminalThemingToggleButton->move(2, 2);
    }

    terminalThemingToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );
    terminalThemingLabel->setText(isTerminalThemingEnabled ? "Disable terminal theming" :
        "Enable terminal theming");

    QPropertyAnimation *terminalThemingToggleAnimation = new
    QPropertyAnimation(terminalThemingToggleButton, "pos");
    terminalThemingToggleAnimation->setDuration(500);
    terminalThemingToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *terminalThemingIconAnim = new
    QPropertyAnimation(terminalThemingToggleButton, "iconSize");
    terminalThemingIconAnim->setDuration(200);
    terminalThemingIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(terminalThemingToggleButton, &QPushButton::pressed, [=]() {
        terminalThemingIconAnim->setStartValue(QSize(60, 60));
        terminalThemingIconAnim->setEndValue(QSize(70, 70));
        terminalThemingIconAnim->start();
    });
    connect(terminalThemingToggleButton, &QPushButton::released, [=]() {
        terminalThemingIconAnim->setStartValue(QSize(70, 70));
        terminalThemingIconAnim->setEndValue(QSize(60, 60));
        terminalThemingIconAnim->start();
    });

    connect(terminalThemingToggleButton, &QPushButton::clicked, this, [=]() mutable {
        // Get CURRENT status
        bool currentOSreleaseStatus = coreInitial->osreleaseStatus();
        bool osreleaseStatus = coreInitial->osreleaseStatus();
        bool konsoleProfStatus = coreInitial->konsoleProfStatus();
        int currentTermStatus = widget->checkTermThemingStatus();

        if (!isTerminalThemingEnabled) {
            if (!osreleaseStatus) {
                coreInitial->setOSrelease();
            }
            if (!konsoleProfStatus) {
                coreInitial->setKonsoleProfile();
            }
            if (currentTermStatus != 1) {
                QPushButton *dummyButton = new QPushButton();
                widget->disableTermTheme(dummyButton);
                dummyButton->deleteLater();
            }
        }

        if (isTerminalThemingEnabled) {
            QPushButton *dummyButton = new QPushButton();
            widget->disableTermTheme(dummyButton);

            coreInitial->setOSrelease();
            coreInitial->setKonsoleProfile();
            dummyButton->deleteLater();
        }

        isTerminalThemingEnabled = !isTerminalThemingEnabled;

        if (isTerminalThemingEnabled) {
            terminalThemingToggleAnimation->setEndValue(QPoint(32, 2));
            terminalThemingToggleSwitch->setStyleSheet(
                "QWidget { background-color: #4a9eff; border-radius: 15px; }"
            );
        } else {
            terminalThemingToggleAnimation->setEndValue(QPoint(2, 2));
            terminalThemingToggleSwitch->setStyleSheet(
                "QWidget { background-color: #666666; border-radius: 15px; }"
            );
        }
        terminalThemingLabel->setText(isTerminalThemingEnabled ? "Disable terminal theming" :
            "Enable terminal theming");
        terminalThemingToggleAnimation->start();
    });
    terminalThemingContainer->setFixedWidth(500);

    terminalThemingLayout->addWidget(terminalThemingIconLabel);
    terminalThemingLayout->addWidget(terminalThemingLabel);
    terminalThemingLayout->addStretch();
    terminalThemingLayout->addWidget(terminalThemingToggleSwitch);

    appearanceLayout->addWidget(terminalThemingContainer, 3, 0, Qt::AlignCenter);

    // Grub theming
    QWidget *grubThemingContainer = new QWidget(this);
    QHBoxLayout *grubThemingLayout = new QHBoxLayout(grubThemingContainer);
    grubThemingLayout->setSpacing(50);

    QLabel *grubThemingIconLabel = new QLabel(this);
    QPixmap grubThemingIcon(":/icons/resources/icons/grub-theming.png");
    QPixmap grubThemingIconScaled = grubThemingIcon.scaled(60, 60,
        Qt::KeepAspectRatio, Qt::SmoothTransformation);

    grubThemingIconLabel->setPixmap(grubThemingIconScaled);
    QLabel *grubThemingLabel = new QLabel("Enable/Disable Grub Theming", this);
    bool isGrubThemeEnabled = coreInitial->grubThemeStatus();
    qDebug() << "grubThemeStatus: " << isGrubThemeEnabled;

    QWidget *grubThemingToggleSwitch = new QWidget(this);
    grubThemingToggleSwitch->setFixedSize(60, 30);

    if (isGrubThemeEnabled) {
        grubThemingToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }");
    } else {
        grubThemingToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
    }

    QPushButton *grubThemingToggleButton = new
    QPushButton(grubThemingToggleSwitch);
    grubThemingToggleButton->setFixedSize(26, 26);
    if (isGrubThemeEnabled) {
        grubThemingToggleButton->move(32, 2);
    } else {
        grubThemingToggleButton->move(2, 2);
    }

    grubThemingToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );
    grubThemingLabel->setText(isGrubThemeEnabled ? "Disable Grub theming" :
        "Enable Grub theming");

    QPropertyAnimation *grubThemingToggleAnimation = new
    QPropertyAnimation(grubThemingToggleButton, "pos");
    grubThemingToggleAnimation->setDuration(500);
    grubThemingToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *grubThemingIconAnim = new
    QPropertyAnimation(grubThemingToggleButton, "iconSize");
    grubThemingIconAnim->setDuration(200);
    grubThemingIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(grubThemingToggleButton, &QPushButton::pressed, [=]() {
        grubThemingIconAnim->setStartValue(QSize(60, 60));
        grubThemingIconAnim->setEndValue(QSize(70, 70));
        grubThemingIconAnim->start();
    });
    connect(grubThemingToggleButton, &QPushButton::released, [=]() {
       grubThemingIconAnim->setStartValue(QSize(70, 70));
       grubThemingIconAnim->setEndValue(QSize(60, 60));
       grubThemingIconAnim->start();
    });

    connect(grubThemingToggleButton, &QPushButton::clicked, [=]() mutable {
        isGrubThemeEnabled = !isGrubThemeEnabled;

        coreInitial->setGrubTheme();

        if (isGrubThemeEnabled) {
            grubThemingToggleAnimation->setEndValue(QPoint(32, 2));
            grubThemingToggleSwitch->setStyleSheet("QWidget { background-color: #4a9eff; border-radius: 15px; }");
        } else {
            grubThemingToggleAnimation->setEndValue(QPoint(2, 2));
            grubThemingToggleSwitch->setStyleSheet("QWidget { background-color: #666666; border-radius: 15px; }");
        }
        grubThemingLabel->setText(isGrubThemeEnabled ? "Disabled Grub theming" :
            "Enable Grub theming");
        grubThemingToggleAnimation->start();
    });
    grubThemingContainer->setFixedWidth(500);

    grubThemingLayout->addWidget(grubThemingIconLabel);
    grubThemingLayout->addWidget(grubThemingLabel);
    grubThemingLayout->addStretch();
    grubThemingLayout->addWidget(grubThemingToggleSwitch);

    appearanceLayout->addWidget(grubThemingContainer, 4, 0, Qt::AlignCenter);

    // Icons section
    QLabel *iconsLabel = new QLabel("Icons", this);
    iconsLabel->setAlignment(Qt::AlignCenter);
    iconsLabel->setStyleSheet("font-size: 18px; color: #888; margin-top: 30px;");
    appearanceLayout->addWidget(iconsLabel, 5, 0, Qt::AlignCenter);

    // Icons container
    QWidget *iconsContainer = new QWidget(this);
    QHBoxLayout *iconsLayout = new QHBoxLayout(iconsContainer);
    iconsLayout->setSpacing(50);

    QString currentIcons = coreInitial->currentIcons();

    // Dracula
    QWidget *draculaWidget = new QWidget(this);
    QVBoxLayout *draculaLayout = new QVBoxLayout(draculaWidget);
    QPushButton *draculaButton = new QPushButton();
    draculaButton->setCursor(Qt::PointingHandCursor);
    draculaButton->setIcon(QIcon(":/icons/resources/icons/steamDracula.svg"));
    draculaButton->setIconSize(QSize(60, 60));
    draculaButton->setFixedSize(100, 100);
    draculaButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *draculaLabel = new QLabel("Dracula", this);
    draculaLabel->setAlignment(Qt::AlignCenter);
    draculaLabel->setStyleSheet("font-size: 10px;");
    QCheckBox *draculaCheck = new QCheckBox(this);
    if (currentIcons == "Dracula") {
        draculaCheck->setChecked(true);
    }
    draculaCheck->setStyleSheet(
        "QCheckBox::indicator { width: 20px; height: 20px; }"
        "QCheckBox::indicator:unchecked { border: none; border-radius: 10px; background: rgba(255,255,255,0.1); }"
        "QCheckBox::indicator:checked { border: none; border-radius: 10px; background: rgba(24,232,236,0.8); }"
    );
    draculaLayout->addWidget(draculaButton);
    draculaLayout->addWidget(draculaLabel);
    draculaLayout->addWidget(draculaCheck, 0, Qt::AlignCenter);

    // Surfn
    QWidget *surfnWidget = new QWidget(this);
    QVBoxLayout *surfnLayout = new QVBoxLayout(surfnWidget);
    QPushButton *surfnButton = new QPushButton();
    surfnButton->setCursor(Qt::PointingHandCursor);
    surfnButton->setIcon(QIcon(":/icons/resources/icons/steamSurfn.svg"));
    surfnButton->setIconSize(QSize(60, 60));
    surfnButton->setFixedSize(100, 100);
    surfnButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *surfnLabel = new QLabel("Surfn-Tela", this);
    surfnLabel->setAlignment(Qt::AlignCenter);
    surfnLabel->setStyleSheet("font-size: 10px;");
    QCheckBox *surfnCheck = new QCheckBox(this);
    if (currentIcons == "Surfn-Tela") {
        surfnCheck->setChecked(true);
    }
    surfnCheck->setStyleSheet(
        "QCheckBox::indicator { width: 20px; height: 20px; }"
        "QCheckBox::indicator:unchecked { border: none; border-radius: 10px; background: rgba(255,255,255,0.1); }"
        "QCheckBox::indicator:checked { border: none; border-radius: 10px; background: rgba(24,232,236,0.8); }"
    );
    surfnLayout->addWidget(surfnButton);
    surfnLayout->addWidget(surfnLabel);
    surfnLayout->addWidget(surfnCheck, 0, Qt::AlignCenter);

    // Vanilla
    QWidget *vanillaWidget = new QWidget(this);
    QVBoxLayout *vanillaLayout = new QVBoxLayout(vanillaWidget);
    QPushButton *vanillaButton = new QPushButton();
    vanillaButton->setCursor(Qt::PointingHandCursor);
    vanillaButton->setIcon(QIcon(":/icons/resources/icons/steam.svg"));
    vanillaButton->setIconSize(QSize(60, 60));
    vanillaButton->setFixedSize(100, 100);
    vanillaButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *vanillaLabel = new QLabel("breeze-dark", this);
    vanillaLabel->setAlignment(Qt::AlignCenter);
    vanillaLabel->setStyleSheet("font-size: 10px;");
    QCheckBox *vanillaCheck = new QCheckBox(this);
    if (currentIcons == "breeze-dark") {
        vanillaCheck->setChecked(true);
    }
    vanillaCheck->setStyleSheet(
        "QCheckBox::indicator { width: 20px; height: 20px; }"
        "QCheckBox::indicator:unchecked { border: none; border-radius: 10px; background: rgba(255,255,255,0.1); }"
        "QCheckBox::indicator:checked { border: none; border-radius: 10px; background: rgba(24,232,236,0.8); }"
    );
    vanillaLayout->addWidget(vanillaButton);
    vanillaLayout->addWidget(vanillaLabel);
    vanillaLayout->addWidget(vanillaCheck, 0, Qt::AlignCenter);

    QLabel *notifyReset = new QLabel("Some of this changes require a Reboot", this);

    iconsLayout->addStretch();
    iconsLayout->addWidget(draculaWidget);
    iconsLayout->addWidget(surfnWidget);
    iconsLayout->addWidget(vanillaWidget);
    iconsLayout->addStretch();
    appearanceLayout->addWidget(iconsContainer, 6, 0, Qt::AlignCenter);
    appearanceLayout->addWidget(notifyReset, 7, 0, Qt::AlignCenter);

    // Radio button behavior
    connect(draculaButton, &QPushButton::clicked, [=]() {
        coreInitial->setIcons("Dracula");
        draculaCheck->setChecked(true);
        vanillaCheck->setChecked(false);
        surfnCheck->setChecked(false);
    });
    connect(surfnButton, &QPushButton::clicked, [=]() {
        coreInitial->setIcons("Surfn-Tela");
        surfnCheck->setChecked(true);
        draculaCheck->setChecked(false);
        vanillaCheck->setChecked(false);
    });
    connect(vanillaButton, &QPushButton::clicked, [=]() {
        coreInitial->setIcons("breeze-dark");
        vanillaCheck->setChecked(true);
        draculaCheck->setChecked(false);
        surfnCheck->setChecked(false);
    });

    // Same check/uncheck logic for the QCheckBoxes (circles)
    connect(draculaCheck, &QCheckBox::clicked, [=]() {
        coreInitial->setIcons("Dracula");
        draculaCheck->setChecked(true);
        surfnCheck->setChecked(false);
        vanillaCheck->setChecked(false);
    });
    connect(surfnCheck, &QCheckBox::clicked, [=]() {
        coreInitial->setIcons("Surfn-Tela");
        surfnCheck->setChecked(true);
        vanillaCheck->setChecked(false);
        draculaCheck->setChecked(false);
    });
    connect(vanillaCheck, &QCheckBox::clicked, [=]() {
        coreInitial->setIcons("breeze-dark");
        vanillaCheck->setChecked(true);
        draculaCheck->setChecked(false);
        surfnCheck->setChecked(false);
    });

    appearancePage->setLayout(appearanceLayout);

    // * == Navigation Buttons
    // * -continue-
    QPushButton *termThemContinue = new QPushButton("Continue",this);
    termThemContinue->setCursor(Qt::PointingHandCursor);
    termThemContinue->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080"
        "}"
    );
    // Add stretch after content but before buttons
    appearanceLayout->setRowStretch(8, 1);

    appearanceLayout->addWidget(termThemContinue, 9, 0, Qt::AlignRight);
    connect(termThemContinue, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(3);
    });
    // * -go back-
    QPushButton *termThemBack = new QPushButton("Back", this);
    termThemBack->setCursor(Qt::PointingHandCursor);
    termThemBack->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );
    appearanceLayout->addWidget(termThemBack, 9, 0, Qt::AlignLeft);
    connect(termThemBack, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(1);
    });

    // === AUR-HELPERS PAGE === //
    QGridLayout *AURhelpersLayout = new QGridLayout(AURhelpersPage);
    AURhelpersLayout->setContentsMargins(20, 20, 20, 20);
    AURhelpersLayout->setSpacing(15);

    AURhelpersLayout->setRowStretch(0, 1);

    QLabel *AURhelperHeader = new QLabel("AUR Helper Setup", this);
    AURhelperHeader->setStyleSheet("font-size: 18px; color: #888; margin-bottom: 30px;");
    AURhelperHeader->setAlignment(Qt::AlignCenter);

    AURhelpersLayout->addWidget(AURhelperHeader, 1, 0, Qt::AlignCenter);

    // Yay Container
    QWidget *yayContainer = new QWidget(this);
    QHBoxLayout *yayLayout = new QHBoxLayout(yayContainer);
    yayLayout->setSpacing(50);

    QLabel *yayIconLabel = new QLabel(this);
    QPixmap yayIcon(":/icons/resources/icons/yay.png");
    QPixmap yayIconScaled = yayIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    yayIconLabel->setPixmap(yayIconScaled);

    bool isYayEnabled = coreInitial->aurStatus(QString("yay"));
    QLabel *yayLabel = new QLabel((isYayEnabled) ? "Disable/Remove Yay" :
        "Enable/Install Yay", this);
    QWidget *yayToggleSwitch = new QWidget(this);
    yayToggleSwitch->setFixedSize(60, 30);
    QPushButton *yayToggleButton = new QPushButton(yayToggleSwitch);
    yayToggleButton->setFixedSize(26, 26);
    yayToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );

    if (isYayEnabled) {
        yayToggleSwitch->setStyleSheet("QWidget { background-color: #4a9eff; border-radius: 15px; }");
        yayToggleButton->move(32, 2);
    } else {
        yayToggleSwitch->setStyleSheet("QWidget { background-color: #666666; border-radius: 15px; }");
        yayToggleButton->move(2, 2);
    }

    QPropertyAnimation *yayToggleAnimation = new QPropertyAnimation(yayToggleButton, "pos");
    yayToggleAnimation->setDuration(500);
    yayToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *yayToggleIconAnim = new QPropertyAnimation(yayToggleButton, "iconSize");
    yayToggleIconAnim->setDuration(200);
    yayToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(yayToggleButton, &QPushButton::pressed, [=]() {
        yayToggleIconAnim->setStartValue(QSize(60, 60));
        yayToggleIconAnim->setEndValue(QSize(70, 70));
        yayToggleIconAnim->start();
    });
    connect(yayToggleButton, &QPushButton::released, [=]() {
       yayToggleIconAnim->setStartValue(QSize(70, 70));
       yayToggleIconAnim->setEndValue(QSize(60, 60));
       yayToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(yayToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveAUR(this, QString("yay"), [=](bool success) mutable {
            if (success) {
                isYayEnabled = coreInitial->aurStatus(QString("yay"));

                if (isYayEnabled) {
                    yayToggleAnimation->setEndValue(QPoint(32, 2));
                    yayToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                    );
                } else {
                    yayToggleAnimation->setEndValue(QPoint(2, 2));
                    yayToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #666666; border-radius: 15px; }"
                    );
                }
                yayLabel->setText(isYayEnabled ? "Disable/Remove Yay" : "Enable/Install Yay");
                yayToggleAnimation->start();
            }
        });
    });

    yayContainer->setFixedWidth(500);
    yayLayout->addWidget(yayIconLabel);
    yayLayout->addWidget(yayLabel);
    yayLayout->addStretch();
    yayLayout->addWidget(yayToggleSwitch);

    AURhelpersLayout->addWidget(yayContainer, 2, 0, Qt::AlignCenter);

    QWidget *paruContainer = new QWidget(this);
    QHBoxLayout *paruContainerLayout = new QHBoxLayout(paruContainer);
    paruContainerLayout->setSpacing(50);

    QLabel *paruIconLabel = new QLabel(this);
    QPixmap paruIcon(":/icons/resources/icons/paru.png");
    QPixmap paruIconScaled = paruIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    paruIconLabel->setPixmap(paruIconScaled);

    bool isParuEnabled = coreInitial->aurStatus(QString("paru"));
    QLabel *paruLabel = new QLabel((isParuEnabled) ? "Disable/Remove Paru" :
        "Enable/Install Paru", this);
    QWidget *paruToggleSwitch = new QWidget(this);
    paruToggleSwitch->setFixedSize(60, 30);
    QPushButton *paruToggleButton = new QPushButton(paruToggleSwitch);
    paruToggleButton->setFixedSize(26, 26);
    paruToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );

    if (isParuEnabled) {
        paruToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }"
        );
        paruToggleButton->move(32, 2);
    } else {
        paruToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
        paruToggleButton->move(2, 2);
    }

    QPropertyAnimation *paruToggleAnimation = new QPropertyAnimation(paruToggleButton, "pos");
    paruToggleAnimation->setDuration(500);
    paruToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *paruToggleIconAnim = new QPropertyAnimation(paruToggleButton, "iconSize");
    paruToggleIconAnim->setDuration(200);
    paruToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(paruToggleButton, &QPushButton::pressed, [=]() {
        paruToggleIconAnim->setStartValue(QSize(60, 60));
        paruToggleIconAnim->setEndValue(QSize(70, 70));
        paruToggleIconAnim->start();
    });
    connect(paruToggleButton, &QPushButton::released, [=]() {
        paruToggleIconAnim->setStartValue(QSize(70, 70));
        paruToggleIconAnim->setEndValue(QSize(60, 60));
        paruToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(paruToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveAUR(this, QString("paru"), [=](bool success) mutable {
            if (success) {
                isParuEnabled = coreInitial->aurStatus(QString("paru"));

                if (isParuEnabled) {
                    paruToggleAnimation->setEndValue(QPoint(32, 2));
                    paruToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                    );
                } else {
                    paruToggleAnimation->setEndValue(QPoint(2, 2));
                    paruToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #666666; border-radius: 15px; }"
                    );
                }
                paruLabel->setText(isParuEnabled ? "Disable/Remove Paru" :
                    "Enable/Install Paru");
                paruToggleAnimation->start();
            }
        });
    });

    paruContainer->setFixedWidth(500);

    paruContainerLayout->addWidget(paruIconLabel);
    paruContainerLayout->addWidget(paruLabel);
    paruContainerLayout->addStretch();
    paruContainerLayout->addWidget(paruToggleSwitch);

    AURhelpersLayout->addWidget(paruContainer, 3, 0, Qt::AlignCenter);

    // Tolito
    QWidget *tolitoContainer = new QWidget(this);
    QHBoxLayout *tolitoContainerLayout = new QHBoxLayout(tolitoContainer);
    tolitoContainerLayout->setSpacing(50);

    QLabel *tolitoIconLabel = new QLabel(this);
    QPixmap tolitoIcon(":/icons/resources/icons/tolito.svg");
    QPixmap tolitoIconScaled = tolitoIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    tolitoIconLabel->setPixmap(tolitoIconScaled);

    bool isTolitoEnabled = coreInitial->aurStatus(QString("tolito"));
    QLabel *tolitoLabel = new QLabel((isTolitoEnabled) ? "Disable/Remove Tolito" :
        "Enable/Install Tolito", this);

    QWidget *tolitoToggleSwitch = new QWidget(this);
    tolitoToggleSwitch->setFixedSize(60, 30);
    QPushButton *tolitoToggleButton = new QPushButton(tolitoToggleSwitch);
    tolitoToggleButton->setFixedSize(26, 26);
    tolitoToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none }"
    );

    if (isTolitoEnabled) {
        tolitoToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }"
        );
        tolitoToggleButton->move(32, 2);
    } else {
        tolitoToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
        paruToggleButton->move(2, 2);
    }

    QPropertyAnimation *tolitoToggleAnimation = new QPropertyAnimation(tolitoToggleButton, "pos");
    tolitoToggleAnimation->setDuration(500);
    tolitoToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *tolitoToggleIconAnim = new QPropertyAnimation(tolitoToggleButton, "iconSize");
    tolitoToggleIconAnim->setDuration(200);
    tolitoToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(tolitoToggleButton, &QPushButton::pressed, [=]() {
        tolitoToggleIconAnim->setStartValue(QSize(60, 60));
        tolitoToggleIconAnim->setEndValue(QSize(70, 70));
        tolitoToggleIconAnim->start();
    });
    connect(tolitoToggleButton, &QPushButton::released, [=]() {
        tolitoToggleIconAnim->setStartValue(QSize(70, 70));
        tolitoToggleIconAnim->setEndValue(QSize(60, 60));
        tolitoToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(tolitoToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveAUR(this, QString("tolito"), [=](bool success) mutable {
           if (success) {
               isTolitoEnabled = coreInitial->aurStatus(QString("tolito"));

               if (isTolitoEnabled) {
                   tolitoToggleAnimation->setEndValue(QPoint(32, 2));
                   tolitoToggleSwitch->setStyleSheet(
                       "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                   );
               } else {
                   tolitoToggleAnimation->setEndValue(QPoint(2, 2));
                   tolitoToggleSwitch->setStyleSheet(
                       "QWidget { background-color: #666666; border-radius: 15px; }"
                   );
               }
               tolitoLabel->setText(isTolitoEnabled ? "Disable/Remove Tolito" :
                   "Enable/Install Tolito");
               tolitoToggleAnimation->start();
           }
        });
    });

    tolitoContainer->setMinimumWidth(500);

    tolitoContainerLayout->addWidget(tolitoIconLabel);
    tolitoContainerLayout->addWidget(tolitoLabel);
    tolitoContainerLayout->addStretch();
    tolitoContainerLayout->addWidget(tolitoToggleSwitch);

    AURhelpersLayout->addWidget(tolitoContainer, 4, 0, Qt::AlignCenter);
    AURhelpersPage->setLayout(AURhelpersLayout);

    // * Navigation Buttons
    QPushButton *AURcontinueButton = new QPushButton("Continue", this);
    AURcontinueButton->setCursor(Qt::PointingHandCursor);
    AURcontinueButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );
    AURhelpersLayout->setRowStretch(5, 1);

    AURhelpersLayout->addWidget(AURcontinueButton, 6, 0, Qt::AlignRight);
    connect(AURcontinueButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(4);
    });
    // 8 -go back-
    QPushButton *AURgoBack = new QPushButton("Back", this);
    AURgoBack->setCursor(Qt::PointingHandCursor);
    AURgoBack->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );
    AURhelpersLayout->addWidget(AURgoBack, 6, 0, Qt::AlignLeft);
    connect(AURgoBack, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(2);
    });

    // === APP-STORES PAGE === //
    QGridLayout *appStoresLayout = new QGridLayout(appStoresPage);
    appStoresLayout->setContentsMargins(20, 20, 20, 20);
    appStoresLayout->setSpacing(15);

    appStoresLayout->setRowStretch(0, 1);

    QLabel *appStoresHeader = new QLabel("Appstores and Package Managers", this);
    appStoresHeader->setStyleSheet("font-size: 18px; color: #888; margin-bottom: 30px;");
    appStoresHeader->setAlignment(Qt::AlignCenter);

    appStoresLayout->addWidget(appStoresHeader, 1, 0, Qt::AlignCenter);

    QWidget *discoverContainer = new QWidget(this);
    QHBoxLayout *discoverLayout = new QHBoxLayout(discoverContainer);
    discoverLayout->setSpacing(50);

    QLabel *discoverIconLabel = new QLabel(this);
    QPixmap discoverIcon(":/icons/resources/icons/discover.svg");
    QPixmap discoverIconScaled = discoverIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    discoverIconLabel->setPixmap(discoverIconScaled);

    bool isDiscoverEnabled = coreInitial->storeStatus(QString("discover"));
    QLabel *discoverLabel = new QLabel((isDiscoverEnabled) ? "Disable/Remove Discover" :
        "Enable/Install Discover", this);

    QWidget *discoverToggleSwitch = new QWidget(this);
    discoverToggleSwitch->setFixedSize(60, 30);
    QPushButton *discoverToggleButton = new QPushButton(discoverToggleSwitch);
    discoverToggleButton->setFixedSize(26, 26);

    discoverToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );

    if (isDiscoverEnabled) {
        discoverToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }"
        );
        discoverToggleButton->move(32, 2);
    } else {
        discoverToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
        discoverToggleButton->move(2, 2);
    }

    QPropertyAnimation *discoverToggleAnimation = new
    QPropertyAnimation(discoverToggleButton, "pos");
    discoverToggleAnimation->setDuration(500);
    discoverToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *discoverToggleIconAnim = new
    QPropertyAnimation(discoverToggleButton, "iconSize");
    discoverToggleIconAnim->setDuration(200);
    discoverToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(discoverToggleButton, &QPushButton::pressed, [=]() {
        discoverToggleIconAnim->setStartValue(QSize(60, 60));
        discoverToggleIconAnim->setEndValue(QSize(70, 70));
        discoverToggleIconAnim->start();
    });
    connect(discoverToggleButton, &QPushButton::clicked, [=]() {
        discoverToggleIconAnim->setStartValue(QSize(70, 70));
        discoverToggleIconAnim->setEndValue(QSize(60, 60));
        discoverToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(discoverToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveStore(this, QString("discover"), [=](bool success) mutable {
            if (success) {
                isDiscoverEnabled = coreInitial->storeStatus(QString("discover"));

                if (isDiscoverEnabled) {
                    discoverToggleAnimation->setEndValue(QPoint(32, 2));
                    discoverToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                    );
                } else {
                    discoverToggleAnimation->setEndValue(QPoint(2, 2));
                    discoverToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #666666; border-radius: 15px; }"
                    );
                }
                discoverLabel->setText(isDiscoverEnabled ? "Disable/Remove Discover" :
                    "Enable/Install Discover");
                discoverToggleAnimation->start();
            }
        });
    });
    discoverContainer->setFixedWidth(500);

    discoverLayout->addWidget(discoverIconLabel);
    discoverLayout->addWidget(discoverLabel);
    discoverLayout->addStretch();
    discoverLayout->addWidget(discoverToggleSwitch);

    appStoresLayout->addWidget(discoverContainer, 2, 0, Qt::AlignCenter);

    // Pamac pkg-manager
    QWidget *pamacContainer = new QWidget(this);
    QHBoxLayout *pamacContainerLayout = new QHBoxLayout(pamacContainer);
    pamacContainerLayout->setSpacing(50);

    QLabel *pamacIconLabel = new QLabel(this);
    QPixmap pamacIcon(":/icons/resources/icons/pamac.svg");
    QPixmap pamacIconScaled = pamacIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    pamacIconLabel->setPixmap(pamacIconScaled);

    bool isPamacEnabled = coreInitial->storeStatus("pamac-all");

    QLabel *pamacLabel = new QLabel((isPamacEnabled) ? "Disable/Remove Pamac All" :
    "Enable/Install Pamac All", this);

    QWidget *pamacToggleSwitch = new QWidget(this);
    pamacToggleSwitch->setFixedSize(60, 30);
    QPushButton *pamacToggleButton = new QPushButton(pamacToggleSwitch);
    pamacToggleButton->setFixedSize(26, 26);
    pamacToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );

    if (isPamacEnabled) {
        pamacToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }");
        pamacToggleButton->move(32, 2);
    } else {
        pamacToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
        pamacToggleButton->move(2, 2);
    }

    QPropertyAnimation *pamacToggleAnimation = new
    QPropertyAnimation(pamacToggleButton, "pos");
    pamacToggleAnimation->setDuration(500);
    paruToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *pamacToggleIconAnim = new
    QPropertyAnimation(pamacToggleButton, "iconSize");
    pamacToggleIconAnim->setDuration(200);
    pamacToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(pamacToggleButton, &QPushButton::pressed, [=]() {
        pamacToggleIconAnim->setStartValue(QSize(60, 60));
        pamacToggleIconAnim->setEndValue(QSize(70, 70));
        pamacToggleIconAnim->start();
    });
    connect(pamacToggleButton, &QPushButton::released, [=]() {
        pamacToggleIconAnim->setStartValue(QSize(70, 70));
        pamacToggleIconAnim->setEndValue(QSize(60, 60));
        pamacToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(pamacToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveStore(this, QString("pamac-all"), [=](bool success) mutable {
            if (success) {
                isPamacEnabled = coreInitial->storeStatus(QString("pamac-all"));
                if (isPamacEnabled) {
                    pamacToggleAnimation->setEndValue(QPoint(32, 2));
                    pamacToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                    );
                } else {
                   pamacToggleAnimation->setEndValue(QPoint(2, 2));
                   pamacToggleSwitch->setStyleSheet(
                       "QWidget { background-color: #666666; border-radius: 15px; }"
                   );
               }
                pamacLabel->setText(isPamacEnabled ? "Disable/Remove Pamac All" :
                    "Enable/Install Pamac All");
                pamacToggleAnimation->start();
            }
        });
    });
    pamacContainer->setFixedWidth(500);

    pamacContainerLayout->addWidget(pamacIconLabel);
    pamacContainerLayout->addWidget(pamacLabel);
    pamacContainerLayout->addStretch();
    pamacContainerLayout->addWidget(pamacToggleSwitch);

    appStoresLayout->addWidget(pamacContainer, 3, 0, Qt::AlignCenter);

    QWidget *octopiContainer = new QWidget(this);
    QHBoxLayout *octopiContainerLayout = new QHBoxLayout(octopiContainer);
    octopiContainerLayout->setSpacing(50);

    QLabel *octopiIconLabel = new QLabel(this);
    QPixmap octopiIcon(":/icons/resources/icons/octopi.svg");
    QPixmap octopiIconScaled = octopiIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    octopiIconLabel->setPixmap(octopiIconScaled);

    bool isOctopiEnabled = coreInitial->storeStatus(QString("octopi"));
    QLabel *octopiLabel = new QLabel((isOctopiEnabled) ? "Disable/Remove Octopi" :
       "Enable/Install Octopi", this);

    QWidget *octopiToggleSwitch = new QWidget(this);
    octopiToggleSwitch->setFixedSize(60, 30);
    QPushButton *octopiToggleButton = new QPushButton(octopiToggleSwitch);
    octopiToggleButton->setFixedSize(26, 26);
    octopiToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none; }"
    );

    if (isOctopiEnabled) {
        octopiToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }");
        octopiToggleButton->move(32, 2);
    } else {
        octopiToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
        octopiToggleButton->move(2, 2);
    }

    QPropertyAnimation *octopiToggleAnimation = new
    QPropertyAnimation(octopiToggleButton, "pos");
    octopiToggleAnimation->setDuration(500);
    octopiToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *octopiToggleIconAnim = new
    QPropertyAnimation(octopiToggleButton, "iconSize");
    octopiToggleIconAnim->setDuration(200);
    octopiToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(octopiToggleButton, &QPushButton::pressed, [=]() {
        octopiToggleIconAnim->setStartValue(QSize(60, 60));
        octopiToggleIconAnim->setEndValue(QSize(70, 70));
        octopiToggleIconAnim->start();
    });
    connect(octopiToggleButton, &QPushButton::released, [=]() {
        octopiToggleIconAnim->setStartValue(QSize(70, 70));
        octopiToggleIconAnim->setEndValue(QSize(60, 60));
        octopiToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(octopiToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveStore(this, QString("octopi"), [=](bool success) mutable {
            if (success) {
                isOctopiEnabled = coreInitial->storeStatus(QString("octopi"));
                if (isOctopiEnabled) {
                    octopiToggleAnimation->setEndValue(QPoint(32, 2));
                    octopiToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                    );
                } else {
                    octopiToggleAnimation->setEndValue(QPoint(2, 2));
                    octopiToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #666666; border-radius: 15px; }"
                    );
                }
                octopiLabel->setText(isOctopiEnabled ? "Disable/Remove Octopi" :
                    "Enable/Install Octopi");
                octopiToggleAnimation->start();
            }
        });
    });
    octopiContainer->setFixedWidth(500);

    octopiContainerLayout->addWidget(octopiIconLabel);
    octopiContainerLayout->addWidget(octopiLabel);
    octopiContainerLayout->addStretch();
    octopiContainerLayout->addWidget(octopiToggleSwitch);

    appStoresLayout->addWidget(octopiContainer, 4, 0, Qt::AlignCenter);

    // Bazaar container
    QWidget *bazaarContainer = new QWidget(this);
    QHBoxLayout *bazaarContainerLayout = new QHBoxLayout(bazaarContainer);
    bazaarContainerLayout->setSpacing(50);

    QLabel *bazaarIconLabel = new QLabel(this);
    QPixmap bazaarIcon(":/icons/resources/icons/bazaar.svg");
    QPixmap bazaarIconScaled = bazaarIcon.scaled(60, 60, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);
    bazaarIconLabel->setPixmap(bazaarIconScaled);

    bool isBazaarEnabled = coreInitial->storeStatus(QString("bazaar"));
    QLabel *bazaarLabel = new QLabel((isBazaarEnabled) ? "Disable/Remove Bazaar" :
        "Enable/Install Bazaar", this);

    QWidget *bazaarToggleSwitch = new QWidget(this);
    bazaarToggleSwitch->setFixedSize(60, 30);
    QPushButton *bazaarToggleButton = new QPushButton(bazaarToggleSwitch);
    bazaarToggleButton->setFixedSize(26, 26);
    bazaarToggleButton->setStyleSheet(
        "QPushButton { background-color: white; border-radius: 13px; border: none }"
    );

    if (isBazaarEnabled) {
        bazaarToggleSwitch->setStyleSheet(
            "QWidget { background-color: #4a9eff; border-radius: 15px; }"
        );
        bazaarToggleButton->move(32, 2);
    } else {
        bazaarToggleSwitch->setStyleSheet(
            "QWidget { background-color: #666666; border-radius: 15px; }"
        );
        bazaarToggleButton->move(2, 2);
    }

    QPropertyAnimation *bazaarToggleAnimation = new
    QPropertyAnimation(bazaarToggleButton, "pos");
    bazaarToggleAnimation->setDuration(500);
    bazaarToggleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation *bazaarToggleIconAnim = new
    QPropertyAnimation(bazaarToggleButton, "iconSize");
    bazaarToggleIconAnim->setDuration(200);
    bazaarToggleIconAnim->setEasingCurve(QEasingCurve::OutBack);

    // Bounce effect on press
    connect(bazaarToggleButton, &QPushButton::pressed, [=]() {
        bazaarToggleIconAnim->setStartValue(QSize(60, 60));
        bazaarToggleIconAnim->setEndValue(QSize(70, 70));
        bazaarToggleIconAnim->start();
    });
    connect(bazaarToggleButton, &QPushButton::released, [=]() {
        bazaarToggleIconAnim->setStartValue(QSize(70, 70));
        bazaarToggleIconAnim->setEndValue(QSize(60, 60));
        bazaarToggleIconAnim->start();
    });

    // Toggle effect on click and logic
    connect(bazaarToggleButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getRemoveStore(this, QString("bazaar"), [=](bool success) mutable {
            if (success) {
                isBazaarEnabled = coreInitial->storeStatus(QString("bazaar"));

                if (isBazaarEnabled) {
                    bazaarToggleAnimation->setEndValue(QPoint(32, 2));
                    bazaarToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #4a9eff; border-radius: 15px; }"
                    );
                } else {
                    bazaarToggleAnimation->setEndValue(QPoint(2, 2));
                    bazaarToggleSwitch->setStyleSheet(
                        "QWidget { background-color: #666666; border-radius: 15px; }"
                    );
                }
                bazaarLabel->setText(isBazaarEnabled ? "Disable/Remove Bazaar" :
                    "Enable/Install Bazaar");
                bazaarToggleAnimation->start();
            }
        });
    });
    bazaarContainer->setFixedWidth(500);

    bazaarContainerLayout->addWidget(bazaarIconLabel);
    bazaarContainerLayout->addWidget(bazaarLabel);
    bazaarContainerLayout->addStretch();
    bazaarContainerLayout->addWidget(bazaarToggleSwitch);

    appStoresLayout->addWidget(bazaarContainer, 5, 0, Qt::AlignCenter);

    // * == Navigation Buttons
    // * -continue-
    QPushButton *bazaarContinueButton = new QPushButton("Continue", this);
    bazaarContinueButton->setCursor(Qt::PointingHandCursor);
    bazaarContinueButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #004080"
        "}"
    );

    appStoresLayout->setRowStretch(6, 1);

    appStoresLayout->addWidget(bazaarContinueButton, 7, 0, Qt::AlignRight);
    connect(bazaarContinueButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(5);
    });
    // * -go back-
    QPushButton *bazaarBackButton = new QPushButton("Back", this);
    bazaarBackButton->setCursor(Qt::PointingHandCursor);
    bazaarBackButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );
    appStoresLayout->addWidget(bazaarBackButton, 7, 0, Qt::AlignLeft);
    connect(bazaarBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(3);
    });

    appStoresPage->setLayout(appStoresLayout);

    // === GAMING === //
    QVBoxLayout *gamingLayout = new QVBoxLayout(gamingPage);
    gamingLayout->setContentsMargins(20, 20, 20, 20);

    gamingLayout->addStretch(1);

    QLabel *arch7zIconLabel = new QLabel(this);
    QPixmap arch7zIcon(":/icons/resources/icons/arch7z-gaming-meta.svg");
    QPixmap arch7zIconScaled = arch7zIcon.scaled(100, 100, Qt::KeepAspectRatio,
        Qt::SmoothTransformation);

    arch7zIconLabel->setPixmap(arch7zIconScaled);
    gamingLayout->addWidget(arch7zIconLabel, 0, Qt::AlignCenter);

    QLabel *gamingHeader = new QLabel("<h1>Get Arch7z-Gaming-Meta</h1>", this);
    gamingHeader->setAlignment(Qt::AlignCenter);
    gamingLayout->addWidget(gamingHeader, 0, Qt::AlignCenter);

    QLabel *gamingMetaDesc = new QLabel("This Gaming Meta pack will allow you to get all "
        "\nthe necessary stuff that you need to play games \non Linux", this);
    gamingMetaDesc->setStyleSheet("font-size: 12px; color: #888; margin-bottom: 30px;");
    gamingMetaDesc->setWordWrap(true);
    gamingMetaDesc->setAlignment(Qt::AlignCenter);
    //gamingMetaDesc->setStyleSheet("margin: 20px;");

    gamingLayout->addWidget(gamingMetaDesc, 0, Qt::AlignCenter);

    bool gamingMetaStatus = coreInitial->gamingMetaStatus();
    qDebug() << "gamingMetaStatus-before-logic: " << gamingMetaStatus;

    QPushButton *arch7zGamingButton = new QPushButton((gamingMetaStatus) ?
        "Remove Arch7z Gaming Meta" : "Get Arch7z Gaming Meta", this);

    arch7zGamingButton->setCursor(Qt::PointingHandCursor);
    arch7zGamingButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #18e8ec;"
        "   color: black;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:pressed {"
        " background-color: #13babd;"
        "}"
    );

    // Button logic lambda
    connect(arch7zGamingButton, &QPushButton::clicked, [=]() mutable {
        coreInitial->getArch7zGamingMeta(this, [=](bool success) mutable {
            if (success) {
                gamingMetaStatus = coreInitial->gamingMetaStatus();

                arch7zGamingButton->setText((gamingMetaStatus) ? "Remove Arch7z Gaming Meta" :
                    "Get Arch7z Gaming Meta");
            }
        });
    });

    gamingLayout->addWidget(arch7zGamingButton, 0, Qt::AlignCenter);
    gamingLayout->addStretch(1);

    // * == Navigation Buttons
    QHBoxLayout *gamingButtonLayout = new QHBoxLayout();

    QPushButton *gamingBackButton = new QPushButton("Back", this);
    gamingBackButton->setCursor(Qt::PointingHandCursor);
    gamingBackButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );

    QPushButton *gamingContinueButton = new QPushButton("Continue", this);
    gamingContinueButton->setCursor(Qt::PointingHandCursor);
    gamingContinueButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );

    gamingButtonLayout->addWidget(gamingBackButton);
    gamingButtonLayout->addStretch();
    gamingButtonLayout->addWidget(gamingContinueButton);

    gamingLayout->addLayout(gamingButtonLayout);

    connect(gamingBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(4);
    });
    connect(gamingContinueButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(6);
    });

    gamingPage->setLayout(gamingLayout);

    // === SUPPORT === //
    QVBoxLayout *supportLayout = new QVBoxLayout(supportPage);
    supportLayout->setContentsMargins(20, 20, 20, 20);

    supportLayout->addStretch(1);

    QLabel *xraySupportIconLabel = new QLabel(this);
    QPixmap xraySupportIcon(":/icons/resources/icons/xray.svg");
    QPixmap xraySupportIconScaled = xraySupportIcon.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    xraySupportIconLabel->setPixmap(xraySupportIconScaled);
    supportLayout->addWidget(xraySupportIconLabel, 0, Qt::AlignCenter);

    QLabel *supportHeader = new QLabel("<h1>Support us</h1>", this);
    supportHeader->setAlignment(Qt::AlignCenter);
    supportLayout->addWidget(supportHeader, 0, Qt::AlignCenter);

    QLabel *supportDescription = new QLabel("Join Us on any of the social media sites below "
        "\njust click on any of the buttons, you'll have a \ngood time, Thank You", this);
    supportDescription->setStyleSheet("font-size: 12px; color: #888; margin-bottom: 30px;");
    supportDescription->setWordWrap(true);
    supportDescription->setAlignment(Qt::AlignCenter);

    supportLayout->addWidget(supportDescription, 0, Qt::AlignCenter);
    supportLayout->addStretch(1);

    QWidget *supportButtonsContainer = new QWidget(this);
    QHBoxLayout *supportButtonsLayout = new QHBoxLayout(supportButtonsContainer);

    // Adding icons for buttons
    QPixmap supportDiscordIcon(":/icons/resources/icons/discord.png");
    QPixmap supportFacebookIcon(":/icons/resources/icons/facebook.png");
    QPixmap supportInstaIcon(":/icons/resources/icons/instagram.png");
    QPixmap supportLinkedinIcon(":/icons/resources/icons/linkedin.png");
    QPixmap supportKofiIcon(":/icons/resources/icons/kofi.svg");
    QPixmap supportTwitterIcon(":/icons/resources/icons/twitter.png");
    QPixmap supportYoutubeIcon(":/icons/resources/icons/youtube.png");

    // Discord
    QWidget *discordButtonWidget = new QWidget(this);
    QVBoxLayout *discordButtonLayout = new QVBoxLayout(discordButtonWidget);
    QPushButton *discordButton = new QPushButton();
    discordButton->setCursor(Qt::PointingHandCursor);
    discordButton->setIcon(supportDiscordIcon);
    discordButton->setIconSize(QSize(60, 60));
    discordButton->setFixedSize(100, 100);
    discordButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *discordLabel = new QLabel("Discord", this);
    discordLabel->setAlignment(Qt::AlignCenter);
    discordLabel->setStyleSheet("font-size: 10px;");

    QPropertyAnimation *discordAnimation = new QPropertyAnimation(discordButton, "iconSize");
    discordAnimation->setDuration(100);
    discordAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(discordButton, &QPushButton::pressed, [=]() {
        discordAnimation->setStartValue(QSize(60, 60));
        discordAnimation->setEndValue(QSize(70, 70));
        discordAnimation->start();
    });

    connect(discordButton, &QPushButton::released, [=]() {
        discordAnimation->setStartValue(QSize(70, 70));
        discordAnimation->setEndValue(QSize(60, 60));
        discordAnimation->start();
    });

    connect(discordButton, &QPushButton::clicked, [=]() {
        // qDebug() << "Discord button clicked - opening URL";
        QDesktopServices::openUrl(QUrl("https://discord.com/invite/dBR7wR3ABk"));
    });

    discordButtonLayout->addWidget(discordButton);
    discordButtonLayout->addWidget(discordLabel);

    // Facebook
    QWidget *facebookButtonWidget = new QWidget(this);
    QVBoxLayout *facebookButtonLayout = new QVBoxLayout(facebookButtonWidget);
    QPushButton *facebookButton = new QPushButton();
    facebookButton->setCursor(Qt::PointingHandCursor);
    facebookButton->setIcon(supportFacebookIcon);
    facebookButton->setIconSize(QSize(60, 60));
    facebookButton->setFixedSize(100, 100);
    facebookButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *facebookLabel = new QLabel("Facebook", this);
    facebookLabel->setAlignment(Qt::AlignCenter);
    facebookLabel->setStyleSheet("font-size: 10px");

    QPropertyAnimation *facebookAnimation = new QPropertyAnimation(facebookButton, "iconSize");
    facebookAnimation->setDuration(100);
    facebookAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(facebookButton, &QPushButton::pressed, [=]() {
        facebookAnimation->setStartValue(QSize(60, 60));
        facebookAnimation->setEndValue(QSize(70, 70));
        facebookAnimation->start();
    });

    connect(facebookButton, &QPushButton::released, [=]() {
        facebookAnimation->setStartValue(QSize(70, 70));
        facebookAnimation->setEndValue(QSize(60, 60));
        facebookAnimation->start();
    });

    connect(facebookButton, &QPushButton::clicked, [=]() {
        QDesktopServices::openUrl(QUrl("https://www.facebook.com/profile.php?id=61572808981258"));
    });

    facebookButtonLayout->addWidget(facebookButton);
    facebookButtonLayout->addWidget(facebookLabel);

    // Instagram
    QWidget *instagramButtonWidget = new QWidget(this);
    QVBoxLayout *instagramButtonLayout = new QVBoxLayout(instagramButtonWidget);
    QPushButton *instagramButton = new QPushButton();
    instagramButton->setCursor(Qt::PointingHandCursor);
    instagramButton->setIcon(supportInstaIcon);
    instagramButton->setIconSize(QSize(60, 60));
    instagramButton->setFixedSize(100, 100);
    instagramButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *instagramLabel = new QLabel("Instagram", this);
    instagramLabel->setAlignment(Qt::AlignCenter);
    instagramLabel->setStyleSheet("font-size: 10px");

    QPropertyAnimation *instagramAnimation = new QPropertyAnimation(instagramButton, "iconSize");
    instagramAnimation->setDuration(100);
    instagramAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(instagramButton, &QPushButton::pressed, [=]() {
        instagramAnimation->setStartValue(QSize(60, 60));
        instagramAnimation->setEndValue(QSize(70, 70));
        instagramAnimation->start();
    });

    connect(instagramButton, &QPushButton::released, [=]() {
        instagramAnimation->setStartValue(QSize(70, 70));
        instagramAnimation->setEndValue(QSize(60, 60));
        instagramAnimation->start();
    });

    connect(instagramButton, &QPushButton::clicked, [=]() {
        QDesktopServices::openUrl(QUrl("https://www.instagram.com/xray_os/"));
    });

    instagramButtonLayout->addWidget(instagramButton);
    instagramButtonLayout->addWidget(instagramLabel);

    // Kofi
    QWidget *kofiButtonWidget = new QWidget(this);
    QVBoxLayout *kofiButtonLayout = new QVBoxLayout(kofiButtonWidget);
    QPushButton *kofiButton = new QPushButton();
    kofiButton->setCursor(Qt::PointingHandCursor);
    kofiButton->setIcon(supportKofiIcon);
    kofiButton->setIconSize(QSize(60, 60));
    kofiButton->setFixedSize(100, 100);
    kofiButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *kofiLabel = new QLabel("Kofi", this);
    kofiLabel->setAlignment(Qt::AlignCenter);
    kofiLabel->setStyleSheet("font-size: 10px");

    QPropertyAnimation *kofiAnimation = new QPropertyAnimation(kofiButton, "iconSize");
    kofiAnimation->setDuration(100);
    kofiAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(kofiButton, &QPushButton::pressed, [=]() {
        kofiAnimation->setStartValue(QSize(60, 60));
        kofiAnimation->setEndValue(QSize(70, 70));
        kofiAnimation->start();
    });

    connect(kofiButton, &QPushButton::pressed, [=]() {
        kofiAnimation->setStartValue(QSize(70, 70));
        kofiAnimation->setEndValue(QSize(60, 60));
        kofiAnimation->start();
    });

    connect(kofiButton, &QPushButton::clicked, [=]() {
        QDesktopServices::openUrl(QUrl("https://ko-fi.com/xray_os"));
    });

    kofiButtonLayout->addWidget(kofiButton);
    kofiButtonLayout->addWidget(kofiLabel);

    // Linkedin
    QWidget *linkedinButtonWidget = new QWidget(this);
    QVBoxLayout *linkedinButtonLayout = new QVBoxLayout(linkedinButtonWidget);
    QPushButton *linkedinButton = new QPushButton();
    linkedinButton->setCursor(Qt::PointingHandCursor);
    linkedinButton->setIcon(supportLinkedinIcon);
    linkedinButton->setIconSize(QSize(60, 60));
    linkedinButton->setFixedSize(100, 100);
    linkedinButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *linkedinLabel = new QLabel("Linkedin", this);
    linkedinLabel->setAlignment(Qt::AlignCenter);
    linkedinLabel->setStyleSheet("font-size: 10px");

    QPropertyAnimation *linkedinAnimation = new QPropertyAnimation(linkedinButton, "iconSize");
    linkedinAnimation->setDuration(100);
    linkedinAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(linkedinButton, &QPushButton::pressed, [=]() {
        linkedinAnimation->setStartValue(QSize(60, 60));
        linkedinAnimation->setEndValue(QSize(70, 70));
        linkedinAnimation->start();
    });

    connect(linkedinButton, &QPushButton::released, [=]() {
        linkedinAnimation->setStartValue(QSize(70, 70));
        linkedinAnimation->setEndValue(QSize(60, 60));
        linkedinAnimation->start();
    });

    connect(linkedinButton, &QPushButton::clicked, [=]() {
       QDesktopServices::openUrl(QUrl("https://www.linkedin.com/in/angel-gustavo-contreras-lopez-258023354/"));
    });

    linkedinButtonLayout->addWidget(linkedinButton);
    linkedinButtonLayout->addWidget(linkedinLabel);

    // Twitter
    QWidget *twitterButtonWidget = new QWidget(this);
    QVBoxLayout *twitterButtonLayout = new QVBoxLayout(twitterButtonWidget);
    QPushButton *twitterButton = new QPushButton();
    twitterButton->setCursor(Qt::PointingHandCursor);
    twitterButton->setIcon(supportTwitterIcon);
    twitterButton->setIconSize(QSize(60, 60));
    twitterButton->setFixedSize(100, 100);
    twitterButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *twitterLabel = new QLabel("Twitter", this);
    twitterLabel->setAlignment(Qt::AlignCenter);
    twitterLabel->setStyleSheet("font-size: 10px");

    QPropertyAnimation *twitterAnimation = new QPropertyAnimation(twitterButton, "iconSize");
    twitterAnimation->setDuration(100);
    twitterAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(twitterButton, &QPushButton::pressed, [=]() {
        twitterAnimation->setStartValue(QSize(60, 60));
        twitterAnimation->setEndValue(QSize(70, 70));
        twitterAnimation->start();
    });

    connect(twitterButton, &QPushButton::pressed, [=]() {
        twitterAnimation->setStartValue(QSize(70, 70));
        twitterAnimation->setEndValue(QSize(60, 60));
        twitterAnimation->start();
    });

    connect(twitterButton, &QPushButton::clicked, [=]() {
        QDesktopServices::openUrl(QUrl("https://x.com/xray_os"));
    });

    twitterButtonLayout->addWidget(twitterButton);
    twitterButtonLayout->addWidget(twitterLabel);

    // YouTube
    QWidget *youtubeButtonWidget = new QWidget(this);
    QVBoxLayout *youtubeButtonLayout = new QVBoxLayout(youtubeButtonWidget);
    QPushButton *youtubeButton = new QPushButton();
    youtubeButton->setCursor(Qt::PointingHandCursor);
    youtubeButton->setIcon(supportYoutubeIcon);
    youtubeButton->setIconSize(QSize(60, 60));
    youtubeButton->setFixedSize(100, 100);
    youtubeButton->setStyleSheet("QPushButton { background: transparent; border: none; }");
    QLabel *youtubeLabel = new QLabel("YouTube", this);
    youtubeLabel->setAlignment(Qt::AlignCenter);
    youtubeLabel->setStyleSheet("font-size: 10px");

    QPropertyAnimation *youtubeAnimation = new QPropertyAnimation(youtubeButton, "iconSize");
    youtubeAnimation->setDuration(100);
    youtubeAnimation->setEasingCurve(QEasingCurve::OutBack);

    connect(youtubeButton, &QPushButton::pressed, [=]() {
        youtubeAnimation->setStartValue(QSize(60, 60));
        youtubeAnimation->setEndValue(QSize(70, 70));
        youtubeAnimation->start();
    });

    connect(youtubeButton, &QPushButton::released, [=]() {
        youtubeAnimation->setStartValue(QSize(70, 70));
        youtubeAnimation->setEndValue(QSize(60, 60));
        youtubeAnimation->start();
    });

    connect(youtubeButton, &QPushButton::clicked, [=]() {
        QDesktopServices::openUrl(QUrl("https://www.youtube.com/@xray-technologies"));
    });

    youtubeButtonLayout->addWidget(youtubeButton);
    youtubeButtonLayout->addWidget(youtubeLabel);

    supportButtonsLayout->addStretch();
    supportButtonsLayout->addWidget(discordButtonWidget);
    supportButtonsLayout->addWidget(facebookButtonWidget);
    supportButtonsLayout->addWidget(instagramButtonWidget);
    supportButtonsLayout->addWidget(kofiButtonWidget);
    supportButtonsLayout->addWidget(linkedinButtonWidget);
    supportButtonsLayout->addWidget(twitterButtonWidget);
    supportButtonsLayout->addWidget(youtubeButtonWidget);
    supportButtonsLayout->addStretch();

    supportButtonsLayout->setContentsMargins(0, 0, 0, 50);

    supportLayout->addWidget(supportButtonsContainer);

    // * == Navigation Buttons
    QHBoxLayout *supportNavButtonLayout = new QHBoxLayout();

    // * -done-
    QPushButton *doneButton = new QPushButton("Done", this);
    doneButton->setCursor(Qt::PointingHandCursor);
    doneButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:hover {"
        "   background-color: #004080;"
        "}"
    );

    connect(doneButton, &QPushButton::clicked, [=]() {
        QString homeDir = QDir::homePath();
        QFile configFile(homeDir + "/tolitica-home-settings/tolitica.conf");

        QTextStream in(&configFile);
        QStringList lines;

        if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while (!in.atEnd()) {
                QString line = in.readLine();

                if (line.startsWith("initialSetup = 0")) {
                    lines << "initialSetup = 1";
                } else {
                    lines << line;
                }
            }

            configFile.close();

            if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&configFile);
                for (const QString &line : lines) {
                    out << line << "\n";
                }
            }
        }
    });

    // * -go-back-
    QPushButton *supportBackButton = new QPushButton("Back", this);
    supportBackButton->setCursor(Qt::PointingHandCursor);
    supportBackButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #007ACC;"
        "   color: white;"
        "   border: none;"
        "   padding: 10px 20px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #005A9E;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004080;"
        "}"
    );

    supportNavButtonLayout->addWidget(supportBackButton);
    supportNavButtonLayout->addStretch();
    supportNavButtonLayout->addWidget(doneButton);

    supportLayout->addLayout(supportNavButtonLayout);

    connect(doneButton, &QPushButton::clicked, this, [stackedWidget]() {
        exit(0);
    });
    connect(supportBackButton, &QPushButton::clicked, this, [stackedWidget]() {
        stackedWidget->setCurrentIndex(5);
    });

    supportPage->setLayout(supportLayout);

}
/// END OF MAIN FUNCTION

// * === BASIC SETUP CONNECTIONS
// void Widget_Initial::basicSetupConnections(QStackedWidget *stackedWidget, QPushButton *flatpakToggleButton) {
//     connect(flatpakToggleButton, &QPushButton::clicked, this, [this]() {
//         //
//     });
// }

Widget_Initial::~Widget_Initial()
{
    // delete ui; // Commented out - no UI file
}

void Widget_Initial::closeEvent(QCloseEvent *event) {
    qDebug() << "closeEvent triggered";
    QMessageBox::StandardButton reply = QMessageBox::question(
      this, "Skip Initial Setup?",
      "Are you sure you want to skip the initial setup?",
      QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Update config file to set initialSetup=1
        QString homeDir = QDir::homePath();
        QString configPath = homeDir + "/tolitica-home-settings/tolitica.conf";
        qDebug() << "Config path: " << configPath;

        QFile file(configPath);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QString content = file.readAll();
            qDebug() << "Original content:" << content;
            content.replace("initialSetup = 0", "initialSetup = 1");
            qDebug() << "Modified content:" << content;
            file.seek(0);
            file.resize(0);
            file.write(content.toUtf8());
            file.close();
            qDebug() << "File written successfully";
        } else {
            qDebug() << "Failed to open file:";
        }
        event->accept();
    } else {
        event->ignore();
    }
}

void Widget_Initial::markSetupComplete() {
    QString homeDir = QDir::homePath();
    QString configPath = homeDir + "/tolitica-home-settings/tolitica.conf";

    QFile file(configPath);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString content = file.readAll();
        content.replace("initialSetup = 0", "initialSetup = 1");
        file.seek(0);
        file.resize(0);
        file.write(content.toUtf8());
        file.close();
    }
}
