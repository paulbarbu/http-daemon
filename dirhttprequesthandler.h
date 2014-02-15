#ifndef DIRHTTPREQUESTHANDLER_H
#define DIRHTTPREQUESTHANDLER_H

#include <QDir>
#include <QFileInfo>

#include "httprequesthandler.h"
#include "httpresponse.h"

class DirHTTPRequestHandler : public HTTPRequestHandler
{
public:
    DirHTTPRequestHandler(const HTTPRequest &r, const QString &path);
    void createResponse(const HTTPRequest &r);

private:
    QDir dir;
    QFileInfo fileInfo;
};

#endif // DIRHTTPREQUESTHANDLER_H
