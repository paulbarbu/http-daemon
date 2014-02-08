#include <QDebug>
#include <QHostAddress>

#include "httpparser.h"

HTTPParser::HTTPParser(QObject *parent) : QObject(parent), isParsedHeader(false)
{
    requestData.contentLength = -1;
}

void HTTPParser::parsePostData()
{
    if(data.isEmpty()){
        return;
    }

    QString postBody = data;
    QStringList pairs = postBody.split("&", QString::SkipEmptyParts);

    foreach(QString pair, pairs){
        QStringList keyVal = pair.split("=");

        if(2 != keyVal.size()){
            emit parseError("Invalid POST data!");
            return;
        }

        requestData.postData.insert(QUrl::fromPercentEncoding(keyVal[0].toUtf8()),
                QUrl::fromPercentEncoding(keyVal[1].toUtf8()));
    }

    bytesToParse = 0;
}

void HTTPParser::parseRequestHeader()
{
    QString request = data;
    QStringList fields = request.replace("\r", "").split("\n", QString::SkipEmptyParts);

    if(fields.isEmpty()){
        emit parseError("Empty request!");
        return;
    }

    QStringList statusLine = fields[0].split(" ");

    if(3 != statusLine.size()){
        emit parseError("Invalid status line!");
        return;
    }

    if("GET" != statusLine[0] && "POST" != statusLine[0] &&
            "HEAD" != statusLine[0]){
        emit parseError("Invalid method!");
        return;
    }

    if(statusLine[1].isEmpty()){
        emit parseError("Path cannot be empty!");
        return;
    }

    QStringList protocol = statusLine[2].split("/");
    bool ok;

    if(2 != protocol.size()){
        emit parseError("Invalid protocol!");
        return;
    }

    double ver = protocol[1].toDouble(&ok);

    if("HTTP" != protocol[0] || !ok || ver < 0.9 || ver > 1.1){
        emit parseError("Invalid protocol!");
        return;
    }

    requestData.url.setUrl(statusLine[1]);

    if(!requestData.url.isValid()){
        emit parseError("Invalid URL!");
        return;
    }

    requestData.method = statusLine[0];
    requestData.protocol = protocol[0];
    requestData.protocolVersion = ver;

    fields.removeAt(0);
    int spacePos;
    foreach(QString line, fields){
        spacePos = line.indexOf(" ");

        requestData.fields.insert(line.left(spacePos-1), line.right(line.size()-spacePos-1));
    }

    if(requestData.fields.contains("Host")){
        QStringList hostLine = requestData.fields["Host"].toString().split(":");

        requestData.port = hostLine[1].toUInt(&ok);
        if(!ok){
            emit parseError("Invalid port number!");
            return;
        }

        requestData.host = QHostAddress(hostLine[0]);
        requestData.fields.remove("Host");
    }

    if(requestData.fields.contains("Content-Length")){
        requestData.contentLength = requestData.fields["Content-Length"].toUInt(&ok);

        if(!ok){
            emit parseError("Invalid Content-Length value!");
            return;
        }

        requestData.fields.remove("Content-Length");
    }

    if(requestData.fields.contains("Cookie")){
        //replace spaces and semicolons with newlines so that the parsing is done properly
        //ugly hack, but it's needed since parseCookies() is designed to work on server-set "Set-Cookie:" headers,
        //not on "Cookie:" headers, set by clients
        requestData.cookieJar = QNetworkCookie::parseCookies(
            requestData.fields["Cookie"].toByteArray().replace(" ", "\n").replace(";", "\n"));

        if(requestData.cookieJar.empty()){
            emit parseError("Invalid Cookie value!");
            return;
        }

        requestData.fields.remove("Cookie");
    }

    bytesToParse = requestData.contentLength;

    isParsedHeader = true;
}

HTTPParser &HTTPParser::operator<<(const QString &chunk)
{
    return *this << chunk.toUtf8();
}

HTTPParser &HTTPParser::operator<<(const QByteArray &chunk)
{
    data.append(chunk);
    parse();

    return *this;
}

void HTTPParser::parse()
{
    if(!isParsedHeader &&
            (-1 != data.indexOf("\r\n\r\n") || -1 != data.indexOf("\n\n"))){
        parseRequestHeader();
        discardRequestHeader();
    }

    if(isParsedHeader && "POST" == requestData.method){
        if(bytesToParse < 0){
            //a POST request with no Content-Length is bogus as per standard
            emit parseError("POST request with erroneous Content-Length field!");
            return;
        }

        if(data.size() == bytesToParse){
            parsePostData();
        }
    }

    if(isParsedHeader && (0 == bytesToParse || "GET" == requestData.method ||
                          "HEAD" == requestData.method)){
        emit parsed(requestData);
    }
}

void HTTPParser::discardRequestHeader()
{
    QString lf = "\n\n";
    int crlfPos = data.indexOf("\r\n\r\n");
    int lfPos = data.indexOf("\n\n");

    if(-1 != crlfPos){
        lf = "\r\n\r\n";
        lfPos = crlfPos;
    }

    //discard the header from the request
    data = data.replace(lf, "").right(data.size()-lfPos-lf.size());
}
