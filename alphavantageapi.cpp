#include <QtWidgets>
#include <QByteArray>
#include <QString>
#include <QFile>
#include <QIODevice>
#include <QUrl>
#include <QtMath>
#include <QRandomGenerator>

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
    QString url_format = QString("https://www.alphavantage.co/query?function=%1&symbol=%2&outputsize=full&apikey=9B0ZQFHBX9TQ4BJ3").arg(time_series_type).arg(ticker);
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

// https://quant.stackexchange.com/questions/42082/calculate-drift-of-brownian-motion-using-euler-method
float AlphaVantageAPI::calculateAverageReturn(QMap<QDateTime, StockDataElement> timeSeries) {
    float sumReturns = 0.0;  // Sum of all log returns
    int count = 0;           // Number of valid data points

    QDateTime previousDate;
    for (auto it = timeSeries.begin(); it != timeSeries.end(); ++it) {

        if (!previousDate.isNull()) {
            float currentPrice = it.value().getClose();
            float previousPrice = timeSeries[previousDate].getClose();
            sumReturns += qLn(currentPrice / previousPrice); // ln(P_t / P_(t-1))
            ++count;
        }
        previousDate = it.key();
    }

    if(count > 0) {
        return sumReturns;
    } else {
        return 0.0;
    }
}


float AlphaVantageAPI::calculateVolatility(QMap<QDateTime, StockDataElement> timeSeries) {
    QVector<float> logReturns;
    QDateTime previousDate;

    for (auto it = timeSeries.begin(); it != timeSeries.end(); ++it) {

        if (!previousDate.isNull()) {
            float currentPrice = it.value().getClose();
            float previousPrice = timeSeries[previousDate].getClose();
            logReturns.append(qLn(currentPrice / previousPrice));
        }
        previousDate = it.key();
    }

    // Calculate the mean of log returns
    // https://www.geeksforgeeks.org/accumulate-and-partial_sum-in-c-stl-numeric-header/
    float meanReturn = std::accumulate(logReturns.begin(), logReturns.end(), 0.0) / logReturns.size();

    // Calculate the variance
    float variance = 0.0;
    for (float ret : logReturns) {
        variance += qPow(ret - meanReturn, 2); // Sum of squared deviations
    }
    variance /= logReturns.size();

    return qSqrt(variance); // Standard deviation = volatility
}


QVector<QPointF> AlphaVantageAPI::simulateGBM(QString ticker, float T, int steps) {
    if (!stock_data_store.contains(ticker)) {
        qDebug() << "Stock data not found for ticker:" << ticker;
        return QVector<QPointF>();
    }

    StockData *stock = stock_data_store[ticker];
    QMap<QDateTime, StockDataElement> timeSeries = stock->getTimeSeries();

    if (timeSeries.isEmpty()) {
        qDebug() << "Time series data is empty for ticker:" << ticker;
        return QVector<QPointF>();
    }

    QVector<QPointF> gbmPath;
    float S0 = timeSeries.last().getClose();
    float mu = calculateAverageReturn(timeSeries); // drift
    float sigma = calculateVolatility(timeSeries); // volatility
    float dt = T / steps; // Time step
    float S = S0;

    for (int i = 0; i <= steps; ++i) {
        // Generate a random number
        float Z = QRandomGenerator::global()->generateDouble() * 2 - 1;
        Z *= 5;

        // Update stock price using the simplified GBM formula and append to path
        S = S * qExp((mu - 0.5 * sigma * sigma) * dt + sigma * qSqrt(dt) * Z);
        gbmPath.append(QPointF(i * dt, S));
    }

    return gbmPath;
}
