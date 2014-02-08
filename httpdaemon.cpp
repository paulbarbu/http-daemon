#include <QDebug>
#include <QThread>
#include <QThreadPool>

#include "httpdaemon.h"
#include "httpconnectionmanager.h"

HTTPDaemon::HTTPDaemon(QObject *parent) : QTcpServer(parent)
{
}

void HTTPDaemon::incomingConnection(int socketDescriptor)
{
    HTTPConnectionManager *httpConnectionManager =
            new HTTPConnectionManager(socketDescriptor);

    connect(httpConnectionManager, SIGNAL(connectionClosed()),
            httpConnectionManager, SLOT(deleteLater()));

    httpConnectionManager->setAutoDelete(false);

    QThreadPool::globalInstance()->start(httpConnectionManager);
}
