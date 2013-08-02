#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QHash>
#include <QUrl>
#include <QString>
#include <QStringList>

struct HTTPRequest
{
    QString method;
    QUrl url;
    QString protocol;
    double protocolVersion;
    int contentLength;

    QHash<QString, QStringList> fields;
    QHash<QString, QString> postData;
};

#endif // HTTPREQUEST_H
