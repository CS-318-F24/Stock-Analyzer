#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVector>

#include "stockdata.h"

#include "stockdataelement.h"

StockData::StockData(QJsonObject _data) : time_series(), close_price_list(), returns(), moving_avg_series()
{
    QJsonObject meta_data = _data["Meta Data"].toObject();
    ticker = meta_data["2. Symbol"].toString();

    QJsonValue time_series_daily = _data["Time Series (Daily)"];
    QJsonObject ts;
    if(time_series_daily.isObject()) {
        ts = time_series_daily.toObject();
    }

    for(QJsonObject::iterator it = ts.begin(); it != ts.end(); ++it)  {
        QString date_string = it.key();
        int year = date_string.left(4).toInt();
        int month = date_string.mid(5, 2).toInt();
        int day = date_string.right(2).toInt();
        QDate date(year, month, day);
        QTime time_m(16, 0); //4pm for market close
        QDateTime candle_date(date, time_m);

        QJsonObject candle_obj = ts.value(it.key()).toObject();
        float open = candle_obj["1. open"].toString().toFloat();
        float high = candle_obj["2. high"].toString().toFloat();
        float low = candle_obj["3. low"].toString().toFloat();
        float close = candle_obj["4. close"].toString().toFloat();
        float volume = candle_obj["5. volume"].toString().toFloat();

        StockDataElement el(candle_date, open, high, low, close, volume);

        time_series.insert(candle_date, el);
        close_price_list.append(close);
    }

    qDebug() << "stock data made for " << ticker << ".";

    //make percent returns list and calculate mean (expected) percent return
    this->makeReturns();
    this->makeLogReturns();
    this->calculateExpectedReturn();
    this->calculateRisk();
}

StockData::StockData() {}

StockData::~StockData() {}


//helper for creating returns list
void StockData::makeReturns() {
    returns.resize(close_price_list.size() - 1);
    for(int i = 1; i < close_price_list.size(); ++i) {
        returns[i - 1] = ((close_price_list[i] - close_price_list[i - 1]) / close_price_list[i]) * 100;
    }
}

//helper for creating log-returns
void StockData::makeLogReturns() {
    log_returns.resize(close_price_list.size() - 1);
    for(int i = 1; i < close_price_list.size(); ++i) {
        log_returns[i - 1] = qLn(close_price_list[i]) - qLn(close_price_list[i - 1]);
    }
}

//helper for caculating expected returns
void StockData::calculateExpectedReturn(QString type) {
    float cumlative_returns = 0;
    if(type == "log") {
        for(int i = 0; i < log_returns.size(); ++i) {
            cumlative_returns += log_returns[i];
        }
    } else if(type == "simple") {
        for(int i = 0; i < returns.size(); ++i) {
            cumlative_returns += returns[i];
        }
    }

    expected_return = cumlative_returns / returns.size(); //expected returns are simple average
}

//helper for calculating risk
void StockData::calculateRisk() {
    if (returns.isEmpty()) {
        //avoid division by zero
        risk = 0.0f;
    }

    float sum_of_squares = 0.0f;

    for (float value : returns) {
        float deviation = value - expected_return;
        sum_of_squares += deviation * deviation;
    }

    // Standard deviation formula: sqrt((Σ(xi - mean)^2) / N)
    float variance = sum_of_squares / returns.size();
    risk = qSqrt(variance);
}


//==============================public methods==============================//

QString StockData::getTicker() {
    return ticker;
}

QMap<QDateTime, StockDataElement> StockData::getTimeSeries() {
    return time_series;
}

QVector<float> StockData::getSimpleReturns() {
    return returns;
}

float StockData::getExpectedReturn() {
    return expected_return;
}

QVector<float> StockData::getLogReturns() {
    return log_returns;
}

float StockData::getRisk() {
    return risk;
}


QMap<QDateTime, float> StockData::getMovingAvgSeries(QString type) {
    if(!(type == "fifty-day" || type == "two-hundred-day")) {
        qDebug() << "error: moving average range not supported; supports 'fifty-day' and 'two-hundred-day' moving average.";
        return QMap<QDateTime, float>(); //return empty map
    }
    if(!(moving_avg_series.isEmpty()) && moving_avg_type == type) {
        return moving_avg_series;
    }
    moving_avg_type = type;
    int window_size;
    if(moving_avg_type == "fifty-day") {
        window_size = 50;
    } else {
        window_size = 200;
    }

    qDebug() << "getting moving average...";

    QList<float> window(window_size);
    for(QMap<QDateTime, StockDataElement>::const_iterator it = time_series.constBegin(); it != time_series.constEnd(); ++it) {
        window.append(it.value().getClose());
        if(window.size() < window_size) {
            continue;
        }

        //calculate avg over window
        float moving_avg = 0;
        for(int i = 0; i < window_size; ++i) {
            moving_avg += window[i];
        }
        moving_avg = moving_avg / window_size;
        moving_avg_series[it.key()] = moving_avg;
        //qDebug() << "moving average at " << it.key() << " is " << moving_avg;

        window.removeFirst();
    }

    return moving_avg_series;
}
