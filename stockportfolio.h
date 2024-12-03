#ifndef STOCKPORTFOLIO_H
#define STOCKPORTFOLIO_H

#include <QMap>

#include "stockdata.h"
#include "stockdataelement.h"

class StockPortfolio
{
    QMap<QString, StockData*> portfolio;
    QMap<QString, float>  allocation; //relative allocation of investment (i.e. sum(allocation.values()) = 1)
    float investment_used; // Tracks the percentage of funds that has been allocated (sum of allocation values)
    int available_funds; // Amount the individual has to invest

    static float covariant(StockData* stock1, StockData* stock2);
    static float correlation(StockData* stock1, StockData* stock2);
    static QPair<QVector<float>, QVector<float>> commonTimeSeries(StockData* stock1, StockData* stock2, QString type="log");

    static float getMean(QVector<float> list);
    static float getStandardDeviation(QVector<float> list);


public:
    StockPortfolio();
    ~StockPortfolio();

    void insert(StockData *stock);
    void remove(QString ticker);
    QList<QString> getStocks();

    //void allocateInvestment(QVector<float> _allocation);
    void allocateInvestment(QString ticker, float allocation_amount);

    float expectedReturn();
    float risk();

    float getInvestmentUsed();

    void setAvailableFunds(int funds);
    int getAvailableFunds();
};

#endif // STOCKPORTFOLIO_H
