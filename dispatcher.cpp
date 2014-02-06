#include <QDebug>
#include <QFileInfo>
#include <QPluginLoader>

#include "dispatcher.h"
#include "iplugin.h"
#include "filehttprequesthandler.h"
#include "dirhttprequesthandler.h"

Dispatcher::Dispatcher(const QString &docRoot, const QString &pluginRoot) :
    docRoot(docRoot), pluginRoot(pluginRoot)
{
    /* TODO: use boost's property_tree:
     * http://www.boost.org/doc/libs/1_54_0/doc/html/property_tree.html
     */
    //TODO: port to windows => squareplugin.dll
    dynamicHandlers.insert("/square", "libsquareplugin.so");
    dynamicHandlers.insert("/login", "libloginplugin.so");
}

HTTPRequestHandler* Dispatcher::getHTTPRequestHandler(const HTTPRequest &requestData) const
{
    if(dynamicHandlers.contains(requestData.url.path())){
        return loadPlugin(dynamicHandlers[requestData.url.path()], requestData);
    }

    QString fullPath = docRoot + requestData.url.path();
    QFileInfo f(fullPath);

    qDebug() << requestData.method << " " << fullPath;

    if(f.isDir()){
        return new DirHTTPRequestHandler(requestData, f.absoluteFilePath());
    }

    return new FileHTTPRequestHandler(requestData, f.absoluteFilePath());
}

HTTPRequestHandler* Dispatcher::loadPlugin(const QString &plugin,
                                           const HTTPRequest &requestData) const
{
    QPluginLoader loader(pluginRoot + "/" + plugin);

    QObject *p = loader.instance();

    IPlugin *requestHandlerFactory = qobject_cast<IPlugin *>(p);

    if(!requestHandlerFactory){
        qDebug() << "Plugin not loaded: " << loader.errorString();
        return NULL;
    }

    return requestHandlerFactory->getHTTPRequestHandler(requestData);
}
