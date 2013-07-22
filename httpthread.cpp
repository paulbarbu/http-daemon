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

    QByteArray response = processRequestData(parseRequest(request)).toUtf8();

    socket.write(response, response.size());

    if(!socket.waitForBytesWritten()){
        qDebug() << "Could not write byts to socket: " << socket.errorString();
        emit error(socket.error());
    }

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

QHash<QString, QStringList> HTTPThread::parseRequest(QString request)
{
    QHash<QString, QStringList> requestData;

    QStringList lines = request.replace("\r", "")
            .split("\n", QString::SkipEmptyParts);

    if(0 == lines.size()){
        return requestData;
    }

    QStringList statusLine = lines[0].split(" ");

    if(3 != statusLine.size()){
        return requestData;
    }

    QStringList protocol = statusLine[2].split("/");
    bool ok;

    double ver = protocol[1].toDouble(&ok);

    if(2 != protocol.size() || "HTTP" != protocol[0] ||
            !ok || ver < 0.9 || ver > 1.1){
        return requestData;
    }

    statusLine.removeAt(2);
    requestData["status-line"] = statusLine + protocol;

    lines.removeAt(0);
    int spacePos;
    foreach(QString line, lines){
        spacePos = line.indexOf(" ");

        requestData.insert(line.left(spacePos-1),
                           QStringList(line.right(line.size()-spacePos-1)));
    }

    if(requestData.contains("Host")){
        requestData["Host"] = requestData["Host"][0].split(":");
    }

    //qDebug() << requestData;
    return requestData;
}

QString HTTPThread::processRequestData(QHash<QString, QStringList> requestData)
{
    //TODO: add support for different Host values

    //TODO: parse the query_string
    QString response("HTTP/1.0 200 OK\r\n\r\ndummy text");

    if(requestData.isEmpty()){
        response = "HTTP/1.0 400 Bad request\r\n";
        return response;
    }

    QString method = requestData["status-line"][0];

    if("GET" == method){
        qDebug() << "GET from " << docRoot << requestData["status-line"][1];
    }
    else if("POST" == method){
        qDebug() << "POST!";
    }
    else{
        response = "HTTP/1.0 501 Not Implemented\r\n";
        qDebug() << "Unsupported HTTP method!";
    }

    return response;
}
