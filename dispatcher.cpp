#include <QDebug>
#include <QFileInfo>
#include <QPluginLoader>
#include <QCoreApplication>

#include "dispatcher.h"
#include "iplugin.h"
#include "filehttprequesthandler.h"
#include "dirhttprequesthandler.h"

Dispatcher::Dispatcher(const QString &docRoot) : docRoot(docRoot)
{
    /* TODO: use boost's property_tree:
     * http://www.boost.org/doc/libs/1_54_0/doc/html/property_tree.html
     */
    dynamicHandlers.insert("/square", "libsquareplugin.so");
    dynamicHandlers.insert("/login", "libloginplugin.so");

    //TODO: boost::program_args
    QStringList args(QCoreApplication::arguments());
    int pos = args.indexOf("--pluginroot");
    pluginRoot = "./plugins";

    if(-1 != pos){
        QFileInfo f(args[pos+1]);

        if(f.isReadable()){
            pluginRoot = f.absoluteFilePath();
        }
    }
}

HTTPRequestHandler* Dispatcher::getHTTPRequestHandler(const HTTPRequest &requestData)
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
                                           const HTTPRequest &requestData)
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
