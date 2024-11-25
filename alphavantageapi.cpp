#include <QtWidgets>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QIODevice>
#include <QUrl>

#include "filedownloader.h"
#include "alphavantageapi.h"
#include "stockdata.h"
#include "stockdataelement.h"


AlphaVantageAPI::AlphaVantageAPI(QString _time_series_type) : stock_data_store() {
    time_series_type = _time_series_type;
}

AlphaVantageAPI::~AlphaVantageAPI() {}

void AlphaVantageAPI::requestStockData(QString ticker) {
    if(this->stock_data_store[ticker]) {
        emit savedRequestedStockData();
        qDebug() << "stock data is cached: no API call necessary for " << ticker;
        return;
    }

    json_ctrl = new FileDownloader();
    curr_ticker = ticker;
    connect(json_ctrl, &FileDownloader::downloaded, this, &AlphaVantageAPI::addTimeSeries);
    QString url_format = QString("https://www.alphavantage.co/query?function=%1&symbol=%2&outputsize=full&apikey=EQ80DAAGFVN4J0QH").arg(time_series_type).arg(ticker);
    QUrl json_url(url_format);
    json_ctrl->makeCall(json_url);
}


void AlphaVantageAPI::addTimeSeries() {
    QByteArray time_series_data = json_ctrl->downloadedData();
    QJsonDocument time_series_doc = QJsonDocument::fromJson(time_series_data);

    QJsonObject time_series_obj;
    if(time_series_doc.isObject()) {
        time_series_obj = time_series_doc.object();
    }


    StockData *requested_stock_data = new StockData(time_series_obj);

    stock_data_store.insert(requested_stock_data->getTicker(), requested_stock_data);
    //this->saveJSON();

    emit savedRequestedStockData();
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
        //emit savedRequestedStockData();
    } else {
        qDebug() << "Error opening file for writing!";
    }
}


// getter for time series type
QString AlphaVantageAPI::type() {
    return time_series_type;
}

// getter for stock data
StockData *AlphaVantageAPI::getStockData(QString ticker) {
    if(stock_data_store.contains(ticker)){
        return stock_data_store[ticker];
    } else {
        qDebug() << "error: data not found for" << ticker << "\nplease fetch data first";
        return NULL;
    }
}
