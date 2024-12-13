#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVector>

#include "stockdata.h"

#include "stockdataelement.h"

StockData::StockData(QJsonObject _data) : time_series(), close_price_list(), returns(), moving_avg_series()
{
    QJsonObject meta_data = _data["Meta Data"].toObject();
    ticker = meta_data["2. Symbol"].toString();

    QJsonValue time_series_daily = _data["Time Series (Daily)"];
    QJsonObject ts;
    if(time_series_daily.isObject()) {
        ts = time_series_daily.toObject();
    }

    for(QJsonObject::iterator it = ts.begin(); it != ts.end(); ++it)  {
        QString date_string = it.key();
        int year = date_string.left(4).toInt();
        int month = date_string.mid(5, 2).toInt();
        int day = date_string.right(2).toInt();
        QDate date(year, month, day);
        QTime time_m(16, 0); //4pm for market close
        QDateTime candle_date(date, time_m);

        QJsonObject candle_obj = ts.value(it.key()).toObject();
        float open = candle_obj["1. open"].toString().toFloat();
        float high = candle_obj["2. high"].toString().toFloat();
        float low = candle_obj["3. low"].toString().toFloat();
        float close = candle_obj["4. close"].toString().toFloat();
        float volume = candle_obj["5. volume"].toString().toFloat();

        StockDataElement el(candle_date, open, high, low, close, volume);

        time_series.insert(candle_date, el);
        close_price_list.append(close);
    }

    //make percent returns list and calculate mean (expected) percent return
    this->makeReturns();
    this->makeLogReturns();
    this->calculateExpectedReturn();
    this->calculateRisk();
}

StockData::StockData() {}

StockData::~StockData() {}

//======================== contructor helpers ========================//
//helper for creating returns list
void StockData::makeReturns() {
    returns.resize(close_price_list.size() - 1);
    for(int i = 1; i < close_price_list.size(); ++i) {
        returns[i - 1] = ((close_price_list[i] - close_price_list[i - 1]) / close_price_list[i]) * 100;
    }
}

//helper for creating log-returns
void StockData::makeLogReturns() {
    log_returns.resize(close_price_list.size() - 1);
    for(int i = 1; i < close_price_list.size(); ++i) {
        log_returns[i - 1] = qLn(close_price_list[i]) - qLn(close_price_list[i - 1]);
    }
}

//helper for caculating expected returns
void StockData::calculateExpectedReturn(QString type) {
    float cumlative_returns = 0;
    if(type == "log") {
        for(int i = 0; i < log_returns.size(); ++i) {
            cumlative_returns += log_returns[i];
        }
    } else if(type == "simple") {
        for(int i = 0; i < returns.size(); ++i) {
            cumlative_returns += returns[i];
        }
    }

    expected_return = cumlative_returns / returns.size(); //expected returns are simple average
}

//helper for calculating risk
void StockData::calculateRisk() {
    if (returns.isEmpty()) {
        //avoid division by zero
        risk = 0.0f;
    }

    float sum_of_squares = 0.0f;

    for (float value : returns) {
        float deviation = value - expected_return;
        sum_of_squares += deviation * deviation;
    }

    // Standard deviation formula: sqrt((Σ(xi - mean)^2) / N)
    float variance = sum_of_squares / returns.size();
    risk = qSqrt(variance);
}

//================================ static helpers =================================//
//static helper method for getting the max length common time series vector from two stocks
/*
 * 'stock1' and 'stock2' are StockData objects for which a common time series is desired
 * 'type' refers to what kind of common time series is requested (defaults
 *      to type 'log' which returns the common log returns time series)
 * return val: returns a QPair of QVectors which contain the common time series data of specified 'type'
 *
 */
