#ifndef HTTPTHREAD_H
#define HTTPTHREAD_H

#include <QThread>
#include <QtNetwork/QTcpSocket>

#include "httpparser.h"

class HTTPThread : public QThread
{
    Q_OBJECT
public:
    explicit HTTPThread(const int socketDescriptor, const QString &docRoot,
                        QObject *parent=0);
    void run();

private:
    HTTPParser parser;

    const int socketDescriptor;
    const QString docRoot;
    const QString responseStatusLine;

    QString readRequest(QTcpSocket *socket);
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
    void error(QAbstractSocket::SocketError socketError);
public slots:
};

#endif // HTTPTHREAD_H
