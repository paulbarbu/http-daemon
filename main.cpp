#include <QCoreApplication>
#include <QTextStream>

#include "httpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    QTextStream out(stdout);
    HTTPServer s;

    if(!s.listen(QHostAddress::LocalHost, 8080)){
        out << "Cannot start the server: " << s.errorString() << endl;
    }
    else{
        out << "Listening on " << s.serverAddress().toString() << ":"
            << s.serverPort() << endl;
    }

    return a.exec();
}
