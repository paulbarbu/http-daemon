#ifndef HTTPCONNECTIONMANAGER_H
#define HTTPCONNECTIONMANAGER_H

#include <QObject>
#include <QRunnable>
#include <QEventLoop>

class HTTPConnectionManager : public QObject, public QRunnable
{
    Q_OBJECT
public:
    HTTPConnectionManager(int descriptor, QString docRoot, QObject *parent=0);
    void run();

signals:
    void connectionClosed();

private:
    int socketDescriptor;
    QString docRoot;
};

#endif // HTTPCONNECTIONMANAGER_H
