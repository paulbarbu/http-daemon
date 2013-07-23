#include <QDebug>

#include "httpserver.h"
#include "httpthread.h"

HTTPServer::HTTPServer(const QString &docRoot, QObject *parent) :
    QTcpServer(parent), docRoot(docRoot)
{
    //TODO: check the network and the files owned by my program to see what QTcpServer does
}

void HTTPServer::incomingConnection(int socketDescriptor){
    qDebug() << "Incoming connection, sd: " << socketDescriptor;

    HTTPThread *t = new HTTPThread(socketDescriptor, docRoot, this);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    t->start();
}
