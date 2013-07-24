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
    QString request;
    QTcpSocket socket;
    HTTPParser parser;
    RequestData requestData;
    bool isParsedHeader;
    int bytesToParse;

    const int socketDescriptor;
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
    void error(QAbstractSocket::SocketError socketError);
    void requestParsed(const RequestData &requestData);
public slots:
    void onError(QAbstractSocket::SocketError socketError);
    void onRequestParsed(const RequestData &requestData);
    void read();
};

#endif // HTTPTHREAD_H
