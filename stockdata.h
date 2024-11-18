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

public:
    StockData(QJsonObject _data);
    StockData();
    ~StockData();

    QString getTicker();
    QMap<QDateTime, StockDataElement> *getTimeSeries();

};

#endif // STOCKDATA_H
