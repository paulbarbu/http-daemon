#include "httpconnectionmanager.h"
#include "httpconnection.h"

HTTPConnectionManager::HTTPConnectionManager(int descriptor,
                                             const QString &docRoot,
                                             const QString &pluginRoot,
                                             QObject *parent) :
    QObject(parent), socketDescriptor(descriptor), docRoot(docRoot),
    pluginRoot(pluginRoot)
{
}

void HTTPConnectionManager::run()
{
    HTTPConnection httpConnection(socketDescriptor, docRoot, pluginRoot);
    httpConnection.start();

    emit connectionClosed();
}
