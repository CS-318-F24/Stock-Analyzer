#ifndef STOCKDATA_H
#define STOCKDATA_H

#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QJsonObject>

#include "stockdataelement.h"

class StockData
{

    QString ticker; //stock ticker symbol
    QMap<QDateTime, StockDataElement> time_series; //stock close price time series

    QString moving_avg_type; //50 or 200-day
    QMap<QDateTime, float> moving_avg_series; //close price moving average time series

public:
    StockData(QJsonObject _data);
    StockData();
    ~StockData();

    QString getTicker();
    QMap<QDateTime, StockDataElement> getTimeSeries();
    QMap<QDateTime, float> getMovingAvgSeries(QString type="fifty-day");


    static QVector<QPair<float, float>> getLogReturns(QVector<QPair<float, float>> close_prices);

    static QVector<QPair<float, float>> longestCommonSubsequence(StockData stock1, StockData stock2);

    static float getCorrealationCoefficient(StockData stock1, StockData stock2);

};

#endif // STOCKDATA_H
