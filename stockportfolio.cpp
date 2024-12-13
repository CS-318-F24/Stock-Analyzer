#include <QRandomGenerator>
#include <QPointF>

#include "stockportfolio.h"

StockPortfolio::StockPortfolio() {

    portfolio = QMap<QString, StockData>();
    allocation = QMap<QString, float>();
    investment_used = 0;
    available_funds = 0;
}

StockPortfolio::~StockPortfolio() {}

//=========================== public methods ===============================//

//============== helper operators ================//
QDataStream &operator<<(QDataStream &out, const StockPortfolio &portfolio) {
    out << portfolio.portfolio << portfolio.allocation << portfolio.investment_used << portfolio.available_funds;
    return out;
}


QDataStream &operator>>(QDataStream &in, StockPortfolio &portfolio) {
    in >> portfolio.portfolio >> portfolio.allocation >> portfolio.investment_used >> portfolio.available_funds;
    return in;
}


//============== portfolio metrics =================//
float StockPortfolio::expectedReturn() {
    float weighted_expected_return = 0;
    for(QString ticker : portfolio.keys()) {
        weighted_expected_return += portfolio[ticker].getExpectedReturn() * allocation[ticker];
    }
    return weighted_expected_return;
}

//================= access methods ====================//
void StockPortfolio::insert(StockData stock) {
    portfolio.insert(stock.getTicker(), stock);
}

void StockPortfolio::remove(QString ticker) {
    portfolio.remove(ticker);
}

bool StockPortfolio::contains(QString ticker) {
    return portfolio.contains(ticker);
}

StockData StockPortfolio::getStock(QString ticker) {
    return portfolio[ticker];
}

QList<QString> StockPortfolio::getStockList() {
    return portfolio.keys();
}

void StockPortfolio::allocateInvestment(QString ticker, float allocation_amount) {
    allocation.insert(ticker, allocation_amount);
    investment_used += allocation_amount;
}

float StockPortfolio::getAllocation(QString ticker) {
    return allocation[ticker];
}

float StockPortfolio::getInvestmentUsed() {
    return investment_used;
}

void StockPortfolio::setAvailableFunds(int funds) {
    available_funds = funds;
}

int StockPortfolio::getAvailableFunds() {
    return available_funds;
}


//=================================== statistical methods for GBM ===================================//

// https://quant.stackexchange.com/questions/42082/calculate-drift-of-brownian-motion-using-euler-method
float StockPortfolio::calculateAverageReturn(QMap<QDateTime, StockDataElement> timeSeries) {
    float sumReturns = 0.0;  // Sum of all log returns
    int count = 0;           // Number of valid data points

    QDateTime previousDate;
    for (auto it = timeSeries.begin(); it != timeSeries.end(); ++it) {

        if (!previousDate.isNull()) {
            float currentPrice = it.value().getClose();
            float previousPrice = timeSeries[previousDate].getClose();
            sumReturns += qLn(currentPrice / previousPrice); // ln(P_t / P_(t-1))
            ++count;
        }
        previousDate = it.key();
    }

    if(count > 0) {
        return sumReturns;
    } else {
        return 0.0;
    }
}


float StockPortfolio::calculateVolatility(QMap<QDateTime, StockDataElement> timeSeries) {
    QVector<float> logReturns;
    QDateTime previousDate;

    for (auto it = timeSeries.begin(); it != timeSeries.end(); ++it) {

        if (!previousDate.isNull()) {
            float currentPrice = it.value().getClose();
            float previousPrice = timeSeries[previousDate].getClose();
            logReturns.append(qLn(currentPrice / previousPrice));
        }
        previousDate = it.key();
    }

    // Calculate the mean of log returns
    // https://www.geeksforgeeks.org/accumulate-and-partial_sum-in-c-stl-numeric-header/
    float meanReturn = std::accumulate(logReturns.begin(), logReturns.end(), 0.0) / logReturns.size();

    // Calculate the variance
    float variance = 0.0;
    for (float ret : logReturns) {
        variance += qPow(ret - meanReturn, 2); // Sum of squared deviations
    }
    variance /= logReturns.size();

    return qSqrt(variance); // Standard deviation = volatility
}


QVector<QPointF> StockPortfolio::simulateGBM(QString ticker, float T, int steps) {
    if (!portfolio.contains(ticker)) {
        return QVector<QPointF>();
    }

    StockData stock = portfolio[ticker];
    QMap<QDateTime, StockDataElement> timeSeries = stock.getTimeSeries();

    if (timeSeries.isEmpty()) {
        return QVector<QPointF>();
    }

    QVector<QPointF> gbmPath;
    float S0 = timeSeries.last().getClose();
    float mu = calculateAverageReturn(timeSeries); // drift
    float sigma = calculateVolatility(timeSeries); // volatility
    float dt = T / steps; // Time step
    float S = S0;

    for (int i = 0; i <= steps; ++i) {
        // Generate a random number
        float Z = QRandomGenerator::global()->generateDouble() * 2 - 1;
        Z *= 5;

        // Update stock price using the simplified GBM formula and append to path
        S = S * qExp((mu - 0.5 * sigma * sigma) * dt + sigma * qSqrt(dt) * Z);
        gbmPath.append(QPointF(i * dt, S));
    }

    return gbmPath;
}

