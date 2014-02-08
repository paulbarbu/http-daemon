#ifndef HTTPCONNECTIONMANAGER_H
#define HTTPCONNECTIONMANAGER_H

#include <QObject>
#include <QRunnable>

class HTTPConnectionManager : public QObject, public QRunnable
{
    Q_OBJECT
public:
    HTTPConnectionManager(int descriptor, QObject *parent=0);
    void run();

signals:
    void connectionClosed();

private:
    const int socketDescriptor;
};

#endif // HTTPCONNECTIONMANAGER_H
