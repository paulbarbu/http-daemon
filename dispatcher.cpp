#include <QDebug>
#include <QFileInfo>
#include "dispatcher.h"
#include "configuration.h"

Dispatcher::Dispatcher()
{
}

HTTPRequestHandler* Dispatcher::getHTTPRequestHandler(const QString &path) const
{
    QStringList urlParts = path.split("/", QString::SkipEmptyParts);
    QString urlStart("");

    if(!urlParts.empty()){
        urlStart = urlParts[0];
    }

    qDebug() << "urlStart:" << urlStart;

    foreach(QString key, Configuration::getPluginKeys()){
        qDebug() << "plugin_key (path):" << key;
        if(key.right(key.size()-1) == urlStart || (urlStart == "" && "/" == key)){
            return Configuration::getPluginRequestHandler(key);
        }
    }

    return Configuration::getPluginRequestHandler("static_file");
}
