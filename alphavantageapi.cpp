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


AlphaVantageAPI::AlphaVantageAPI(QString _time_series_type, bool _json_saving) : stock_data_store() {
    time_series_type = _time_series_type;
    json_saving = _json_saving;
}

AlphaVantageAPI::~AlphaVantageAPI() {}


//slot for making api call to Alpha Vantage endpoint
void AlphaVantageAPI::requestStockData(QString ticker) {
    if(this->stock_data_store.contains(ticker)) {
        emit savedRequestedStockData(ticker);
        qDebug() << "stock data is cached: no API call necessary for " << ticker;
        return;
    }

    json_ctrl = new FileDownloader();
    connect(json_ctrl, &FileDownloader::downloaded, this, &AlphaVantageAPI::addTimeSeries);
    QString url_format = QString("https://www.alphavantage.co/query?function=%1&symbol=%2&outputsize=full&apikey=EQ80DAAGFVN4J0QH").arg(time_series_type).arg(ticker);
    QUrl json_url(url_format);
    json_ctrl->makeCall(json_url);
}


//slot for adding time series data after a stock request is downloaded and optionally saving
void AlphaVantageAPI::addTimeSeries() {
    QByteArray time_series_data = json_ctrl->downloadedData();
    QJsonDocument time_series_doc = QJsonDocument::fromJson(time_series_data);

    QJsonObject time_series_obj;
    if(time_series_doc.isObject()) {
        time_series_obj = time_series_doc.object();
    }

    StockData *requested_stock_data = new StockData(time_series_obj);

    stock_data_store.insert(requested_stock_data->getTicker(), requested_stock_data);
    if(json_saving) {
        this->saveJSON(requested_stock_data->getTicker());
    }

    emit savedRequestedStockData(requested_stock_data->getTicker());
}

// (slot) for saving downloaded stock data to user specified location
void AlphaVantageAPI::saveJSON(QString ticker) {
    QByteArray json_data = json_ctrl->downloadedData();
    //qDebug() << json_data;
    //QJsonDocument json_doc = QJsonDocument::fromJson(json_data);

    QString path = save_loc;
    QFile test_json(path);
    if (test_json.open(QIODevice::WriteOnly)) {
        test_json.write(json_data);
        test_json.close();
        qDebug() << "JSON data saved to file successfully!";
    } else {
        qDebug() << "Error opening file for writing!";
    }
}


// setter for if stock data is automatically saved
void AlphaVantageAPI::setSaving(bool _json_saving) {
    json_saving = _json_saving;
}

// setter for save location
void AlphaVantageAPI::setSaveLocation(QString _save_location) {
    save_loc = _save_location;
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

int AlphaVantageAPI::numStoredStocks() {
    return stock_data_store.size();
}

QVector<QString> AlphaVantageAPI::stockList() {
    return stock_data_store.keys();
}
