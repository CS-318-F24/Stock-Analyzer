#include <QtWidgets>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QIODevice>
#include <QUrl>

#include "filedownloader.h"
#include "alphavantageapi.h"


AlphaVantageAPI::AlphaVantageAPI(QString _time_series) {
    time_series = _time_series;
}

AlphaVantageAPI::~AlphaVantageAPI() {}

void AlphaVantageAPI::requestStockData(QString ticker) {
    json_ctrl = new FileDownloader();
    curr_ticker = ticker;
    connect(json_ctrl, &FileDownloader::downloaded, this, &AlphaVantageAPI::saveJSON);
    QString url_format = QString("https://www.alphavantage.co/query?function=%1&symbol=%2&outputsize=full&apikey=EQ80DAAGFVN4J0QH").arg(time_series).arg(ticker);
    QUrl json_url(url_format);
    json_ctrl->makeCall(json_url);
}

void AlphaVantageAPI::saveJSON() {
    QByteArray json_data = json_ctrl->downloadedData();
    //qDebug() << json_data;
    //QJsonDocument json_doc = QJsonDocument::fromJson(json_data);

    QString path = QString("/Users/ottoq/Documents/Middlebury/Computer_Science/CS318/stock_data/%1.json").arg(curr_ticker);
    QFile test_json(path);
    if (test_json.open(QIODevice::WriteOnly)) {
        test_json.write(json_data);
        test_json.close();
        qDebug() << "JSON data saved to file successfully!";
        emit savedRequestedStockData();
    } else {
        qDebug() << "Error opening file for writing!";
    }
}
