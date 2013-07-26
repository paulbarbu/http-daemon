#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>
#include <QStringList>
#include <QDir>
#include <QThread>

#include "httpconnection.h"

/*TODO: refactor
Creezi doua clase model, HTTPRequest, HTTPResponse ce contin informatiile
primite/de trimis catre client. De exemplu metoda, versiunea, content length,
cookie-uri, etc. Query-ul, de exemplu ar fi stocat frumos sub forma de perechi
cheie-valoare. Cele doua clase ar contine si cate un stream pentru citire
respectiv scriere. Creezi o clasa HTTPRequestParser ce ar avea rolul de a citi
informatiile primite prin socket si de a crea o clasa HTTPRequest
corespunzatoare. Stream-ul la care va face referire aceasta clase va permite
citirea ulterioara a informatiilor in cazul unui POST de exemplu.
Dupa ce ai obtinut un HTTPRequest, il dai mai departe catre o instanta a unei
noi clase, sa-i zicem Dispatcher, ce verifica ce este cerut defapt si, pe baza
unor reguli, returneaza o referinta spre un HTTPRequestHandler.
HTTPRequestHandler-ul ales va primi apoi cererea si va returna un raspuns,
HTTPResponse. Din stream-ul mentionat in acesta, se vor citi datele pe care
serverul le va trimite inapoi, spre client. Exemple de HTTPRequestHandler ar fi
FileHTTPRequestHandler ce serveste static un fisier aflat pe disc, sau diverse
clase ce ar reprezenta aplicatii custom gen cea necesara pentru /patrat. Astfel
de aplicatii ar putea fi stocate si in DLL-uri, ele fiind destul de
independente. Treaba lor ar fi astfel doar de a returna un raspuns la o cerere,
neinteresandu-le cum citesti tu din socket, sau cum gestionezi 1000 de conexiuni
simultane.
*/

HTTPConnection::HTTPConnection(int socketDescriptor, const QString &docRoot,
                       QObject *parent) : QObject(parent), socket(this),
    isParsedHeader(false), eventLoop(this), docRoot(docRoot),
    responseStatusLine("HTTP/1.0 %1\r\n")
{
    if(!socket.setSocketDescriptor(socketDescriptor)){
        qDebug() << socket.errorString() << "Cannot set sd: " << socketDescriptor;
        emit closed();
    }
}

void HTTPConnection::start()
{
    qDebug() << "start() in: " << QThread::currentThread();
    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(onError(QAbstractSocket::SocketError)));
    connect(&socket, SIGNAL(readyRead()), this, SLOT(read()));
    connect(this, SIGNAL(requestParsed(RequestData)), this,
            SLOT(onRequestParsed(RequestData)));
    connect(this, SIGNAL(closed()), &eventLoop, SLOT(quit()));

    eventLoop.exec();
}

void HTTPConnection::read(){
    qDebug() << "reead() in: " << QThread::currentThread();
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

QByteArray HTTPConnection::processRequestData(const RequestData &requestData)
{
    //TODO: add support for different Host values?
    //TODO: URL rewriting?
    //TODO: integrate FastCGI

    QByteArray response = responseStatusLine.arg("200 OK").toUtf8();

    if(!requestData.valid){
        return responseStatusLine.arg("400 Bad request\r\n").toAscii();
    }

    if("GET" == requestData.method || "POST" == requestData.method){
        //serve static files
        /* TODO: optimize the sending of static files (don't load the whole
         * thing in memory)
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
}

QByteArray HTTPConnection::serveStaticFile(const QByteArray &partialResponse,
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

QByteArray HTTPConnection::serveStaticDir(const QByteArray &partialResponse,
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

QByteArray HTTPConnection::square(const QByteArray &partialResponse,
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

QByteArray HTTPConnection::login(const QByteArray &partialResponse,
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

QByteArray HTTPConnection::check(const QByteArray &partialResponse,
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

void HTTPConnection::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError << ": " << socket.errorString();
    socket.disconnectFromHost();
    socket.waitForDisconnected(1000);
    emit closed();
}

void HTTPConnection::onRequestParsed(const RequestData &requestData)
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
    socket.waitForDisconnected(1000);
    emit closed();
}
