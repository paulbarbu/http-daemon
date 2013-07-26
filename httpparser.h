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
};

class HTTPParser : public QObject
{
    Q_OBJECT
public:
    HTTPParser(QObject *parent=0);

    HTTPParser &operator<<(const QString &chunk);
    HTTPParser &operator<<(const QByteArray &chunk);

signals:
    void parsed(const RequestData &requestData);
    void parseError(const QString &reason);

private:
    int bytesToParse;
    QByteArray data;
    bool isParsedHeader;
    RequestData requestData;

    //TODO: think if I can use these methods to parse data without using the streams
    void parsePostData();
    void parseRequestHeader();
    void parse();

    void discardRequestHeader();
};

#endif // HTTPPARSER_H
