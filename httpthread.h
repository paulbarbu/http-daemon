#ifndef HTTPTHREAD_H
#define HTTPTHREAD_H

#include <QThread>
#include <QtNetwork/QTcpSocket>
#include <QStringList>

class HTTPThread : public QThread
{
    Q_OBJECT
public:
    explicit HTTPThread(int socketDescriptor, QString docRoot, QObject *parent=0);
    void run();

private:
    int socketDescriptor;
    QString docRoot;
    QString read(QTcpSocket *socket);
    QStringList parseRequest(QString request);

signals:
    void error(QAbstractSocket::SocketError socketError);
public slots:
};

#endif // HTTPTHREAD_H
