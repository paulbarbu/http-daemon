#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QDir>

#include "httpthread.h"

HTTPThread::HTTPThread(const int socketDescriptor, const QString &docRoot,
                       QObject *parent) :
    QThread(parent), isParsedHeader(false),
    socketDescriptor(socketDescriptor), docRoot(docRoot),
    responseStatusLine("HTTP/1.0 %1\r\n")
{
    if(!socket.setSocketDescriptor(socketDescriptor)){
        qDebug() << "Setting the sd has failed: " << socket.errorString();
        emit error(socket.error());

        return;
    }

    connect(&socket, SIGNAL(disconnected()), this, SLOT(quit()));
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(requestParsed(RequestData)), this,
                         SLOT(onRequestParsed(RequestData)));
    connect(&socket, SIGNAL(readyRead()), this, SLOT(read()));
}

void HTTPThread::run()
{

    //TODO: create a ResponseData class instead of running around with QStrings?
    exec();
}

void HTTPThread::read(){
    request.append(QString(socket.readAll()));

    QString lf = "\n\n";
    int crlfPos = request.indexOf("\r\n\r\n");
    int lfPos = request.indexOf("\n\n");

    if(!isParsedHeader && (-1 != crlfPos || -1 != lfPos)){
        isParsedHeader = true;
        requestData = parser.parseRequestHeader(request);

        if(-1 != crlfPos){
            lf = "\r\n\r\n";
            lfPos = crlfPos;
        }

        bytesToParse = requestData.contentLength;

        //discard the header from the request
        request = request.replace(lf, "").right(request.size()-lfPos-lf.size());
    }

    if(isParsedHeader && "POST" == requestData.method && !request.isEmpty()){

        if(0 >= bytesToParse){
            //a POST request with no Content-Length is bogus as per standard
            requestData.valid = false;
            emit requestParsed(requestData);
        }

        bytesToParse -= request.size();
        QHash<QString, QString> postData = parser.parsePostBody(request);

        if(!postData.isEmpty()){
            QHash<QString, QString>::const_iterator i;
            for(i = postData.constBegin(); i != postData.constEnd(); ++i){
                requestData.postData.insert(i.key(), i.value());
            }

            request = "";
        }
    }

    if(isParsedHeader &&
            (0 == bytesToParse || "GET" == requestData.method)){
        emit requestParsed(requestData);
    }
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
        /* TODO:
         * Metoda de servire a fisierelor statice e cam ineficienta.
         * Practic incarci tot fisierul in memorie si tot concatenezi array-uri,
         * cand ai putea sa trimiti direct pe tava pe masura ce citesti.
         */
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

void HTTPThread::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError << ": " << socket.errorString();

    //TODO: check the state(){
        socket.disconnectFromHost();
    //}

    emit error(socketError);
    quit();
}

void HTTPThread::onRequestParsed(const RequestData &requestData)
{
    qDebug() << "Request data:\n\tMethod: "
             << requestData.method << "\n\tUrl: "
             << requestData.url << "\n\tProtocol: "
             << requestData.protocol << "\n\tVer: "
             <<requestData.protocolVersion
             << "\n\tFields: " << requestData.fields
             << "\n\tContent-Length: " << requestData.contentLength
             << "\n\tpost: " << requestData.postData;

    QByteArray response = processRequestData(requestData);

    socket.write(response, response.size());
    socket.disconnectFromHost();
}