QPair<QVector<float>, QVector<float>> StockData::commonTimeSeries(StockData stock1, StockData stock2, QString type) {
    QVector<float> common_data_stock1;
    QVector<float> common_data_stock2;

    QMap<QDateTime, StockDataElement> ts_reference;
    ts_reference = stock1.getTimeSeries().size() <= stock2.getTimeSeries().size() ? stock1.getTimeSeries(): stock2.getTimeSeries();

    QVector<float> data_reference_stock1;
    QVector<float> data_reference_stock2;
    if(type == "log") {
        data_reference_stock1 = stock1.getLogReturns();
        data_reference_stock2 = stock2.getLogReturns();
    } else if(type == "simple") {
        data_reference_stock1 = stock1.getSimpleReturns();
        data_reference_stock2 = stock2.getSimpleReturns();
    } else {
        return QPair<QVector<float>, QVector<float>>();
    }

    QMap<QDateTime, StockDataElement> ts_stock1 = stock1.getTimeSeries();
    QMap<QDateTime, StockDataElement> ts_stock2 = stock2.getTimeSeries();
    int counter = 0;
    for(QMap<QDateTime, StockDataElement>::const_iterator it = ts_reference.cbegin(); it != ts_reference.cend(); ++it) {
        if(it == ts_reference.cbegin() || counter >= ts_reference.size()) {
            continue;
            ++counter;
        }
        if(ts_stock1.contains(it.key()) && ts_stock2.contains(it.key())) {
            common_data_stock1.append(data_reference_stock1[counter]);
            common_data_stock2.append(data_reference_stock2[counter]);
        }
        ++counter;
    }

    QPair<QVector<float>, QVector<float>> common_data(common_data_stock1, common_data_stock2);
    return common_data;
}


//general mean getter for a vector of floats
float StockData::getMean(QVector<float> list) {
    float total = 0;
    for(int i = 0; i < list.size(); ++i) {
        total += list[i];
    }

    return total / list.size();
}

//general standard deviation getter for a vector of floats
float StockData::getStandardDeviation(QVector<float> list) {
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
float StockData::correlation(StockData stock1, StockData stock2) {
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
float StockData::covariant(StockData stock1, StockData stock2) {
    float correlation_coefficient = correlation(stock1, stock2);
    float std_dev1 = getStandardDeviation(stock1.getLogReturns());
    float std_dev2 = getStandardDeviation(stock2.getLogReturns());

    return (correlation_coefficient * (std_dev1 * std_dev2));
}


//==============================public methods==============================//

//============= helper operators =============//
//helper operators (for time series)

//class operator
QDataStream &operator<<(QDataStream &out, const StockData &stock_data) {
    out << stock_data.ticker << stock_data.time_series << stock_data.close_price_list << stock_data.returns << stock_data.log_returns << stock_data.expected_return << stock_data.risk;
    return out;
}

QDataStream &operator>>(QDataStream &in, StockData &stock_data) {
    in >> stock_data.ticker >> stock_data.time_series >> stock_data.close_price_list >> stock_data.returns >> stock_data.log_returns >> stock_data.expected_return >> stock_data.risk;
    return in;
}


//================ getters ================//
QString StockData::getTicker() {
    return ticker;
}

QMap<QDateTime, StockDataElement> StockData::getTimeSeries() {
    return time_series;
}

QVector<float> StockData::getSimpleReturns() {
    return returns;
}

float StockData::getExpectedReturn() {
    return expected_return;
}

QVector<float> StockData::getLogReturns() {
    return log_returns;
}

float StockData::getRisk() {
    return risk;
}


QMap<QDateTime, float> StockData::getMovingAvgSeries(QString type) {
    if(!(type == "fifty-day" || type == "two-hundred-day")) {
        return QMap<QDateTime, float>(); //return empty map
    }
    if(!(moving_avg_series.isEmpty()) && moving_avg_type == type) {
        return moving_avg_series;
    }
    moving_avg_type = type;
    int window_size;
    if(moving_avg_type == "fifty-day") {
        window_size = 50;
    } else {
        window_size = 200;
    }

    QList<float> window(window_size);
    for(QMap<QDateTime, StockDataElement>::const_iterator it = time_series.constBegin(); it != time_series.constEnd(); ++it) {
        window.append(it.value().getClose());
        if(window.size() < window_size) {
            continue;
        }

        //calculate avg over window
        float moving_avg = 0;
        for(int i = 0; i < window_size; ++i) {
            moving_avg += window[i];
        }
        moving_avg = moving_avg / window_size;
        moving_avg_series[it.key()] = moving_avg;

        window.removeFirst();
    }

    return moving_avg_series;
}
