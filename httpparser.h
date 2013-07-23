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

    QHash<QString, QStringList> fields;
    QHash<QString, QString> postData;

    bool valid;
};

class HTTPParser
{
public:
    HTTPParser();
    QHash<QString, QString> parsePostBody(QString postBody);
    RequestData parseRequest(QString request);

    void setRequest(const QString &value);
};

#endif // HTTPPARSER_H
