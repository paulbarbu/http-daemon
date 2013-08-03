#include <QDebug>

#include "filehttprequesthandler.h"

FileHTTPRequestHandler::FileHTTPRequestHandler(const QString &path) :
    file(path), fileInfo(path)
{
    //TODO: set the mime type - libmagic
    //TODO: don't load the whole thing in memory - use onBytesWritten()
}

HTTPResponse FileHTTPRequestHandler::getResponse()
{
    HTTPResponse response;

    if(!fileInfo.exists()){
        response.setStatusCode(404);
        response.setReasonPhrase("Not Found");

        return response;
    }

    if(!fileInfo.isReadable()){
        qDebug() << "Not readable!";

        response.setStatusCode(403);
        response.setReasonPhrase("Forbidden");
        response.setBody("Permission denied\n");

        return response;
    }

    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Cannot open";

        response.setStatusCode(500);
        response.setReasonPhrase("Internal Server Error");
        return response;
    }

    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    response.setBody(file.readAll());

    return response;
}
