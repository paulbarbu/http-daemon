#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <QtNetwork/QTcpSocket>
#include <QEventLoop>

#include "httpparser.h"
#include "httpresponse.h"

class HTTPConnection : public QObject
{
    Q_OBJECT
public:
    explicit HTTPConnection(int socketDescriptor, const QString &docRoot,
                        QObject *parent=0);

private:
    QTcpSocket socket;
    HTTPParser parser;
    QEventLoop eventLoop;

    const QString docRoot;

    QByteArray square(const QByteArray &partialResponse,
                      const HTTPRequest &requestData);
    QByteArray login(const QByteArray &partialResponse,
                     const HTTPRequest &requestData);
    QByteArray check(const QByteArray &partialResponse,
                     const HTTPRequest &requestData);

signals:
    void closed();

private slots:
    void onParseError(const QString &reason);

public slots:
    void start();
    void onError(QAbstractSocket::SocketError socketError);
    void processRequestData(const HTTPRequest &requestData);
    void read();
    void write(HTTPResponse response);
    void close();
};

#endif // HTTPTHREAD_H
