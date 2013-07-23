#include <QtNetwork/QTcpSocket>
#include <QFileInfo>
#include <QFile>

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

    QString request = read(&socket);
    qDebug() << request;

    QByteArray response = processRequestData(parseRequest(request));

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

    if(lines.isEmpty()){
        return requestData;
    }

    QStringList statusLine = lines[0].split(" ");

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

QByteArray HTTPThread::processRequestData(
        const QHash<QString, QStringList> &requestData)
{
    //TODO: add support for different Host values
    //TODO: URL rewriting?

    QByteArray response = responseStatusLine.arg("200 OK").toUtf8();

    if(requestData.isEmpty()){
        return responseStatusLine.arg("400 Bad request\r\n").toAscii();
    }

    QString method = requestData["status-line"][0];

    /* dynamic: load HTTPSCripts via HTTPScriptLoader (.so and .dll files) ?
     * -> evolve this into FastCGI ?
     * static: serve content as is
     *
     * check the network and the files owned by my program to see what QTcpServer does
     */

    if("GET" == method || "POST" == method){
        QString path = requestData["status-line"][1];
        int numberSignPos = path.indexOf("#");

        if(-1 != numberSignPos){
            path = path.left(numberSignPos);
        }

        int questionMarkPos = path.indexOf("?");
        QString queryString;

        if(-1 != questionMarkPos){
            //serve "dynamic" files

            queryString = path.right(path.size()-questionMarkPos-1);
            path = path.left(questionMarkPos);
            //TODO: parse the query_string
        }
        else{
            //serve static files

            QString fullPath = docRoot + path;
            qDebug() << "Full path: " << fullPath;
            QFileInfo f(fullPath);

            if(!f.exists()){
                return responseStatusLine.arg("404 Not Found\r\n").toAscii();
            }

            if(!f.isReadable()){
                qDebug() << "Not readable!";
                return responseStatusLine.arg("403 Forbidden\r\n").toAscii() +
                        "Permission denied\n";
            }

            qDebug() << "Readable!";

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

        qDebug() << method << " " << docRoot << path << queryString;
    }
    else{
        response = responseStatusLine.arg("501 Not Implemented\r\n").toAscii();
        qDebug() << "Unsupported HTTP method!";
    }

    return response;
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
    return partialResponse + dirPath.toUtf8();
}
