#include <QDebug>
#include <QFileInfo>
#include <QPair>

#include "dispatcher.h"
#include "configuration.h"

Dispatcher::Dispatcher()
{
}
HTTPRequestHandler *Dispatcher::getHTTPRequestHandler(const QString &path) const
{
    QStringList urlParts = path.split("/", QString::SkipEmptyParts);
    QString urlStart("");

    if(!urlParts.empty()){
        urlStart = urlParts[0];
    }

    qDebug() << "urlStart:" << urlStart;
    QPair<QString, IPlugin*> pluginInfo;


    //TODO: store "path: <name, plugin>"
    foreach(QString key, Configuration::getPluginKeys()){
        qDebug() << "plugin_key (path):" << key;
        if(key.right(key.size()-1) == urlStart || (urlStart == "" && "/" == key)){
            //TODO: think if this returns NULL
            pluginInfo = Configuration::getPlugin(key);
            return pluginInfo.second->getHTTPRequestHandler(Configuration::get(pluginInfo.first).toHash());

            //return Configuration::getPlugin(key);
            //HTTPRequestHandler *requestHandler = plugin->getHTTPRequestHandler(Configuration::get(getPluginName(fullName)).toHash());
        }
    }

    //getPlugin returned "error"
    if(true){
    }

    pluginInfo = Configuration::getPlugin("static_file");
    return pluginInfo.second->getHTTPRequestHandler(Configuration::get(pluginInfo.first).toHash());
}
