#include <QFileInfo>
#include <QSettings>
#include <QReadWriteLock>
#include <QDebug>
#include <QStringList>

#include "configuration.h"
#include "logging.h"

QHash<QString, QVariant> Configuration::conf;

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

    QHash<QString, QVariant> plugins;

    QSettings settings(settingsPath, QSettings::IniFormat);
    qDebug() << "Settings filename:" << settings.fileName();

    qDebug() << "Settings read:";

    lock.lockForWrite();
    foreach(const QString key, settings.allKeys()){
        if(0 == key.indexOf("plugins/")){
            //insert the URL (from which clients will communicate with the plugin) as the key
            //and the name of the library as the value
            plugins.insert(settings.value(key).toString(), key.right(key.size() - key.indexOf("/") - 1));
        }
        else{
            conf[key] = settings.value(key);

            qDebug() << key << ":" << conf[key];
        }
    }

    if(!plugins.isEmpty()){
        conf["plugins"] = plugins;

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
