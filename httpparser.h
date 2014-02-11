#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <QString>
#include <QStringList>

#include <httprequest.h>

class HTTPParser : public QObject
{
    Q_OBJECT
public:
    HTTPParser(QObject *parent=0);

    HTTPParser &operator<<(const QString &chunk);
    HTTPParser &operator<<(const QByteArray &chunk);

signals:
    void parsed(HTTPRequest requestData);
    void parseError(const QString &reason);

private:
    int bytesToParse;
    QByteArray data;
    bool isParsedHeader;
    HTTPRequest requestData;

    void parsePostData();
    void parseRequestHeader();
    void parse();

    void discardRequestHeader();
};

#endif // HTTPPARSER_H
