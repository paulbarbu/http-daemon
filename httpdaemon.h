#ifndef HTTPDAEMON_H
#define HTTPDAEMON_H

#include <QtNetwork/QTcpServer>

class HTTPDaemon : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPDaemon(const QString &docRoot, const QString &pluginRoot,
                        QObject *parent = 0);

private:
    void incomingConnection(int socketDescriptor);
    const QString docRoot;
    const QString pluginRoot;
};

#endif // HTTPDAEMON_H
