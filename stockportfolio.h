#ifndef STOCKPORTFOLIO_H
#define STOCKPORTFOLIO_H

#include <QMap>

#include "stockdata.h"
#include "stockdataelement.h"

class StockPortfolio
{
    QMap<QString, StockData> portfolio;
    QMap<QString, float>  allocation; //relative allocation of investment (i.e. sum(allocation.values()) = 1)
    float investment_used; // Tracks the percentage of funds that has been allocated (sum of allocation values)
    int available_funds; // Amount the individual has to invest


public:
    StockPortfolio();
    ~StockPortfolio();

    void insert(StockData stock);
    void remove(QString ticker);
    bool contains(QString ticker);
    StockData getStock(QString ticker);
    QList<QString> getStockList();

    //void allocateInvestment(QVector<float> _allocation);
    void allocateInvestment(QString ticker, float allocation_amount);
    float getAllocation(QString ticker);

    float expectedReturn();

    float getInvestmentUsed();

    void setAvailableFunds(int funds);
    int getAvailableFunds();

    friend QDataStream &operator<<(QDataStream &out, const StockPortfolio &portfolio);
    friend QDataStream &operator>>(QDataStream &in, StockPortfolio &portfolio);


    //GBM methods
    QVector<QPointF> simulateGBM(QString ticker, float T, int steps);
    float calculateVolatility(QMap<QDateTime, StockDataElement> timeSeries);
    float calculateAverageReturn(QMap<QDateTime, StockDataElement> timeSeries);
};

#endif // STOCKPORTFOLIO_H
