#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QtNetwork/QTcpSocket>
#include <QEventLoop>

#include "httpparser.h"

class HTTPConnection : public QObject
{
    Q_OBJECT
public:
    explicit HTTPConnection(int socketDescriptor, const QString &docRoot,
                        QObject *parent=0);

private:
    QString request;
    QTcpSocket socket;
    HTTPParser parser;
    RequestData requestData;
    bool isParsedHeader;
    int bytesToParse;
    QEventLoop eventLoop;

    const QString docRoot;
    const QString responseStatusLine;

    QByteArray processRequestData(const RequestData &requestData);
    QByteArray serveStaticFile(const QByteArray &partialResponse,
                               const QString &filePath);
    QByteArray serveStaticDir(const QByteArray &partialResponse,
                              const QString &dirPath);
    QByteArray square(const QByteArray &partialResponse,
                      const RequestData &requestData);
    QByteArray login(const QByteArray &partialResponse,
                     const RequestData &requestData);
    QByteArray check(const QByteArray &partialResponse,
                     const RequestData &requestData);

signals:
    void closed();
    void requestParsed(const RequestData &requestData);
public slots:
    void start();
    void onError(QAbstractSocket::SocketError socketError);
    void onRequestParsed(const RequestData &requestData);
    void read();
};

#endif // HTTPTHREAD_H
