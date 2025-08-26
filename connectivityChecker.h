#ifndef CONNECTIVITYCHECKER_H
#define CONNECTIVITYCHECKER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class ConnectivityChecker : public QObject {
    Q_OBJECT
public:
    explicit ConnectivityChecker(QObject *parent = nullptr);
    void checkConnectivity();

signals:
    void connectivityChecked(bool isConnected);

private slots:
    void onReplyFinished();

private:
    QNetworkAccessManager m_manager;
};

#endif // CONNECTIVITYCHECKER_H
