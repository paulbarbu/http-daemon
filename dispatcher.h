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
    QString pluginRoot; //TODO: boost::program_args

    HTTPRequestHandler *loadPlugin(const QString &plugin, const HTTPRequest &requestData);
};

#endif // DISPATCHER_H
