#include <errno.h>
#include <stdlib.h>

#include <QtGlobal>

#ifndef Q_OS_WIN32
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include "httpdaemon.h"
#include "logging.h"
#include "configuration.h"

//TODO: make some test on other platforms/versions with/without QT += concurrent and LIBS += -lQt5Concurrent
//TODO: test from telnet without port number => bad port number in httprequest
int main(int argc, char *argv[])
{
    openlog("http-daemon", LOG_ODELAY, LOG_DAEMON);
    atexit(closelog);

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
        QByteArray msg;
        msg = c.getSettingsPath().toLocal8Bit();

        syslog(LOG_ERR, "Cannot read configuration file at: %s", msg.constData());
        return 1;
    }

    if(!c.check()){
        QByteArray msg;
        msg = c.getSettingsPath().toLocal8Bit();

        syslog(LOG_ERR, "Invalid configuration file at: %s", msg.constData());
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
        out << "Successfully forked child with PID: " << pid << endl;
        syslog(LOG_INFO, "Successfully forked child with PID: %i", pid);

        return 0;
    }

    umask(0);
    pid_t sid = setsid();

    if(-1 == sid){
        syslog(LOG_ERR, "setsid() failed: %m");
        return 1;
    }

    if(-1 == chdir("/")){
        syslog(LOG_ERR, "chdir() failed: %m");
        return 1;
    }

    //TODO: figure out why if I enable these s.listen will fail, create a POC
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    if(0 != getuid()){
        qDebug () << "Current UID:" << getuid();
        syslog(LOG_ERR, "Must be root to switch user");
        return 1;
    }

    QByteArray msg;
    msg = Configuration::get("user", "root").toString().toLocal8Bit();
    const char *user = msg.constData();

    struct passwd *pw = getpwnam(user);

    if(NULL == pw){
        syslog(LOG_ERR, "Cannot find user %s, error: %m", user);
        return 1;
    }

    if(0 != setgid(pw->pw_gid)){
        syslog(LOG_ERR, "Cannot set gid to %i, error: %m", pw->pw_gid);
        return 1;
    }

    //set secondary groups

    if(0 != setuid(pw->pw_uid)){
        syslog(LOG_ERR, "Cannot set uid to %i, error: %m", pw->pw_uid);
        return 1;
    }

    syslog(LOG_INFO, "Set uid to %d and gid to %d", pw->pw_uid, pw->pw_gid);
#endif

    bool ok;
    int port = Configuration::get("port", 80).toInt(&ok);

    if(!ok){
        port = 80;
    }

    HTTPDaemon s;
    s.setObjectName("HTTPDaemon");

    if(!s.listen(QHostAddress(Configuration::get("address").toString()), port)){
        QByteArray msg;
        msg = s.errorString().toLocal8Bit();

        syslog(LOG_ERR, "Cannot start the server: %s", msg.constData());
        return 1;
    }
    else{
        QByteArray msg, msg1;
        msg = s.serverAddress().toString().toLocal8Bit();

        syslog(LOG_NOTICE, "Listening on %s:%i", msg.constData(), s.serverPort());

        msg = Configuration::get("documentroot").toString().toLocal8Bit();
        msg1 = Configuration::get("pluginroot").toString().toLocal8Bit();
        syslog(LOG_INFO, "Document root is in: %s\nPlugin root is in: %s", msg.constData(), msg1.constData());
    }

    return a.exec();
}
