#ifndef HTTPCONNECTIONMANAGER_H
#define HTTPCONNECTIONMANAGER_H

#include <QObject>
#include <QRunnable>

class HTTPConnectionManager : public QObject, public QRunnable
{
    Q_OBJECT
public:
    HTTPConnectionManager(int descriptor, const QString &docRoot,
                          const QString &pluginRoot, QObject *parent=0);
    void run();

signals:
    void connectionClosed();

private:
    int socketDescriptor;
    const QString docRoot;
    const QString pluginRoot;
};

#endif // HTTPCONNECTIONMANAGER_H
