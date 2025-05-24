#include "connectivityChecker.h"
#include <QtNetwork/QNetworkRequest>
#include <QUrl>

ConnectivityChecker::ConnectivityChecker(QObject *parent)
    : QObject(parent)
{
    // QNetworkAcessManager automatically uses the proper Qt include paths.
}

void ConnectivityChecker::checkConnectivity() {
    // Use a HEAD request to a reliable endpoint
    QNetworkRequest request(QUrl("https://www.archlinux.org"));
    QNetworkReply *reply = m_manager.head(request);

    // Connect the finished signal to our slot.
    connect(reply, &QNetworkReply::finished, this, &ConnectivityChecker::onReplyFinished);
}

void ConnectivityChecker::onReplyFinished() {
    // 'sender()' returns the QNetworkReply triggered.
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        bool isConnected = (reply && reply->error() == QNetworkReply::NoError);
        emit connectivityChecked(isConnected);
        reply->deleteLater();
    } else {
        emit connectivityChecked(false);
    }
}
