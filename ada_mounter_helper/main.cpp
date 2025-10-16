#include <QCoreApplication>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDebug>
#include "ada_mounter_helper.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Get the configuration file path from the helper and its directory.
    QString configPath = AdaMounterHelper::configPath();
    QFileInfo configInfo(configPath);
    QString configDir = configInfo.absolutePath();

    // Set up a file system watcher on the directory.
    QFileSystemWatcher watcher;
    watcher.addPath(configDir);
    qDebug() << "Watching directory:" << configDir;

    // Connect the directoryChanged signal to re-enforce mounts/unmounts.
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged,
                     [&watcher, configPath](const QString &changedDir) {
                         Q_UNUSED(changedDir);
                         qDebug() << "Directory changed. Checking configuration file:" << configPath;

                         // If the configuration file isn't being watched (for example, if it was replaced),
                         // add it back to the watch list.
                         if (!watcher.files().contains(configPath)) {
                             watcher.addPath(configPath);
                             qDebug() << "Re-added config file to watcher.";
                         }

                         AdaMounterHelper::enforceMounts();
                         AdaMounterHelper::enforceUnmounts();
                     });

    // Enforce the mount/unmount state at startup.
    AdaMounterHelper::enforceMounts();
    AdaMounterHelper::enforceUnmounts();

    return a.exec();
}
