#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QThread>

#include "httpconnection.h"
#include "dispatcher.h"

HTTPConnection::HTTPConnection(int socketDescriptor, const QString &docRoot,
                               const QString &pluginRoot, QObject *parent) :
    QObject(parent), socket(this), eventLoop(this), docRoot(docRoot),
    pluginRoot(pluginRoot)
{
    if(!socket.setSocketDescriptor(socketDescriptor)){
        qDebug() << socket.errorString() << "Cannot set sd: " << socketDescriptor;
        emit closed();
    }
}

void HTTPConnection::close()
{
    socket.disconnectFromHost();
    socket.waitForDisconnected(1000);
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

void HTTPConnection::read(){
    parser << socket.readAll();
}

void HTTPConnection::write(HTTPResponse &response)
{
    QByteArray partialResponse = response.getPartial();

    socket.write(partialResponse, partialResponse.size());
}

void HTTPConnection::processRequestData(const HTTPRequest &requestData)
{
    qDebug() << "Request data:\n\tMethod:"
             << requestData.method << "\n\tUrl:"
             << requestData.url << "\n\tProtocol:"
             << requestData.protocol << "\n\tVer:"
             <<requestData.protocolVersion
             << "\n\tFields:" << requestData.fields
             << "\n\tContent-Length:" << requestData.contentLength
             << "\n\tpost:" << requestData.postData;

    //TODO: integrate FastCGI

    if("GET" != requestData.method && "POST" != requestData.method){
        HTTPResponse response;
        response.setStatusCode(501);
        response.setReasonPhrase("Not Implemented");

        write(response);
        close();
        return;
    }

    Dispatcher dispatcher(docRoot, pluginRoot);

    HTTPRequestHandler *requestHandler =
            dispatcher.getHTTPRequestHandler(requestData);

    if(NULL == requestHandler){
        HTTPResponse response;
        response.setStatusCode(500);
        response.setReasonPhrase("Internal Server Error");

        write(response);
        close();
        return;
    }

    connect(requestHandler, SIGNAL(responseWritten(HTTPResponse&)), this,
            SLOT(write(HTTPResponse&)));

    connect(requestHandler, SIGNAL(endOfWriting()), this,
            SLOT(close()));

    connect(requestHandler, SIGNAL(endOfWriting()), requestHandler,
            SLOT(deleteLater()));

    requestHandler->createResponse();
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
