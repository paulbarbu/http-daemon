#include <QDebug>
#include <QThread>

#include "httpserver.h"
#include "httpconnection.h"

HTTPServer::HTTPServer(const QString &docRoot, QObject *parent) :
    QTcpServer(parent), docRoot(docRoot)
{
    //TODO: check the network and the files owned by my program to see what
    //QTcpServer does
}

void HTTPServer::incomingConnection(int socketDescriptor)
{
    //TODO: QThreadPool or QtConcurrent
    QThread *t = new QThread;

    HTTPConnection *httpConnection = new HTTPConnection(socketDescriptor,
                                                        docRoot);

    httpConnection->moveToThread(t);

    connect(t, SIGNAL(started()), httpConnection, SLOT(start()));
    connect(httpConnection, SIGNAL(closed()), t, SLOT(quit()));
    connect(t, SIGNAL(finished()), httpConnection, SLOT(deleteLater()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    t->start();
}
