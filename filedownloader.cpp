#include <QByteArray>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "filedownloader.h"

FileDownloader::FileDownloader(QObject *parent) :
    QObject(parent)
{
    connect(&web_ctrl, SIGNAL (finished(QNetworkReply*)), this, SLOT (fileDownloaded(QNetworkReply*)));
}

FileDownloader::~FileDownloader() {}

void FileDownloader::makeCall(QUrl url) {
    QNetworkRequest request(url);
    web_ctrl.get(request);
}

void FileDownloader::fileDownloaded(QNetworkReply* reply) {
    downloaded_data = reply->readAll();
    reply->deleteLater();

    //emit a signal
    emit downloaded();
}

QByteArray FileDownloader::downloadedData() const {
    return downloaded_data;
}
