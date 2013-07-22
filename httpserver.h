#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtNetwork/QTcpServer>



class HTTPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPServer(QObject *parent = 0);
    
signals:
    
public slots:

private:
    void incomingConnection(int socketDescriptor);
};

#endif // HTTPSERVER_H
