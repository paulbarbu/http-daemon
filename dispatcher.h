#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QHash>

#include "httprequest.h"
#include "httprequesthandler.h"

class Dispatcher
{
public:
    Dispatcher(const HTTPRequest &requestData, const QString &docRoot);
    HTTPRequestHandler getHTTPRequestHandler();
private:
    QHash<QString, QString> dynamicHandlers;
    HTTPRequest requestData;
    const QString docRoot;

    HTTPRequestHandler loadModule(QString module);
};

#endif // DISPATCHER_H
