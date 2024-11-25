#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <QMap>
#include <QDateTime>
#include <QJsonObject>

#include "stockdataelement.h"

class StockData
{

    QString ticker;
    QMap<QDateTime, StockDataElement> time_series;

    QString moving_avg_type;
    QMap<QDateTime, float> moving_avg_series;

public:
    StockData(QJsonObject _data);
    StockData();
    ~StockData();

    QString getTicker();
    QMap<QDateTime, StockDataElement> getTimeSeries();
    QMap<QDateTime, float> getMovingAvgSeries(QString type="fifty-day");

};

#endif // STOCKDATA_H
