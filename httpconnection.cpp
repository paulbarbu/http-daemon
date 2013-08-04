#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QThread>

#include "httpconnection.h"
#include "dispatcher.h"

HTTPConnection::HTTPConnection(int socketDescriptor, const QString &docRoot,
                       QObject *parent) : QObject(parent), socket(this),
    eventLoop(this), docRoot(docRoot)
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
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onError(QAbstractSocket::SocketError)));

    connect(&socket, SIGNAL(readyRead()), this, SLOT(read()));

    connect(this, SIGNAL(closed()), &eventLoop, SLOT(quit()));

    connect(&parser, SIGNAL(parseError(QString)), this,
            SLOT(onParseError(QString)));

    connect(&parser, SIGNAL(parsed(HTTPRequest)), this,
            SLOT(processRequestData(HTTPRequest)));

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

    Dispatcher dispatcher(docRoot);

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

    connect(requestHandler, SIGNAL(endOfWriting()), requestHandler,
            SLOT(deleteLater()));

    connect(requestHandler, SIGNAL(endOfWriting()), this,
            SLOT(close()));

    connect(requestHandler, SIGNAL(responseWritten(HTTPResponse&)), this,
            SLOT(write(HTTPResponse&)));

    requestHandler->createResponse();

//        else if("/patrat" == requestData.url.path()){
//            response = square(response, requestData);
//        }
//        else if("/login" == requestData.url.path()){
//            response = login(response, requestData);
//        }
//        else if("/verifica" == requestData.url.path()){
//            response = check(response, requestData);
//        }
}

//QByteArray HTTPConnection::square(const QByteArray &partialResponse,
//                              const HTTPRequest &requestData)
//{
//    if("GET" != requestData.method || !requestData.url.hasQueryItem("a")){
//        return responseStatusLine.arg("400 Bad Request\r\n").toAscii();
//    }

//    QString numToSquare = requestData.url.queryItemValue("a");
//    QString body;

//    bool ok;
//    double n = numToSquare.toDouble(&ok);

//    if(!ok){
//        body = "a-ul trebuie sa fie numar!\n";
//    }
//    else{
//        body = numToSquare + "^2 = " + QString::number(n*n) + "\n";
//    }

//    return partialResponse + "\r\n" + body.toAscii();
//}

//QByteArray HTTPConnection::login(const QByteArray &partialResponse,
//                             const HTTPRequest &requestData)
//{
//    QString page = "\r\n<html><body>"
//            "<form method=\"POST\">"
//            "%1"
//            "Username: <input type=\"text\" name=\"username\">"
//            "Password: <input type=\"password\" name=\"pass\">"
//            "<INPUT type=\"submit\" value=\"Auth\">"
//            "</form></body></html>";

//    if("GET" == requestData.method){
//        return partialResponse + page.arg("").toAscii();
//    }

//    if("POST" == requestData.method && !requestData.postData.isEmpty()){
//        if(requestData.postData.contains("username") &&
//                "Ion" == requestData.postData["username"] &&
//                requestData.postData.contains("pass") &&
//                "1234" == requestData.postData["pass"]){

//            return partialResponse + "Set-Cookie: loggedin=1\r\n\r\nYou're logged in!";
//        }

//        return partialResponse +
//                page.arg("Login failed, try again!<br>").toAscii();
//    }

//    return responseStatusLine.arg("400 Bad request\r\n").toAscii();
//}

//QByteArray HTTPConnection::check(const QByteArray &partialResponse,
//                             const HTTPRequest &requestData)
//{
//    if("GET" != requestData.method){
//        return responseStatusLine.arg("400 Bad request\r\n").toAscii();
//    }

//    if(requestData.fields.contains("Cookie") &&
//            "loggedin=1" == requestData.fields["Cookie"][0]){

//        return partialResponse + "\r\nYou're logged in!";
//    }

//    return partialResponse + "\r\nYou're not logged in!";;
//}

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
