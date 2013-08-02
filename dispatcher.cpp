#include <QDebug>
#include <QFileInfo>

#include "dispatcher.h"

Dispatcher::Dispatcher(const HTTPRequest &requestData, const QString &docRoot) :
    requestData(requestData), docRoot(docRoot)
{
    /* TODO: use boost's property_tree:
     * http://www.boost.org/doc/libs/1_54_0/doc/html/property_tree.html
     */
    dynamicHandlers.insert("/patrat", "TODO: add handler");
    dynamicHandlers.insert("/login", "TODO: add handler");
    dynamicHandlers.insert("/verifica", "TODO: add handler");
}

HTTPRequestHandler Dispatcher::getHTTPRequestHandler()
{
    /* TODO: try to find the dynamic/static paths if nothing is found, return a
     * HTTPRequestHandler (not a subclass) that will return a 404 or 403 when
     * asked for the HTTPResponse
     */

    if("GET" != requestData.method && "POST" != requestData.method){
        //TODO: properly implement this
        return responseStatusLine.arg("501 Not Implemented\r\n").toAscii();
    }

    if(dynamicHandlers.contains(requestData.url.path())){
        return loadModule(dynamicHandlers[requestData.url.path()]);
    }

    //serve static files
    /* TODO: optimize the sending of static files (don't load the whole
     * thing in memory)
     */

    QString fullPath = docRoot + requestData.url.path();
    QFileInfo f(fullPath);

    qDebug() << requestData.method << " " << fullPath;


    if(!f.exists()){
        //TODO: properly implement this
        return responseStatusLine.arg("404 Not Found\r\n").toAscii();
    }

    if(!f.isReadable()){
        //TODO: properly implement this
        qDebug() << "Not readable!";
        return responseStatusLine.arg("403 Forbidden\r\n").toAscii() +
                "Permission denied\n";
    }

    if(f.isDir()){
        //TODO: properly implement this
        return serveStaticDir(response, f.absoluteFilePath());
    }

    //TODO: properly implement this
    return serveStaticFile(response, f.absoluteFilePath());
}

HTTPRequestHandler Dispatcher::loadModule(QString module)
{
    //TODO: implement this
}
