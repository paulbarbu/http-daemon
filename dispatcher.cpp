#include <QDebug>
#include <QFileInfo>
#include "dispatcher.h"
#include "filehttprequesthandler.h"
#include "dirhttprequesthandler.h"
#include "configuration.h"

Dispatcher::Dispatcher()
{
}

//TODO: pass this just what it needs, the URL (after File and DirHTTPRequestHandlers become plugins
HTTPRequestHandler* Dispatcher::getHTTPRequestHandler(const HTTPRequest &requestData) const
{
    QStringList urlParts = requestData.url.path().split("/", QString::SkipEmptyParts);
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

    const QString docRoot(Configuration::get("documentroot").toString());

    QString fullPath = docRoot + requestData.url.path();
    QFileInfo f(fullPath);

    qDebug() << requestData.method << " " << fullPath;

    //TODO: move these to plugins
    //TODO: HTTPConnection will not delete these if I make exception for the plugins - won't be a problem if they will be plugins, too
    if(f.isDir()){
        return new DirHTTPRequestHandler(requestData, f.absoluteFilePath());
    }

    return new FileHTTPRequestHandler(requestData, f.absoluteFilePath());
}
