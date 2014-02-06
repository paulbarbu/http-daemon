#include <errno.h>

#include <QtGlobal>

#ifndef Q_OS_WIN32
    #include <unistd.h>
    #include <sys/stat.h>
    #include <syslog.h>
#endif

#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include "httpdaemon.h"
#include "logging.h"

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

    /*TODO: if I won't use boost::program_args here then I should do something
     *with the arguments since I don't like passing them all the way through
     *every class in the app
     */
    QStringList args(a.arguments());

    int pos = args.indexOf("--docroot");
    QString docRoot = "/tmp";

    if(-1 != pos){
        QFileInfo f(args[pos+1]);

        if(f.isReadable()){
            docRoot = f.absoluteFilePath();
        }
    }

    pos = args.indexOf("--pluginroot");
    QString pluginRoot;
    QString pluginRootError = "Please provide a readable path to the directory"
            " that holds the plugins using the --pluginroot argument!";

    if(-1 == pos){
        err << pluginRootError << endl;
        syslog(LOG_ERR, pluginRootError.toStdString().c_str());

        return 1;
    }

    QFileInfo f(args[pos+1]);

    if(f.isReadable()){
        pluginRoot = f.absoluteFilePath();
    }
    else{
        err << pluginRootError << endl;
        syslog(LOG_ERR, pluginRootError.toStdString().c_str());

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

    //TODO: figure out why if I enable these s.listen will fail
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
#endif

    HTTPDaemon s(docRoot, pluginRoot);
    s.setObjectName("HTTPDaemon");

    //TODO: parametrize the port
    if(!s.listen(QHostAddress::LocalHost, 8282)){
         syslog(LOG_ERR, "Cannot start the server: %s", s.errorString().toStdString().c_str());
         return 1;
    }
    else{
        syslog(LOG_NOTICE, "Listening on %s:%i", s.serverAddress().toString().toStdString().c_str(), s.serverPort());
        syslog(LOG_INFO, "Document root is in: %s\nPlugin root is in: %s", docRoot.toStdString().c_str(),
            pluginRoot.toStdString().c_str());
    }

    return a.exec();
}
