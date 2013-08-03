#ifndef FILEHTTPREQUESTHANDLER_H
#define FILEHTTPREQUESTHANDLER_H

#include <QString>
#include <QFile>
#include <QFileInfo>

#include "httprequesthandler.h"
#include "httpresponse.h"

class FileHTTPRequestHandler : public HTTPRequestHandler
{
public:
    FileHTTPRequestHandler(const QString &path);
    HTTPResponse getResponse();

private:
    QFile file;
    QFileInfo fileInfo;
};

#endif // FILEHTTPREQUESTHANDLER_H
