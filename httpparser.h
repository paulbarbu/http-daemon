#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <QHash>
#include <QString>
#include <QUrl>
#include <QStringList>

struct RequestData
{
    QString method;
    QUrl url;
    QString protocol;
    double protocolVersion;
    int contentLength;

    QHash<QString, QStringList> fields;
    QHash<QString, QString> postData;

    bool valid;
};

class HTTPParser
{
public:
    HTTPParser();
    QHash<QString, QString> parsePostBody(const QString &postBody);
    RequestData parseRequestHeader(const QString &req);
};

#endif // HTTPPARSER_H
