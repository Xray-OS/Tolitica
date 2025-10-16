#ifndef CORE_INITIAL_H
#define CORE_INITIAL_H

#include <QObject>
#include <QString>
#include <QMessageBox>

class CoreInitial : public QObject
{
    Q_OBJECT

public:
    explicit CoreInitial(QObject *parent = nullptr);

public slots:
    void applyGlobalTheme(const QString &themeId);
    bool osreleaseStatus();
    void setOSrelease();
    bool konsoleProfStatus();
    void setKonsoleProfile();
    void reloadPlasmaByReplace();
    void reloadPlasmaByDBus();
    bool themeStatus();
    bool xrayThemeStatus();
    bool grubThemeStatus();
    void setGrubTheme();
    QString currentIcons();
    void setIcons(const QString &icons);
    bool aurStatus(const QString &aurHelper);
    void getRemoveAUR(QWidget *parent, const QString &aurHelper,
        std::function<void(bool)> callback = nullptr);
    bool storeStatus(const QString &store);
    void getRemoveStore(QWidget *parent, const QString &store,
        std::function<void(bool)> callback = nullptr);
    bool gamingMetaStatus();
    void getArch7zGamingMeta(QWidget *parent,
        std::function<void(bool)> callback = nullptr);

signals:
    void themeApplied(const QString &themeId);
    void reloadFinished();
};

#endif
