#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QThread>

#include "httpconnection.h"
#include "dispatcher.h"

HTTPConnection::HTTPConnection(int socketDescriptor, QObject *parent) :
    QObject(parent), socket(this), eventLoop(this)
{
    if(!socket.setSocketDescriptor(socketDescriptor)){
        qDebug() << socket.errorString() << "Cannot set sd: " << socketDescriptor;
        emit closed();
    }
}

void HTTPConnection::close()
{
    socket.waitForBytesWritten(500);

    socket.disconnectFromHost();
    if (socket.state() == QAbstractSocket::UnconnectedState || socket.waitForDisconnected(1000)){
            qDebug("Disconnected!");
    }
    emit closed();
}

void HTTPConnection::start()
{
    connect(&socket, SIGNAL(readyRead()), this, SLOT(read()));

    connect(&parser, SIGNAL(parsed(HTTPRequest)), this,
            SLOT(processRequestData(HTTPRequest)));

    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onError(QAbstractSocket::SocketError)));

    connect(&parser, SIGNAL(parseError(QString)), this,
            SLOT(onParseError(QString)));

    connect(this, SIGNAL(closed()), &eventLoop, SLOT(quit()));

    eventLoop.exec();
}

void HTTPConnection::read()
{
    parser << socket.readAll();
}

void HTTPConnection::write(HTTPResponse response)
{
    QByteArray partialResponse = response.getPartial();

    qDebug() << "Writing" << partialResponse.size() << "bytes";

    socket.write(partialResponse);
}

void HTTPConnection::processRequestData(HTTPRequest requestData)
{
    requestData.remoteAddress = socket.peerAddress();

    if(socket.localPort() != requestData.port){
        requestData.port = socket.localPort();
    }

    qDebug() << "Request data:\n\tMethod:"
             << requestData.method << "\n\tUrl:"
             << requestData.url << "\n\tProtocol:"
             << requestData.protocol << "\n\tVer:"
             << requestData.protocolVersion << "\n\tHost:"
             << requestData.host << "\n\tPort:"
             << requestData.port << "\n\tRemote addr:"
             << requestData.remoteAddress << "\n\tCookies:"
             << requestData.cookieJar
             << "\n\tFields:" << requestData.fields
             << "\n\tContent-Length:" << requestData.contentLength
             << "\n\tpost:" << requestData.postData;

    //TODO: integrate FastCGI
    //TOOD: implement other methods

    if("GET" != requestData.method && "POST" != requestData.method){
        HTTPResponse response;
        response.setStatusCode(501);
        response.setReasonPhrase("Not Implemented");

        write(response);
        close();
        return;
    }

    Dispatcher dispatcher;

    HTTPRequestHandler *requestHandler =
            dispatcher.getHTTPRequestHandler(requestData.url.path());

    if(NULL == requestHandler){
        HTTPResponse response;
        response.setStatusCode(500);
        response.setReasonPhrase("Internal Server Error");

        write(response);
        close();
        return;
    }

    connect(requestHandler, SIGNAL(responseWritten(HTTPResponse)), this,
            SLOT(write(HTTPResponse)));

    connect(requestHandler, SIGNAL(endOfWriting()), this,
            SLOT(close()));

    requestHandler->createResponse(requestData);
}

void HTTPConnection::onParseError(const QString &reason)
{
    HTTPResponse response;
    response.setStatusCode(400);
    response.setReasonPhrase("Bad Request");
    response.setBody(reason);

    write(response);

    close();
}

void HTTPConnection::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError << ":" << socket.errorString();
    close();
}
