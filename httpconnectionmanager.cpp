#include "httpconnectionmanager.h"
#include "httpconnection.h"

HTTPConnectionManager::HTTPConnectionManager(int descriptor, QObject *parent) :
    QObject(parent), socketDescriptor(descriptor)
{
}

void HTTPConnectionManager::run()
{
    HTTPConnection httpConnection(socketDescriptor);
    httpConnection.start();

    emit connectionClosed();
}
