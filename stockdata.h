#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <QMap>
#include <QJsonObject>

#include "stockdataelement.h"

typedef QString Date;

class StockData
{

    QString ticker;
    QMap<Date, StockDataElement> time_series;


public:
    StockData(QJsonObject _data);
    StockData();
    ~StockData();
};

#endif // STOCKDATA_H
