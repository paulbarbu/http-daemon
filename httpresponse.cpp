#include "httpresponse.h"

HTTPResponse::HTTPResponse() : statusCode(0)
{
}

bool HTTPResponse::setStatusCode(unsigned int value)
{
    if(!isValidStatusCode(value)){
        return false;
    }

    statusCode = value;

    return true;
}

void HTTPResponse::setBody(const QString &value)
{
    body = value;
}

void HTTPResponse::setBody(const char *value)
{
    body = value;
}

void HTTPResponse::appendBody(const QString &value)
{
    body += value;
}

void HTTPResponse::appendBody(const QByteArray &value)
{
    body += value;
}

void HTTPResponse::appendBody(const char *value)
{
    body += value;
}

void HTTPResponse::setBody(const QByteArray &value)
{
    body = value;
}

QByteArray HTTPResponse::get()
{
    //TODO: memoize the response until a set*() method is called

    if(0 == statusCode || "" == reasonPhrase){
        return "";
    }

    QString response;
    QString statusLine = "HTTP/1.0 %1 %2\r\n";
    QString headerPattern = "%1: %2\r\n";

    response = statusLine.arg(QString::number(statusCode), reasonPhrase);

    QHash<QString, QString>::const_iterator i;
    for(i = fields.constBegin(); i != fields.constEnd(); ++i){
        response += headerPattern.arg(i.key(), i.value());
    }

    response += "\r\n" + body;

    return response.toUtf8();
}

QByteArray HTTPResponse::getPartial()
{
    QByteArray response = get();

    if( "" == response){
        response += body;
    }

    statusCode = 0;
    reasonPhrase = "";
    body = "";

    return response;
}

bool HTTPResponse::isValidStatusCode(unsigned int value)
{
    int firstDigit = value / 100;
    if(firstDigit >= 1 && firstDigit <= 5){
        return true;
    }

    return false;
}

void HTTPResponse::setReasonPhrase(const QString &value)
{
    reasonPhrase = value;
}

void HTTPResponse::setReasonPhrase(const char *value)
{
    reasonPhrase = value;
}

void HTTPResponse::setHeaderField(const QString &key, const QString &value)
{
    fields.insert(key, value);
}

void HTTPResponse::addHeaderFields(const QHash<QString, QString> &value)
{
    QHash<QString, QString>::const_iterator i;

    for(i = value.constBegin(); i != value.constEnd(); ++i){
        setHeaderField(i.key(), i.value());
    }
}

void HTTPResponse::setReasonPhrase(const QByteArray &value)
{
    reasonPhrase = value;
}

