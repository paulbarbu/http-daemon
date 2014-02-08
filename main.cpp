#include <errno.h>

#include <QtGlobal>

#ifndef Q_OS_WIN32
    #include <unistd.h>
    #include <sys/stat.h>
#endif

#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include "httpdaemon.h"
#include "logging.h"
#include "configuration.h"

int main(int argc, char *argv[])
{
    openlog("http-daemon", LOG_ODELAY, LOG_DAEMON);

    #if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        qInstallMessageHandler(qtOutputToLog);
    #else
        qInstallMsgHandler(qtOutputToLog);
    #endif

    QCoreApplication a(argc, argv);
    a.setObjectName("main app");

    QTextStream out(stdout);
    QTextStream err(stderr);

    #ifndef CONFIG_DIR
        #define CONFIG_DIR "";
    #endif

    Configuration c(CONFIG_DIR CONFIG_FILE_NAME);

    if(!c.read()){
        syslog(LOG_ERR, "Cannot read configuration file at: %s", c.getSettingsPath().toStdString().c_str());
        return 1;
    }

    if(!c.check()){
        syslog(LOG_ERR, "Invalid configuration file at: %s", c.getSettingsPath().toStdString().c_str());
        return 1;
    }

#ifndef Q_OS_WIN32
    pid_t pid = fork();

    if(-1 == pid){
        err << strerror(errno);
        syslog(LOG_ERR, strerror(errno));

        return 1;
    }

    if(pid > 0){
        out << "Successfully forked child with PID: " << pid;
        syslog(LOG_INFO, "Successfully forked child with PID: %i", pid);

        return 0;
    }

    //TODO: become a user (configured via http-daemon.conf)
    umask(0);
    pid_t sid = setsid();

    if(-1 == sid){
        syslog(LOG_ERR, "setsid() failed: %s", strerror(errno));
        return 1;
    }

    if(-1 == chdir("/")){
        syslog(LOG_ERR, "chdir() failed: %s", strerror(errno));
        return 1;
    }

    //TODO: figure out why if I enable these s.listen will fail, create a POC
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
#endif

    bool ok;
    int port = Configuration::get("port", 80).toInt(&ok);

    if(!ok){
        port = 80;
    }

    HTTPDaemon s;
    s.setObjectName("HTTPDaemon");

    if(!s.listen(QHostAddress(Configuration::get("address").toString()), port)){
         syslog(LOG_ERR, "Cannot start the server: %s", s.errorString().toStdString().c_str());
         return 1;
    }
    else{
        syslog(LOG_NOTICE, "Listening on %s:%i", s.serverAddress().toString().toStdString().c_str(), s.serverPort());
        syslog(LOG_INFO, "Document root is in: %s\nPlugin root is in: %s",
            Configuration::get("documentroot").toString().toStdString().c_str(),
            Configuration::get("pluginroot").toString().toStdString().c_str());
    }

    return a.exec();
}
