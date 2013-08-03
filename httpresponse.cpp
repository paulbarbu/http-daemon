#include "httpresponse.h"

HTTPResponse::HTTPResponse() : response("HTTP/1.0 %1\r\n\r\n%2")
{
}

HTTPResponse::HTTPResponse(const QString &statusLine, const QString &body)
{
    response = response.arg(statusLine, body);
}

HTTPResponse::HTTPResponse(const QByteArray &statusLine, const QByteArray &body)
{
    response = response.arg(statusLine, body);
}
