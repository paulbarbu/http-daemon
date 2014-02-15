#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <QHash>

#include "httprequest.h"
#include "httprequesthandler.h"

class Dispatcher
{
public:
    Dispatcher();
    HTTPRequestHandler *getHTTPRequestHandler(const QString &path) const;
};

#endif // DISPATCHER_H
