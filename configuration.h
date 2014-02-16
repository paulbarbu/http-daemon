#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "httprequesthandler.h"

#define CONFIG_FILE_NAME "httpdaemon.conf"

class Configuration
{
public:
    explicit Configuration(const QString &iniPath);
    ~Configuration();
    bool read();
    static QVariant get(const QString &key, QVariant defaultValue = QVariant());
    static QStringList getPluginKeys();
    static HTTPRequestHandler *getPluginRequestHandler(const QString &key);
    QString getSettingsPath() const;
    bool check() const;
protected:
    void loadPlugins(const QHash<QString, QString> &confPlugins);
    QString getPluginName(const QString &fullName) const;
    QString dereference(const QString &value);
private:
    const QString settingsPath;
    static QHash<QString, QVariant> conf;
    static QHash<QString, HTTPRequestHandler *> plugins;
};

#endif // CONFIGURATION_H
