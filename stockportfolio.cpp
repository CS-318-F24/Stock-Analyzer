#include "stockportfolio.h"

StockPortfolio::StockPortfolio() {

    portfolio = QMap<QString, StockData*>();
    allocation = QMap<QString, float>();
    investment_used = 0;
    available_funds = 0;
}

StockPortfolio::~StockPortfolio() {}


//========================== private helpers ==========================//

//static helper method for getting the max length common time series vector from two stocks
/*
 * 'stock1' and 'stock2' are StockData objects for which a common time series is desired
 * 'type' refers to what kind of common time series is requested (defaults
 *      to type 'log' which returns the common log returns time series)
 * return val: returns a QPair of QVectors which contain the common time series data of specified 'type'
 *
 */
QPair<QVector<float>, QVector<float>> StockPortfolio::commonTimeSeries(StockData* stock1, StockData* stock2, QString type) {
    QVector<float> common_data_stock1;
    QVector<float> common_data_stock2;

    QMap<QDateTime, StockDataElement> ts_reference;
    ts_reference = stock1->getTimeSeries().size() <= stock2->getTimeSeries().size() ? stock1->getTimeSeries(): stock2->getTimeSeries();

    QVector<float> data_reference_stock1;
    QVector<float> data_reference_stock2;
    if(type == "log") {
        data_reference_stock1 = stock1->getLogReturns();
        data_reference_stock2 = stock2->getLogReturns();
    } else if(type == "simple") {
        data_reference_stock1 = stock1->getSimpleReturns();
        data_reference_stock2 = stock2->getSimpleReturns();
    } else {
        qDebug() << "commonTimeSeries(): type" << type << "not supported: supports types 'log' and 'simple'";
        return QPair<QVector<float>, QVector<float>>();
    }

    QMap<QDateTime, StockDataElement> ts_stock1 = stock1->getTimeSeries();
    QMap<QDateTime, StockDataElement> ts_stock2 = stock2->getTimeSeries();
    int counter = 0;
    for(QMap<QDateTime, StockDataElement>::const_iterator it = ts_reference.cbegin(); it != ts_reference.cend(); ++it) {
        if(it == ts_reference.cbegin() || counter >= ts_reference.size()) {
            continue;
            ++counter;
        }
        if(ts_stock1.contains(it.key()) && ts_stock2.contains(it.key())) {
            qDebug() << "  both stocks have data for" << it.key();
            common_data_stock1.append(data_reference_stock1[counter]);
            common_data_stock2.append(data_reference_stock2[counter]);
            qDebug() << "\t" << counter;
        }
        ++counter;
    }

    QPair<QVector<float>, QVector<float>> common_data(common_data_stock1, common_data_stock2);
    return common_data;
}

//general mean getter for a vector of floats
float StockPortfolio::getMean(QVector<float> list) {
    float total = 0;
    for(int i = 0; i < list.size(); ++i) {
        total += list[i];
    }

    return total / list.size();
}

//general standard deviation getter for a vector of floats
float StockPortfolio::getStandardDeviation(QVector<float> list) {
    if (list.isEmpty()) {
        //avoid division by zero
        return 0.0f;
    }

    float sum_of_squares = 0.0f;

    for (float value : list) {
        float deviation = value - getMean(list);
        sum_of_squares += deviation * deviation;
    }

    // Standard deviation formula: sqrt((Σ(xi - mean)^2) / N)
    float variance = sum_of_squares / list.size();
    return qSqrt(variance);
}


//general correlation coefficient calculator for two StockData objects
float StockPortfolio::correlation(StockData *stock1, StockData *stock2) {
    //get common data range
    QPair<QVector<float>, QVector<float>> common_data = commonTimeSeries(stock1, stock2);
    QVector<float> log_returns1 = common_data.first;
    QVector<float> log_returns2 = common_data.second;

    float mean1 = getMean(log_returns1);
    float mean2 = getMean(log_returns2);

    float log_product = 1;
    for(int i = 0; i < log_returns1.size(); ++i) {
        log_product *= (log_returns1[i] * log_returns2[i]);
    }

    float numerator = log_product - (mean1 * mean2);

    QVector<float> log_returns1_squared(log_returns1.size());
    QVector<float> log_returns2_squared(log_returns2.size());
    for(int i = 0; i < log_returns1.size(); ++i) {
        log_returns1_squared[i] = log_returns1[i] * log_returns1[i];
        log_returns2_squared[i] = log_returns2[i] * log_returns2[i];
    }
    float mean_of_squares1 = getMean(log_returns1_squared);
    float mean_of_squares2 = getMean(log_returns2_squared);

    float mean1_squared = mean1 * mean1;
    float mean2_squared = mean2 * mean2;

    float denominator = std::sqrt((mean_of_squares1 - mean1_squared) * (mean_of_squares2 - mean2_squared));

    return numerator / denominator;
}

//helper for getting the covariant
float StockPortfolio::covariant(StockData *stock1, StockData *stock2) {
    float correlation_coefficient = correlation(stock1, stock2);
    float std_dev1 = getStandardDeviation(stock1->getLogReturns());
    float std_dev2 = getStandardDeviation(stock2->getLogReturns());

    return (correlation_coefficient * (std_dev1 * std_dev2));
}


//=========================== public methods ===============================//

void StockPortfolio::insert(StockData *stock) {
    portfolio.insert(stock->getTicker(), stock);
}

void StockPortfolio::remove(QString ticker) {
    int removed = portfolio.remove(ticker);
    qDebug() << "1 if removed" << ticker << ":" << removed;
}

StockData *StockPortfolio::getStock(QString ticker) {
    return portfolio[ticker];
}

QList<QString> StockPortfolio::getStockList() {
    return portfolio.keys();
}

void StockPortfolio::allocateInvestment(QString ticker, float allocation_amount) {
    allocation.insert(ticker, allocation_amount);
    investment_used += allocation_amount;
    qDebug() << "Total investment used:" << investment_used;
    qDebug() << "Investment by stock:" << allocation;
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
