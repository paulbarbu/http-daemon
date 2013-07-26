#include <QDebug>
#include <QThread>
#include <QThreadPool>

#include "httpserver.h"
#include "httpconnectionmanager.h"

HTTPServer::HTTPServer(const QString &docRoot, QObject *parent) :
    QTcpServer(parent), docRoot(docRoot)
{
    //TODO: check the network and the files owned by my program to see what
    //QTcpServer does
}

//TODO: convert everything to C++11
void HTTPServer::incomingConnection(int socketDescriptor)
{
    HTTPConnectionManager *httpConnectionManager =
            new HTTPConnectionManager(socketDescriptor, docRoot);

    connect(httpConnectionManager, SIGNAL(connectionClosed()),
            httpConnectionManager, SLOT(del()));

    httpConnectionManager->setAutoDelete(false);

    QThreadPool::globalInstance()->start(httpConnectionManager);
}
