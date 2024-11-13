#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject
{
    Q_OBJECT

    QNetworkAccessManager web_ctrl;
    QByteArray downloaded_data;

public:
    FileDownloader(QObject *parent = 0); //constructor
    virtual ~FileDownloader();

    QByteArray downloadedData() const;
    void makeCall(QUrl url);

signals:
    void downloaded();

public slots:
    void fileDownloaded(QNetworkReply *reply);

};

#endif // FILEDOWNLOADER_H
