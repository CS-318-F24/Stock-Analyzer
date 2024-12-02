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
    QVector<float> close_price_list; //stock close price as vector

    QVector<float> returns; //percentage returns close-to-close
    QVector<float> log_returns; //log-returns close-to-close
    float expected_return; //mean percent return
    float risk; //standard deviation of returns

    //for graphing
    QString moving_avg_type; //50 or 200-day
    QMap<QDateTime, float> moving_avg_series; //close price moving average time series

    void makeReturns();
    void makeLogReturns();
    void calculateExpectedReturn(QString type="log");
    void calculateRisk();

public:
    StockData(QJsonObject _data);
    StockData();
    ~StockData();

    QString getTicker();
    QMap<QDateTime, StockDataElement> getTimeSeries();
    QMap<QDateTime, float> getMovingAvgSeries(QString type="fifty-day");
    QVector<float> getSimpleReturns();
    QVector<float> getLogReturns();
    float getExpectedReturn();
    float getRisk();


    //old, may deprecate
    static QVector<QPair<float, float>> longestCommonSubsequence(StockData stock1, StockData stock2);

    static float getCorrealationCoefficient(StockData stock1, StockData stock2);

};

#endif // STOCKDATA_H
