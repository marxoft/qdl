#include "formpost.h"

FormPostPlugin::FormPostPlugin(QObject *parent) :
    QObject(parent)
{
    userAgentS="";
    encodingS="utf-8";
    refererS="";
}

QString FormPostPlugin::userAgent() {
    return userAgentS;
}

void FormPostPlugin::setUserAgent(QString agent) {
    userAgentS=agent;
}

QString FormPostPlugin::referer() {
    return refererS;
}

void FormPostPlugin::setReferer(QString ref) {
    refererS=ref;
}

QString FormPostPlugin::encoding() {
    return encodingS;
}

void FormPostPlugin::setEncoding(QString enc) {
    if (enc=="utf-8" || enc=="ascii") {
        encodingS=enc;
    }
}

QByteArray FormPostPlugin::strToEnc(QString s) {
    if (encodingS=="utf-8") {
        return s.toUtf8();
    } else {
        return s.toLatin1();
    }
}

void FormPostPlugin::addField(QString name, QString value) {
    fieldNames.append(name);
    fieldValues.append(value);
}

void FormPostPlugin::addFile(QString fieldName, QByteArray file, QString name, QString mime) {
    files.append(file);
    fileFieldNames.append(fieldName);
    fileNames.append(name);
    fileMimes.append(mime);
}

void FormPostPlugin::addFile(QString fieldName, QString fileName, QString mime) {
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    QByteArray file=f.readAll();
    f.close();
    QString name;
    if (fileName.contains("/")) {
        int pos=fileName.lastIndexOf("/");
        name=fileName.right(fileName.length()-pos-1);
    } else if (fileName.contains("\\")) {
        int pos=fileName.lastIndexOf("\\");
        name=fileName.right(fileName.length()-pos-1);
    } else {
        name=fileName;
    }
    addFile(fieldName,file,name,mime);
}

QNetworkReply * FormPostPlugin::postData(QString url) {
    QString host;
    host=url.right(url.length()-url.indexOf("://")-3);
    host=host.left(host.indexOf("/"));
    QString crlf="\r\n";
    qsrand(QDateTime::currentDateTime().toTime_t());
    QString b=QVariant(qrand()).toString()+QVariant(qrand()).toString()+QVariant(qrand()).toString();
    QString boundary="---------------------------"+b;
    QString endBoundary=crlf+"--"+boundary+"--"+crlf;
    QString contentType="multipart/form-data; boundary="+boundary;
    boundary="--"+boundary+crlf;
    QByteArray bond=boundary.toLatin1();
    QByteArray send;
    bool first=true;

    for (int i=0; i<fieldNames.size(); i++) {
        send.append(bond);
        if (first) {
            boundary=crlf+boundary;
            bond=boundary.toLatin1();
            first=false;
        }
        send.append(QString("Content-Disposition: form-data; name=\""+fieldNames.at(i)+"\""+crlf).toLatin1());
        if (encodingS=="utf-8") send.append(QString("Content-Transfer-Encoding: 8bit"+crlf).toLatin1());
        send.append(crlf.toLatin1());
        send.append(strToEnc(fieldValues.at(i)));
    }
    for (int i=0; i<files.size(); i++) {
        send.append(bond);
        send.append(QString("Content-Disposition: form-data; name=\""+fileFieldNames.at(i)+"\"; filename=\""+fileNames.at(i)+"\""+crlf).toLatin1());
        send.append(QString("Content-Type: "+fileMimes.at(i)+crlf+crlf).toLatin1());
        send.append(files.at(i));
    }

    send.append(endBoundary.toLatin1());

    fieldNames.clear();
    fieldValues.clear();
    fileFieldNames.clear();
    fileNames.clear();
    fileMimes.clear();
    files.clear();

    QNetworkAccessManager * http=new QNetworkAccessManager(this);
    connect(http,SIGNAL(finished(QNetworkReply *)),this,SLOT(readData(QNetworkReply *)));
    QNetworkRequest request;
    request.setRawHeader("Host", host.toLatin1());
    if (userAgentS!="") request.setRawHeader("User-Agent", userAgentS.toLatin1());
    if (refererS!="") request.setRawHeader("Referer", refererS.toLatin1());
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType.toLatin1());
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(send.size()).toString());
    request.setUrl(QUrl(url));
    QNetworkReply * reply=http->post(request,send);
    return reply;
}

void FormPostPlugin::readData(QNetworkReply * r) {
    data=r->readAll();
}

QByteArray FormPostPlugin::response() {
    return data;
} 
