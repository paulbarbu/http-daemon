#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QByteArray>
#include <QString>
#include <QHash>

class HTTPResponse
{
public:
    HTTPResponse();
    bool setStatusCode(int value); //TODO: replace with an enum
    void setReasonPhrase(const QString &value);
    void setReasonPhrase(const QByteArray &value);
    void setReasonPhrase(const char *value);
    void setBody(const QString &value);
    void setBody(const QByteArray &value);
    void setBody(const char *value);
    void setHeaderField(const QString &key, const QString &value);
    void addHeaderFields(const QHash<QString, QString> &value);
    QByteArray getResponseData();

private:
    QString body;
    QString reasonPhrase;
    int statusCode;
    QHash<QString, QString> fields;
};

#endif // HTTPRESPONSE_H
