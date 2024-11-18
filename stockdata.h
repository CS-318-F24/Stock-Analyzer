#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <QMap>
#include <QJsonObject>

#include "stockdataelement.h"

class StockData
{

    QString ticker;
    QMap<Date, StockDataElement> time_series;

public:
    StockData(QJsonObject _data);
    StockData();
    ~StockData();

    QString getTicker();
    QMap<Date, StockDataElement> *getTimeSeries();

};

#endif // STOCKDATA_H
