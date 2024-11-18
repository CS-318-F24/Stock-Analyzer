#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "stockdata.h"

#include "stockdataelement.h"

StockData::StockData(QJsonObject _data) : time_series()
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
        QDateTime close_datetime(date, time_m, Qt::TimeZone);
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
}

StockData::StockData() {}


StockData::~StockData() {}

QString StockData::getTicker() {
    return ticker;
}

QMap<QDateTime, StockDataElement> *StockData::getTimeSeries() {
    return &time_series;
}
