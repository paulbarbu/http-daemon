#include <QDebug>

#include "httpparser.h"

HTTPParser::HTTPParser()
{

}
//TODO: work with QByteArrays
QHash<QString, QString> HTTPParser::parsePostBody(const QString &postBody)
{
    QHash<QString, QString> retval;
    QStringList pairs = postBody.split("&", QString::SkipEmptyParts);

    foreach(QString pair, pairs){
        QStringList keyVal = pair.split("=");

        if(2 != keyVal.size()){
            return QHash<QString, QString>();
        }

        //TODO: URL-decode these
        retval.insert(keyVal[0].trimmed(), keyVal[1].trimmed());
    }

    return retval;
}

RequestData HTTPParser::parseRequest(QString request)
{
    RequestData requestData;
    requestData.valid = false;

    QStringList parts = request.replace("\r", "").split("\n\n", QString::SkipEmptyParts);

    if(parts.isEmpty()){
        return requestData;
    }

    QStringList fields = parts[0].split("\n", QString::SkipEmptyParts);

    qDebug() << "Fields" << fields;

    QStringList statusLine = fields[0].split(" ");

    if(3 != statusLine.size()){
        return requestData;
    }

    QStringList protocol = statusLine[2].split("/");
    bool ok;

    if(2 != protocol.size()){
        return requestData;
    }

    double ver = protocol[1].toDouble(&ok);

    if("HTTP" != protocol[0] || !ok || ver < 0.9 || ver > 1.1){
        return requestData;
    }

    requestData.method = statusLine[0];
    requestData.url.setUrl(statusLine[1]);
    requestData.protocol = protocol[0];
    requestData.protocolVersion = ver;

    fields.removeAt(0);
    int spacePos;
    foreach(QString line, fields){
        spacePos = line.indexOf(" ");

        requestData.fields.insert(line.left(spacePos-1),
                           QStringList(line.right(line.size()-spacePos-1)));
    }

    if(requestData.fields.contains("Host")){
        requestData.fields["Host"] = requestData.fields["Host"][0].split(":");
    }


    if("POST" == requestData.method && 2 == parts.size() &&
            !parts[1].isEmpty()){
        requestData.postData = parsePostBody(parts[1]);

        if(requestData.postData.isEmpty()){
            return requestData;
        }
    }

    qDebug() << "Request data:\n\tStatusLine:\n\t\tMethod: "
             << requestData.method << "\n\t\tUrl: "
             << requestData.url << "\n\t\tProtocol: "
             << requestData.protocol << "\n\t\tVer: "
             <<requestData.protocolVersion
             << "\n\tFields: " << requestData.fields
             << "\n\tpost: " << requestData.postData;

    requestData.valid = true;
    return requestData;
}
