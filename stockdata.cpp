#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "stockdata.h"

#include "stockdataelement.h"

StockData::StockData(QJsonObject _data) : time_series(), moving_avg_series()
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
        QDateTime candle_date(date, time_m, Qt::TimeZone);

        QJsonObject candle_obj = ts.value(it.key()).toObject();
        float open = candle_obj["1. open"].toString().toFloat();
        float high = candle_obj["2. high"].toString().toFloat();
        float low = candle_obj["3. low"].toString().toFloat();
        float close = candle_obj["4. close"].toString().toFloat();
        float volume = candle_obj["5. volume"].toString().toFloat();

        StockDataElement el(candle_date, open, high, low, close, volume);

        time_series.insert(candle_date, el);
    }

    qDebug() << "stock data made for " << ticker << ".";
}

StockData::StockData() {}


StockData::~StockData() {}

QString StockData::getTicker() {
    return ticker;
}

QMap<QDateTime, StockDataElement> StockData::getTimeSeries() {
    return time_series;
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
