#ifndef FORMPOST_H
#define FORMPOST_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QProgressBar>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FormPostPlugin: public QObject
{ 
    
    Q_OBJECT

public:
    explicit FormPostPlugin(QObject *parent = 0);
    QString userAgent();
    void setUserAgent(QString agent);
    QString referer();
    void setReferer(QString ref);
    QString encoding();
    void setEncoding(QString enc);
    void addField(QString name, QString value);
    void addFile(QString fieldName, QByteArray file, QString name, QString mime);
    void addFile(QString fieldName, QString fileName, QString mime);
    QNetworkReply * postData(QString url);
    QByteArray response();
    
private:
    QByteArray data;
    QByteArray strToEnc(QString s);
    QString encodingS;
    QString userAgentS;
    QString refererS;
    QStringList fieldNames;
    QStringList fieldValues;
    QStringList fileFieldNames;
    QStringList fileNames;
    QStringList fileMimes;
    QList<QByteArray> files;
    
private slots:
    void readData(QNetworkReply * r);
};

#endif  
