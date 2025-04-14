#include "widget.h"
#include "./ui_widget.h"
#include <QLabel>
//#include <QPixmap>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Tolitica Ada Assist");
    resize(800,600);
    setWindowIcon(QIcon(":/Icons/Icons/tolitica-icon.png"));


    // create a label for the logo
    QLabel *adaLogo = new QLabel(this);
    QPixmap adaPixmap(":/Logos/Logos/ada-logo-with-white.png");
    adaLogo->setPixmap(adaPixmap.scaled(200, 150));

    // Create a vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // Remove margins

    // Add the logo to the layout with some spacing
    mainLayout->addWidget(adaLogo, 0, Qt::AlignCenter); // Center the logo in the layout
    mainLayout->addStretch(); // Allow the layout to expand

    // Set the layout for the main widget
    setLayout(mainLayout);
}

Widget::~Widget()
{
    delete ui;
}
