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
    FileHTTPRequestHandler(const HTTPRequest &r, const QString &path);
    void createResponse();

private:
    QFile file;
    QFileInfo fileInfo;
};

#endif // FILEHTTPREQUESTHANDLER_H
