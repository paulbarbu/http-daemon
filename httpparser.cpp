#include <QDebug>
#include <QHostAddress>

#include "httpparser.h"

HTTPParser::HTTPParser(QObject *parent) : QObject(parent), isParsedHeader(false)
{
    requestData.contentLength = -1;
}

void HTTPParser::parsePostData()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "DATA:" << data;
    if(data.isEmpty()){
        return;
    }

    //TODO: take into account more content types
    if(requestData.contentType.contains("multipart/form-data")){
        requestData.rawPostData = data;
        bytesToParse = 0;
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

void HTTPParser::parseRequestHeader(const QByteArray &h)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "HEADER:" << h;
    QString header(h);

    QStringList fields = header.replace("\r", "").split("\n", QString::SkipEmptyParts);

    if(fields.isEmpty()){
        emit parseError("Empty request!");
        return;
    }

    QString err = parseStatusLine(fields[0]);
    if(!err.isEmpty()){
        emit parseError(err);
        return;
    }

    fields.removeAt(0);
    qDebug() << "FIELDS:" << fields;
    err = parseFields(fields);
    if(!err.isEmpty()){
        emit parseError(err);
        return;
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
    if(!isParsedHeader){
        int pos = data.indexOf("\r\n\r\n");
        int len = 4;

        if(-1 == pos){
            pos = data.indexOf("\n\n");
            len = 2;
        }

        if(-1 != pos){
            parseRequestHeader(data.left(pos));
            data = data.right(data.size()-pos-len);
        }
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

QString HTTPParser::parseFields(const QStringList &fields)
{
    bool ok;
    int colonPos;
    foreach(QString line, fields){
        colonPos = line.indexOf(":");

        if(-1 == colonPos){
            continue; //discard invalid fields
        }

        QString key = line.left(colonPos).trimmed();
        QString val = line.right(line.size()-colonPos-1).trimmed();

        if("host" == key.toLower()){
            if(val.size()){
                QStringList hostLine = val.split(":");

                if(hostLine.size() == 2){
                    requestData.port = hostLine[1].toUInt(&ok);
                    if(!ok){
                        return "Invalid port number!";
                    }
                }

                requestData.host = QHostAddress(hostLine[0]);
            }
        }
        else if("content-length" == key.toLower()){
            requestData.contentLength = val.toUInt(&ok);

            if(!ok){
                return "Invalid Content-Length value!";
            }
        }
        else if("content-type" == key.toLower()){
            requestData.contentType = val;
        }
        else if("cookie" == key.toLower()){
            //replace spaces and semicolons with newlines so that the parsing is done properly
            //ugly hack, but it's needed since parseCookies() is designed to work on server-set "Set-Cookie:" headers,
            //not on "Cookie:" headers, set by clients
            val.replace(" ", "\n").replace(";", "\n");
            requestData.cookieJar = QNetworkCookie::parseCookies(val.toLocal8Bit());

            if(requestData.cookieJar.empty()){
                return "Invalid Cookie value!";
            }
        }
        else{
            requestData.fields.insert(key, val);
        }
    }

    return "";
}

QString HTTPParser::parseStatusLine(const QString &statusLine)
{
    QStringList statusLineLst = statusLine.split(" ");

    if(3 != statusLineLst.size()){
        return "Invalid status line!";
    }

    if("GET" != statusLineLst[0] && "POST" != statusLineLst[0] &&
            "HEAD" != statusLineLst[0]){
        return "Invalid method!";
    }

    if(statusLineLst[1].isEmpty()){
        return "Path cannot be empty!";
    }

    QStringList protocol = statusLineLst[2].split("/");
    bool ok;

    if(2 != protocol.size()){
        return "Invalid protocol!";
    }

    double ver = protocol[1].toDouble(&ok);

    if("HTTP" != protocol[0] || !ok || ver < 0.9 || ver > 1.1){
        return "Invalid protocol!";
    }

    requestData.url.setUrl(statusLineLst[1]);

    if(!requestData.url.isValid()){
        return "Invalid URL!";
    }

    requestData.method = statusLineLst[0].toUpper();
    requestData.protocol = protocol[0];
    requestData.protocolVersion = ver;

    return "";
}
