#include <QtNetwork/QTcpSocket>

#include "httpthread.h"

HTTPThread::HTTPThread(int socketDescriptor, QString docRoot, QObject *parent) :
    QThread(parent), socketDescriptor(socketDescriptor), docRoot(docRoot)
{
}

void HTTPThread::run()
{
    QTcpSocket socket;

    if(!socket.setSocketDescriptor(socketDescriptor)){
        qDebug() << "Setting the sd has failed: " << socket.errorString();
        emit error(socket.error());

        return;
    }

    QString request = read(&socket);
    qDebug() << request;

    parseRequest(request);

    //TODO: process the data
    //TODO: create the response
    //TODO: send the response

    socket.close();
}

QString HTTPThread::read(QTcpSocket *socket){
    QString request;

    do{
        if(!socket->waitForReadyRead()){
            qDebug() << "Error while waiting for client data: "
                     << socket->errorString();
            emit error(socket->error());

            return request;
        }

        request.append(QString(socket->readAll()));
    }while(-1 == request.lastIndexOf("\n\n") &&
           -1 == request.lastIndexOf("\r\n\r\n"));

    return request;
}

QStringList HTTPThread::parseRequest(QString request)
{
    QStringList retval = request.replace("\r", "").split("\n");
    retval.removeAll("");
    //qDebug() << retval;

    return retval;
}
