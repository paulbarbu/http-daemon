#include <QDebug>

#include "httpserver.h"

HTTPServer::HTTPServer(QObject *parent) :
    QTcpServer(parent)
{
}

void HTTPServer::incomingConnection(int socketDescriptor){
    qDebug() << "Incomoing connection, sd: " << socketDescriptor;
}
