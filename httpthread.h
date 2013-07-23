#ifndef HTTPTHREAD_H
#define HTTPTHREAD_H

#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QStringList>

class HTTPThread : public QThread
{
    Q_OBJECT
public:
    explicit HTTPThread(const int socketDescriptor, const QString &docRoot,
                        QObject *parent=0);
    void run();

private:
    const int socketDescriptor;
    const QString docRoot;

    const QString responseStatusLine;

    QString read(QTcpSocket *socket);
    QHash<QString, QStringList> parseRequest(QString request);
    QByteArray processRequestData(const QHash<QString, QStringList> &requestData);
    QByteArray serveStaticFile(const QByteArray &partialResponse,
                               const QString &filePath);
    QByteArray serveStaticDir(const QByteArray &partialResponse,
                              const QString &dirPath);

signals:
    void error(QAbstractSocket::SocketError socketError);
public slots:
};

#endif // HTTPTHREAD_H
