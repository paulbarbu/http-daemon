#include <QDebug>

#include "dirhttprequesthandler.h"

DirHTTPRequestHandler::DirHTTPRequestHandler(const HTTPRequest &r, const QString &path) :
    HTTPRequestHandler(r), dir(path), fileInfo(path)
{
    requestData = r;
}

void DirHTTPRequestHandler::createResponse()
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

    QStringList dirList = dir.entryList();

    if(dirList.isEmpty()){
        response.setStatusCode(404);
        response.setReasonPhrase("Not Found");

        emit responseWritten(response);
        emit endOfWriting();

    }

    //TODO: format as HTML
    response.setStatusCode(200);
    response.setReasonPhrase("OK");
    response.setHeaderField("Content-Type", "text/plain");
    response.setBody(dirList.join("\n"));

    emit responseWritten(response);
    emit endOfWriting();
}
