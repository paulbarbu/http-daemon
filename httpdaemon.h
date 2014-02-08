#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QtNetwork/QTcpServer>

class HTTPDaemon : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPDaemon(QObject *parent = 0);

private:
    void incomingConnection(int socketDescriptor);
};

#endif // HTTPDAEMON_H
