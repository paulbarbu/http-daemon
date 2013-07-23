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

    QByteArray response = processRequestData(parseHTTPRequest(request));

    //TODO: construct the response in a nice data structure and put it toghether here
    //also be good and send a Content-Length field (bytes)
    socket.write(response, response.size());

    if(!socket.waitForBytesWritten()){
        qDebug() << "Could not write byts to socket: " << socket.errorString();
        emit error(socket.error());
    }

    socket.close();
}

QString HTTPThread::readRequest(QTcpSocket *socket){
    QString request;

    //TODO: what if is a POST?

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

RequestData HTTPThread::parseHTTPRequest(QString request)
{
    RequestData requestData;
    requestData.valid = false;

    //TODO: what if I encounter \r\n in the URL or in the POST data?
    //TODO: be tolerant and replace "\r" with "" and then work with "\n"
    QStringList parts = request.split("\r\n\r\n", QString::SkipEmptyParts);

    if(parts.isEmpty()){
        return requestData;
    }

    QStringList fields = parts[0].split("\r\n", QString::SkipEmptyParts);

    QStringList statusLine = fields[0].split(" ");

    if(3 != statusLine.size()){
        return requestData;
    }

    QStringList protocol = statusLine[2].split("/");
    bool ok;

    if(2 != protocol.size()){
        return requestData;
    }

    double ver = protocol[1].toDouble(&ok);

    if("HTTP" != protocol[0] || !ok || ver < 0.9 || ver > 1.1){
        return requestData;
    }

    requestData.method = statusLine[0];
    requestData.url.setUrl(statusLine[1]);
    requestData.protocol = protocol[0];
    requestData.protocolVersion = ver;

    fields.removeAt(0);
    int spacePos;
    foreach(QString line, fields){
        spacePos = line.indexOf(" ");

        requestData.fields.insert(line.left(spacePos-1),
                           QStringList(line.right(line.size()-spacePos-1)));
    }

    if(requestData.fields.contains("Host")){
        requestData.fields["Host"] = requestData.fields["Host"][0].split(":");
    }


    if("POST" == requestData.method && 2 == parts.size() &&
            !parts[1].isEmpty()){
        requestData.postData = parsePostBody(parts[1]);
        //TODO: if it's empty error out
    }

    qDebug() << "Request data:\n\tStatusLine:\n\t\tMethod: "
             << requestData.method << "\n\t\tUrl: "
             << requestData.url << "\n\t\tProtocol: "
             << requestData.protocol << "\n\t\tVer: "
             <<requestData.protocolVersion
             << "\n\tFields: " << requestData.fields
             << "\n\tpost: " << requestData.postData;

    requestData.valid = true;
    return requestData;
}

QHash<QString, QString> HTTPThread::parsePostBody(QString postBody)
{
    // what if a radiobox is checked and sent?
    // what if: "option_set", with no "=" sign, is it possible?
    // what if the data contains "&"?
    QHash<QString, QString> retval;

    QStringList pairs = postBody.split("&", QString::SkipEmptyParts); //TODO: ???

    foreach(QString pair, pairs){
        QStringList keyVal = pair.split("="); //TODO: ???
        retval.insert(keyVal[0], keyVal[1]); //TODO: ???
    }

    return retval;
}

QByteArray HTTPThread::processRequestData(const RequestData &requestData)
{
    //TODO: add support for different Host values
    //TODO: URL rewriting?

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

        /* TODO: load HTTPSCripts via HTTPScriptLoader (.so and .dll files) ?
         * -> evolve this into FastCGI? read more
         *
         * or at least create a mapping of path -> method
         *
         *What's below isn't good because I have to modify the daemon every
         *time I want new functionality and the "login", "check", etc. methods are not a semantic part of HTTPThread
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
