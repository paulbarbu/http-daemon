#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QHash>

#include "httprequest.h"
#include "httprequesthandler.h"

class Dispatcher
{
public:
    Dispatcher(const QString &docRoot,const QString &pluginRoot);
    HTTPRequestHandler *getHTTPRequestHandler(const HTTPRequest &requestData) const;
private:
    QHash<QString, QString> dynamicHandlers;
    const QString docRoot;
    const QString pluginRoot;

    HTTPRequestHandler *loadPlugin(const QString &plugin, const HTTPRequest &requestData) const;
};

#endif // DISPATCHER_H
