#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QByteArray>
#include <QString>

class HTTPResponse
{
public:
    HTTPResponse();
    HTTPResponse(const QString &statusLine, const QString &body="");
    HTTPResponse(const QByteArray &statusLine, const QByteArray &body="");

private:
    QString response;
};

#endif // HTTPRESPONSE_H
