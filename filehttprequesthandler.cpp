#include <QDebug>

#include "filehttprequesthandler.h"

FileHTTPRequestHandler::FileHTTPRequestHandler(const HTTPRequest &r, const QString &path) :
    HTTPRequestHandler(r), file(path), fileInfo(path)
{
    //TODO: set the mime type - libmagic
    requestData = r;
}

void FileHTTPRequestHandler::createResponse()
{
    HTTPResponse response;

    if(!fileInfo.exists()){
        response.setStatusCode(404);
        response.setReasonPhrase("Not Found");

        emit responseWritten(response);
        emit endOfWriting();

        return;
    }

    if(!fileInfo.isReadable()){
        qDebug() << "Not readable!";

        response.setStatusCode(403);
        response.setReasonPhrase("Forbidden");
        response.setBody("Permission denied\n");

        emit responseWritten(response);
        emit endOfWriting();

        return;
    }

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Cannot open";

        response.setStatusCode(500);
        response.setReasonPhrase("Internal Server Error");

        emit responseWritten(response);
        emit endOfWriting();

        return;
    }

    response.setStatusCode(200);
    response.setReasonPhrase("OK");

    QByteArray content;
    content = file.read(1024);

    while(!content.isEmpty()){
        response.appendBody(content);
        emit responseWritten(response);

        content = file.read(1024);
    }

    emit endOfWriting();
}
