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

public:
    FileDownloader(QObject *parent = 0); //constructor
    virtual ~FileDownloader();
    QByteArray downloadedData() const;
    void makeCall(QUrl url);

signals:
    void downloaded();

public slots:
    void fileDownloaded(QNetworkReply *reply);

private:
    QNetworkAccessManager web_ctrl;
    QByteArray downloaded_data;
};

#endif // FILEDOWNLOADER_H
