#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include <QObject>

#include "httpresponse.h"

class HTTPRequestHandler : public QObject
{
    Q_OBJECT
public:
    virtual ~HTTPRequestHandler() {}
    virtual void createResponse()=0;

 signals:
    void responseWritten(HTTPResponse response);
    void endOfWriting();
};

#endif // HTTPREQUESTHANDLER_H
