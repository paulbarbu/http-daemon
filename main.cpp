#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QFileInfo>

#include "httpdaemon.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setObjectName("main app");
    QTextStream out(stdout);

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
        out << pluginRootError << endl;

        return a.exec();
    }

    QFileInfo f(args[pos+1]);

    if(f.isReadable()){
        pluginRoot = f.absoluteFilePath();
    }
    else{
        out << pluginRootError << endl;

        return a.exec();
    }

    HTTPDaemon s(docRoot, pluginRoot);

    s.setObjectName("HTTPServer");

    //parametrize the port
    if(!s.listen(QHostAddress::LocalHost, 8282)){
        out << "Cannot start the server: " << s.errorString() << endl;
    }
    else{
        out << "Listening on " << s.serverAddress().toString() << ":"
            << s.serverPort() << endl;
        out << "Document root is in: " << docRoot << endl
            << "Plugin root is in: " << pluginRoot << endl;
    }

    return a.exec();
}
