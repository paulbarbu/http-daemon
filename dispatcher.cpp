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

    foreach(QString key, Configuration::getPluginKeys()){
        qDebug() << "plugin_key (path):" << key;
        if(key.right(key.size()-1) == urlStart || (urlStart == "" && "/" == key)){
            pluginInfo = Configuration::getPlugin(key);
            return pluginInfo.second->getHTTPRequestHandler(Configuration::get(pluginInfo.first).toHash());
        }
    }

    pluginInfo = Configuration::getPlugin("static_file");
    QPair<QString, IPlugin*> err;
    if(err == pluginInfo){
        return NULL;
    }

    return pluginInfo.second->getHTTPRequestHandler(Configuration::get(pluginInfo.first).toHash());
}
