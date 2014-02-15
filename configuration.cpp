#include <QFileInfo>
#include <QSettings>
#include <QReadWriteLock>
#include <QDebug>
#include <QStringList>
#include <QPluginLoader>

#include "configuration.h"
#include "logging.h"
#include "iplugin.h"

QHash<QString, QVariant> Configuration::conf;
QHash<QString, HTTPRequestHandler *> Configuration::plugins;

Configuration::Configuration(const QString &iniPath) : settingsPath(iniPath)
{
}

QReadWriteLock lock;

bool Configuration::read()
{
    QFileInfo info(settingsPath);
    if(!info.isReadable()){
        return false;
    }

    QHash<QString, QString> confPlugins;

    QSettings settings(settingsPath, QSettings::IniFormat);
    qDebug() << "Settings filename:" << settings.fileName();

    qDebug() << "Settings read:";

    lock.lockForWrite();
    foreach(const QString key, settings.allKeys()){
        if(0 == key.indexOf("plugins/")){
            //insert the URL (from which clients will communicate with the plugin) as the key
            //and the name of the library as the value
            confPlugins.insert(settings.value(key).toString(), key.right(key.size() - key.indexOf("/") - 1));
        }
        else if(-1 != key.indexOf("/")){
            QString new_key = key.right(key.size() - key.indexOf("/") - 1);
            QString section = key.left(key.indexOf("/"));

            if(!conf.contains(new_key)){
                conf[new_key] = QHash<QString, QVariant>();
            }

            QHash<QString, QVariant> t(conf[section].toHash());
            t.insert(new_key, settings.value(key).toString());
            conf[section] = t;

            qDebug() << section << "/" << new_key << ":" << conf[section].toHash()[new_key];
            qDebug() <<"to be:" << key << ":" << settings.value(key).toString();
        }
        else{
            conf[key] = settings.value(key);

            qDebug() << key << ":" << conf[key];
        }
    }

    if(!confPlugins.isEmpty()){
        loadPlugins(confPlugins);

        qDebug() << "Plugins read from config:" << plugins;
    }

    lock.unlock();

    return true;
}

QVariant Configuration::get(const QString &key, QVariant defaultValue)
{
    //TODO: think about read-locking this
    return conf.value(key, defaultValue);
}

QStringList Configuration::getPluginKeys()
{
    return plugins.keys();
}

HTTPRequestHandler *Configuration::getPluginRequestHandler(const QString &key)
{
    if(plugins.contains(key)){
        return plugins[key];
    }

    return NULL;
}

QString Configuration::getSettingsPath() const
{
    QFileInfo info(settingsPath);
    return info.absoluteFilePath();
}

bool Configuration::check() const
{
    bool ok = true;
    QFileInfo info(get("documentroot").toString());

    if(!info.isReadable()){
        QByteArray msg;
        msg = info.absoluteFilePath().toLocal8Bit();

        syslog(LOG_ERR, "The documentroot path (%s) is not readable!", msg.constData());
        ok = false;
    }

    info.setFile(get("pluginroot").toString());

    if(!info.isReadable()){
        QByteArray msg;
        msg = info.absoluteFilePath().toLocal8Bit();

        syslog(LOG_ERR, "The pluginroot path (%s) is not readable!", msg.constData());
        ok = false;
    }

    return ok;
}

void Configuration::loadPlugins(const QHash<QString, QString> &confPlugins)
{
    const QString pluginRoot(conf["pluginroot"].toString());

    QHash<QString, QString>::const_iterator i;

    for(i = confPlugins.constBegin(); i != confPlugins.constEnd(); ++i){
        QPluginLoader loader(pluginRoot + i.value());

        #ifdef QT_DEBUG
            qDebug() << "Loading from: " << loader.fileName();
        #endif

        QObject *p = loader.instance();

        IPlugin *requestHandlerFactory = qobject_cast<IPlugin *>(p);

        if(!requestHandlerFactory){
            qDebug() << "Plugin (" << i.key() << ":" << i.value() << ") not loaded: " << loader.errorString();
            continue;
        }

        plugins.insert(i.key(),
            requestHandlerFactory->getHTTPRequestHandler(Configuration::get(getPluginName(i.value())).toHash()));
    }
}

QString Configuration::getPluginName(const QString &fullName) const
{
    QString name(fullName);

    name.replace(".so", "").replace(".dll", "");
    if(0 == name.indexOf("lib")){
        name.replace(0, 3, "");
    }

    return name;
}
