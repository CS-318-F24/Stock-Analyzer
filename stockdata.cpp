#include "stockdata.h"

#include "stockdataelement.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

StockData::StockData(QJsonObject _data) : time_series()
{
    QJsonObject meta_data = _data["Meta Data"].toObject();
    ticker = meta_data["2. Symbol"].toString();

    QJsonValue time_series_daily = _data["Time Series (Daily)"];
    QJsonObject ts;
    if(time_series_daily.isObject()) {
        ts = time_series_daily.toObject();
    }

    for(const auto &[key, value] : ts) {
        Date candle_date = key;

        QJsonObject candle_obj = ts.value(key).toObject();
        float open = candle_obj["1. open"].toString().toFloat();
        float high = candle_obj["2. high"].toString().toFloat();
        float low = candle_obj["3. low"].toString().toFloat();
        float close = candle_obj["4. close"].toString().toFloat();
        float volume = candle_obj["5. volume"].toString().toFloat();

        StockDataElement el(open, high, low, close, volume);

        time_series.insert(key, el);
    }
}

StockData::StockData() {}


StockData::~StockData() {}
