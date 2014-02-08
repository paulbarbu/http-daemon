#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QHash>
#include <QString>
#include <QVariant>

#define CONFIG_FILE_NAME "httpdaemon.conf"

class Configuration
{
public:
    explicit Configuration(const QString &iniPath);
    bool read();
    static QVariant get(const QString &key, QVariant defaultValue = QVariant());
    QString getSettingsPath() const;
    bool check() const;
private:
    const QString settingsPath;
    static QHash<QString, QVariant> conf;
};

#endif // CONFIGURATION_H
