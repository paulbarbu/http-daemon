#include <QDebug>
#include <QFileInfo>
#include <QPluginLoader>

#include "dispatcher.h"
#include "iplugin.h"
#include "filehttprequesthandler.h"
#include "dirhttprequesthandler.h"
#include "configuration.h"

Dispatcher::Dispatcher()
{
}

HTTPRequestHandler* Dispatcher::getHTTPRequestHandler(const HTTPRequest &requestData) const
{
    QHash<QString, QVariant> plugins(Configuration::get("plugins").toHash());

    QStringList urlParts = requestData.url.path().split("/", QString::SkipEmptyParts);
    QString urlStart("");

    if(!urlParts.empty()){
        urlStart = urlParts[0];
    }

    qDebug() << "urlStart:" << urlStart;

    foreach(QString key, plugins.keys()){
        qDebug() << "plugin_key (path):" << key << "index of urlStart in key:" << key.indexOf(urlStart);
        if(1 == key.indexOf(urlStart) || (urlStart == "" && "/" == key)){
            return loadPlugin(plugins[key].toString(), requestData);
        }
    }

    const QString docRoot(Configuration::get("documentroot").toString());

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
    const QString pluginRoot(Configuration::get("pluginroot").toString());

    QPluginLoader loader(pluginRoot + "/" + plugin);

    #ifdef QT_DEBUG
        QFileInfo info(pluginRoot + "/" + plugin);
        qDebug() << "Loading from: " << info.absoluteFilePath();
    #endif

    QObject *p = loader.instance();

    IPlugin *requestHandlerFactory = qobject_cast<IPlugin *>(p);

    if(!requestHandlerFactory){
        qDebug() << "Plugin not loaded: " << loader.errorString();
        return NULL;
    }

    QString conf(plugin);

    conf.replace(".so", "").replace(".dll", "");
    if(0 == conf.indexOf("lib")){
        conf.replace(0, 3, "");
    }

    return requestHandlerFactory->getHTTPRequestHandler(requestData, Configuration::get(conf).toHash());
}
