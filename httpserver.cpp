#include <QDebug>

#include "httpserver.h"
#include "httpthread.h"

HTTPServer::HTTPServer(const QString &docRoot, QObject *parent) :
    QTcpServer(parent), docRoot(docRoot)
{
    //TODO: check the network and the files owned by my program to see what QTcpServer does
}

void HTTPServer::incomingConnection(int socketDescriptor){
    qDebug() << "Incoming connection, sd: " << socketDescriptor;

    //TODO: thread pool
    /*https://qt-project.org/doc/qt-4.8/qthread.html#details
     *
     *HTTPThread becomes HTTPConnection and is movedToThread() because:
     *It is important to remember that a QThread object usually lives in the
     *thread where it was created, not in the thread that it manages.
     *This oft-overlooked detail means that a QThread's slots will be executed
     *in the context of its home thread, not in the context of the thread it is
     *managing. For this reason, implementing new slots in a QThread subclass is
     *error-prone and discouraged.
     */

    HTTPThread *t = new HTTPThread(socketDescriptor, docRoot, this);
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));

    t->start();
}
