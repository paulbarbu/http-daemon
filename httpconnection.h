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
    QTcpSocket socket;
    HTTPParser parser;
    HTTPRequest requestData;
    QEventLoop eventLoop;

    const QString docRoot;
    const QString responseStatusLine;

    void close();
    QByteArray processRequestData(const HTTPRequest &requestData);
    QByteArray serveStaticFile(const QByteArray &partialResponse,
                               const QString &filePath);
    QByteArray serveStaticDir(const QByteArray &partialResponse,
                              const QString &dirPath);
    QByteArray square(const QByteArray &partialResponse,
                      const HTTPRequest &requestData);
    QByteArray login(const QByteArray &partialResponse,
                     const HTTPRequest &requestData);
    QByteArray check(const QByteArray &partialResponse,
                     const HTTPRequest &requestData);

signals:
    void closed();
    void requestParsed(const HTTPRequest &requestData);

private slots:
    void onParseError(const QString &reason);

public slots:
    void start();
    void onError(QAbstractSocket::SocketError socketError);
    void onRequestParsed(const HTTPRequest &requestData);
    void read();
};

#endif // HTTPTHREAD_H
