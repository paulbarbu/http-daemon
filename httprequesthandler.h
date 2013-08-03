#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include <QString>
#include <QByteArray>
#include <string>

#include "httpresponse.h"

class HTTPRequestHandler
{
public:
    HTTPRequestHandler();
    HTTPRequestHandler(const char *statusLine, const char *body="");
    HTTPRequestHandler(const QString &statusLine, const QString &body="");
    HTTPRequestHandler(const QByteArray &statusLine, const QByteArray &body="");
    virtual HTTPResponse getResponse(); //TODO: rename this

private:
    HTTPResponse response;
};

#endif // HTTPREQUESTHANDLER_H
