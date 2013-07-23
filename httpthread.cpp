#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QDir>

#include "httpthread.h"

HTTPThread::HTTPThread(const int socketDescriptor, const QString &docRoot,
                       QObject *parent) :
    QThread(parent), socketDescriptor(socketDescriptor), docRoot(docRoot),
    responseStatusLine("HTTP/1.0 %1\r\n")
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

    QString request = readRequest(&socket);
    qDebug() << request;

    //TODO: create a ResponseData class instead of running around with QStrings?
    QByteArray response = processRequestData(parser.parseRequest(request));

    socket.write(response, response.size());

    if(!socket.waitForBytesWritten()){
        qDebug() << "Could not write byts to socket: " << socket.errorString();
        emit error(socket.error());
    }

    socket.close();
}

QString HTTPThread::readRequest(QTcpSocket *socket){
    QString request;

    //TODO: what if is a POST and the data won't come in one chunk?
    // if the header is sent and then the POST data then this will fail to get
    // the POST data

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

QByteArray HTTPThread::processRequestData(const RequestData &requestData)
{
    //TODO: add support for different Host values?
    //TODO: URL rewriting?
    //TODO: integrate PHP

    QByteArray response = responseStatusLine.arg("200 OK").toUtf8();

    if(!requestData.valid){
        return responseStatusLine.arg("400 Bad request\r\n").toAscii();
    }

    if("GET" == requestData.method || "POST" == requestData.method){
        //serve static files

        QString fullPath = docRoot + requestData.url.path();
        QFileInfo f(fullPath);

        qDebug() << requestData.method << " " << fullPath;

        if(f.exists() && f.isReadable()){
            if(f.isDir()){
                response = serveStaticDir(response, f.absoluteFilePath());
            }
            else{
                response = serveStaticFile(response, f.absoluteFilePath());

                if(response.isEmpty()){
                    response = responseStatusLine.arg(
                                "500 Internal Server Error\r\n").toAscii();
                }
            }
        }

        /* TODO: load things via a common interface from .so and .dll files?
         * -> evolve this into FastCGI? read more
         *
         * or at least create a mapping of path -> method
         *
         * What's below isn't good because I have to modify the daemon every
         * time I want new functionality and the "login", "check", etc.
         * methods are not a semantic part of HTTPThread
         */
        else if("/patrat" == requestData.url.path()){
            response = square(response, requestData);
        }
        else if("/login" == requestData.url.path()){
            response = login(response, requestData);
        }
        else if("/verifica" == requestData.url.path()){
            response = check(response, requestData);
        }
        else if(!f.exists()){
            response = responseStatusLine.arg("404 Not Found\r\n").toAscii();
        }
        else if(!f.isReadable()){
            qDebug() << "Not readable!";
            response = responseStatusLine.arg("403 Forbidden\r\n").toAscii() +
                    "Permission denied\n";
        }
    }
    else{
        response = responseStatusLine.arg("501 Not Implemented\r\n").toAscii();
        qDebug() << "Unsupported HTTP method!";
    }

    return response;
    return responseStatusLine.arg("200 OK\r\n").toAscii();
}

QByteArray HTTPThread::serveStaticFile(const QByteArray &partialResponse,
                                       const QString &filePath)
{
    //TODO: set the mime type
    QFile file(filePath);

    if(!file.open( QIODevice::ReadOnly)){
        qDebug() << "Cannot open";
        return "";
    }

    return partialResponse + "\r\n" + file.readAll();
}

QByteArray HTTPThread::serveStaticDir(const QByteArray &partialResponse,
                                      const QString &dirPath)
{
    QDir dir(dirPath);
    QStringList dirList = dir.entryList();

    if(dirList.isEmpty()){
        return responseStatusLine.arg("404 Not Found\r\n").toAscii();
    }

    //TODO: format as HTML
    return partialResponse + "Content-Type: text/plain\r\n\r\n" +
            dirList.join("\n").toUtf8();
}

QByteArray HTTPThread::square(const QByteArray &partialResponse,
                              const RequestData &requestData)
{
    if("GET" != requestData.method || !requestData.url.hasQueryItem("a")){
        return responseStatusLine.arg("400 Bad Request\r\n").toAscii();
    }

    QString numToSquare = requestData.url.queryItemValue("a");
    QString body;

    bool ok;
    double n = numToSquare.toDouble(&ok);

    if(!ok){
        body = "a-ul trebuie sa fie numar!\n";
    }
    else{
        body = numToSquare + "^2 = " + QString::number(n*n) + "\n";
    }

    return partialResponse + "\r\n" + body.toAscii();
}

QByteArray HTTPThread::login(const QByteArray &partialResponse,
                             const RequestData &requestData)
{
    QString page = "\r\n<html><body>"
            "<form method=\"POST\">"
            "%1"
            "Username: <input type=\"text\" name=\"username\">"
            "Password: <input type=\"password\" name=\"pass\">"
            "<INPUT type=\"submit\" value=\"Auth\">"
            "</form></body></html>";

    if("GET" == requestData.method){
        return partialResponse + page.arg("").toAscii();
    }

    if("POST" == requestData.method && !requestData.postData.isEmpty()){
        if(requestData.postData.contains("username") &&
                "Ion" == requestData.postData["username"] &&
                requestData.postData.contains("pass") &&
                "1234" == requestData.postData["pass"]){

            return partialResponse + "Set-Cookie: loggedin=1\r\n\r\nYou're logged in!";
        }

        return partialResponse +
                page.arg("Login failed, try again!<br>").toAscii();
    }

    return responseStatusLine.arg("400 Bad request\r\n").toAscii();
}

QByteArray HTTPThread::check(const QByteArray &partialResponse,
                             const RequestData &requestData)
{
    if("GET" != requestData.method){
        return responseStatusLine.arg("400 Bad request\r\n").toAscii();
    }

    if(requestData.fields.contains("Cookie") &&
            "loggedin=1" == requestData.fields["Cookie"][0]){

        return partialResponse + "\r\nYou're logged in!";
    }

    return partialResponse + "\r\nYou're not logged in!";;
}
