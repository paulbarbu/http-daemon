#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>

#include "httpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //TODO: boost::program_args
    QStringList args(a.arguments());

    int pos = args.indexOf("--docroot");
    QString docRoot = "/tmp";

    if(-1 != pos){
        docRoot = args[pos+1];
    }

    HTTPServer s(docRoot);
    QTextStream out(stdout);

    if(!s.listen(QHostAddress::LocalHost, 8080)){
        out << "Cannot start the server: " << s.errorString() << endl;
    }
    else{
        out << "Listening on " << s.serverAddress().toString() << ":"
            << s.serverPort() << endl;
        out << "Documnet root is in: " << docRoot << endl;
    }

    return a.exec();
}
