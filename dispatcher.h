#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QHash>

#include "httprequest.h"
#include "httprequesthandler.h"

class Dispatcher
{
public:
    Dispatcher(const QString &docRoot);
    HTTPRequestHandler *getHTTPRequestHandler(const HTTPRequest &requestData);
private:
    QHash<QString, QString> dynamicHandlers;
    const QString docRoot;

    HTTPRequestHandler *loadModule(QString module);
};

#endif // DISPATCHER_H
