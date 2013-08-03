#include <QDebug>
#include <QFileInfo>

#include "dispatcher.h"
#include "filehttprequesthandler.h"
#include "dirhttprequesthandler.h"

Dispatcher::Dispatcher(const QString &docRoot) : docRoot(docRoot)
{
    /* TODO: use boost's property_tree:
     * http://www.boost.org/doc/libs/1_54_0/doc/html/property_tree.html
     */
    dynamicHandlers.insert("/patrat", "TODO: add handler");
    dynamicHandlers.insert("/login", "TODO: add handler");
    dynamicHandlers.insert("/verifica", "TODO: add handler");
}

HTTPRequestHandler* Dispatcher::getHTTPRequestHandler(const HTTPRequest &requestData)
{
    if(dynamicHandlers.contains(requestData.url.path())){
        return loadModule(dynamicHandlers[requestData.url.path()]);
    }

    QString fullPath = docRoot + requestData.url.path();
    QFileInfo f(fullPath);

    qDebug() << requestData.method << " " << fullPath;

    if(f.isDir()){
        return new DirHTTPRequestHandler(f.absoluteFilePath());
    }

    return new FileHTTPRequestHandler(f.absoluteFilePath());
}

HTTPRequestHandler* Dispatcher::loadModule(QString module)
{
    //TODO: implement this
    Q_UNUSED(module);
    return NULL;
}
