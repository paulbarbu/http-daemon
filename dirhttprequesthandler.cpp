#include <QDebug>

#include "dirhttprequesthandler.h"

DirHTTPRequestHandler::DirHTTPRequestHandler(const QString &path) :
    dir(path), fileInfo(path)
{
}

HTTPResponse DirHTTPRequestHandler::getResponse()
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

    QStringList dirList = dir.entryList();

    if(dirList.isEmpty()){
        response.setStatusCode(404);
        response.setReasonPhrase("Not Found");

        return response;
    }

    //TODO: format as HTML
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    response.setHeaderField("Content-Type", "text/plain");
    response.setBody(dirList.join("\n"));

    return response;
}
