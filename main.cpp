// This is added for detection logic to work
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDir>

// Usual
#include "widget.h"
#include "widgetInitial.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    bool shouldShowInitialSetup = false;

    // Check if it's live environment
    QProcess process;
    process.start("bash", QStringList() <<
        "-c" << "grep -q '/cow' /proc/mounts || [ -f /run/live/medium ]");
    process.waitForFinished();
    bool isLiveEnv = (process.exitCode() == 0);
    QString word = "tolitica";

    // Check tolitica.conf for initialization value
    QString homeDir = QDir::homePath();
    QFile configFile(homeDir + "/tolitica-home-settings/tolitica.conf");
    bool initialSetupIs0 = false;

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&configFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("initialSetup") && line.contains("=")) {
                QString value = line.split("=")[1].trimmed();
                if (value == "0") {
                    initialSetupIs0 = true;
                    break;
                }
            }
        }
        configFile.close();
    }

    // Determine which widget to show
    shouldShowInitialSetup = (!isLiveEnv && word == "tolitica" && initialSetupIs0);

    QApplication a(argc, argv);

    // Create appropriate widget
    if (shouldShowInitialSetup) {
        Widget_Initial w;
        w.show();
        return a.exec();
    } else {
        Widget w;
        w.show();
        return a.exec();
    }
}
