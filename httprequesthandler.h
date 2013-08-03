#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include "httpresponse.h"

class HTTPRequestHandler
{
public:
    virtual ~HTTPRequestHandler();
    virtual HTTPResponse getResponse()=0;
};

#endif // HTTPREQUESTHANDLER_H
