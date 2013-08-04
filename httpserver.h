#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtNetwork/QTcpServer>

class HTTPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPServer(const QString &docRoot, const QString &pluginRoot,
                        QObject *parent = 0);

private:
    void incomingConnection(int socketDescriptor);
    const QString docRoot;
    const QString pluginRoot;
};

#endif // HTTPSERVER_H
