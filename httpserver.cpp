#include <QDebug>

#include "httpserver.h"
#include "httpthread.h"

HTTPServer::HTTPServer(QObject *parent) :
    QTcpServer(parent)
{
}

void HTTPServer::incomingConnection(int socketDescriptor){
    qDebug() << "Incomoing connection, sd: " << socketDescriptor;

    HTTPThread *t = new HTTPThread(socketDescriptor, this);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    t->start();
}
