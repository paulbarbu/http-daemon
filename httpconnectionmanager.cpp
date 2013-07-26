#include "httpconnectionmanager.h"
#include "httpconnection.h"

HTTPConnectionManager::HTTPConnectionManager(int descriptor, QString docRoot, QObject *parent) :
    QObject(parent), socketDescriptor(descriptor), docRoot(docRoot)
{
}

void HTTPConnectionManager::run()
{
    HTTPConnection httpConnection(socketDescriptor, docRoot);
    httpConnection.start();

    emit connectionClosed();
}
