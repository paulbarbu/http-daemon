#include "httprequesthandler.h"

HTTPRequestHandler::HTTPRequestHandler()
{
}

HTTPRequestHandler::HTTPRequestHandler(const char *statusLine, const char *body) :
    response(statusLine, body)
{

}

HTTPRequestHandler::HTTPRequestHandler(const QString &statusLine,
                                       const QString &body) :
    response(statusLine, body)
{
}

HTTPRequestHandler::HTTPRequestHandler(const QByteArray &statusLine,
                                       const QByteArray &body="") :
    response(statusLine, body)
{
}

HTTPResponse HTTPRequestHandler::getResponse()
{
    return response;
}
