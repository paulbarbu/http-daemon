#include <QDebug>

#include "httpserver.h"
#include "httpthread.h"

HTTPServer::HTTPServer(const QString &docRoot, QObject *parent) :
    QTcpServer(parent), docRoot(docRoot)
{
}

void HTTPServer::incomingConnection(int socketDescriptor){
    qDebug() << "Incoming connection, sd: " << socketDescriptor;

    HTTPThread *t = new HTTPThread(socketDescriptor, docRoot, this);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    t->start();
}
