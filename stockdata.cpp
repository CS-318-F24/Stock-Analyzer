#include <QtMath>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QVector>

#include "stockdata.h"

#include "stockdataelement.h"

StockData::StockData(QJsonObject _data) : time_series(), moving_avg_series()
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
    }

    qDebug() << "stock data made for " << ticker << ".";
}

StockData::StockData() {}

StockData::~StockData() {}


//public methods
QString StockData::getTicker() {
    return ticker;
}

QMap<QDateTime, StockDataElement> StockData::getTimeSeries() {
    return time_series;
}


//static helpers for calculating correlation between two stock data
QVector<QPair<float, float>> StockData::longestCommonSubsequence(StockData stock1, StockData stock2) {
    qDebug() << "longestCommonSubsequence(" << stock1.getTicker() << "," << stock2.getTicker() << ")";
    QMap<QDateTime, StockDataElement> stock1_time_series = stock1.getTimeSeries();
    QMap<QDateTime, StockDataElement> stock2_time_series = stock2.getTimeSeries();
    qDebug() << "got time series for " << stock1.getTicker() << " and " << stock2.getTicker();

    // get common time series of close prices from stock 1 and 2
    QVector<QPair<float, float>> common_time_series;
    if(stock1_time_series.size() < stock2_time_series.size()){
        qDebug() << stock1.getTicker() << " is longer than " << stock2.getTicker();
        for(QMap<QDateTime, StockDataElement>::const_iterator it = stock1_time_series.cbegin(); it != stock1_time_series.cend(); ++it) {
            if(stock1_time_series.contains(it.key()) && stock2_time_series.contains(it.key())) {
                qDebug() << "\t both time series have data for " << it.key();
                QPair<float, float> close_prices(it.value().getClose(), stock2_time_series[it.key()].getClose());
                common_time_series.append(close_prices);
            }
        }
    } else {
        qDebug() << stock2.getTicker() << " is longer than " << stock1.getTicker();
        for(QMap<QDateTime, StockDataElement>::const_iterator it = stock2_time_series.cbegin(); it != stock2_time_series.cend(); ++it) {
            if(stock2_time_series.contains(it.key()) && stock1_time_series.contains(it.key())) {
                qDebug() << "\t both time series have data for " << it.key();
                QPair<float, float> close_prices(it.value().getClose(), stock1_time_series[it.key()].getClose());
                common_time_series.append(close_prices);
            }
        }
    }

}


//helper for calculating log-returns series
QVector<QPair<float, float>> StockData::getLogReturns(QVector<QPair<float, float>> close_prices) {
    QVector<QPair<float, float>> log_returns;
    for (int i = 1; i < close_prices.size(); ++i) {
        QPair<float, float> returns(qLn(close_prices[i].first) - qLn(close_prices[i - 1].first), qLn(close_prices[i].second) - qLn(close_prices[i - 1].second));
        log_returns.append(returns);
    }
    return log_returns;
}


float StockData::getCorrealationCoefficient(StockData stock1, StockData stock2) {
    QVector<QPair<float, float>> common_time_series = StockData::longestCommonSubsequence(stock1, stock2);
    qDebug() << "got longest common time series between " << stock1.getTicker() << "and " << stock2.getTicker();

    QVector<QPair<float, float>> log_returns = StockData::getLogReturns(common_time_series);
    qDebug() << "got log returns.";

    float mean1 = 0.0;
    float mean_of_squares1 = 0.0;
    float mean2 = 0.0;
    float mean_of_squares2 = 0.0;
    float prod_mean = 0.0;
    for(int i = 0; i < log_returns.size(); ++i) {
        mean1 += log_returns[i].first;
        mean_of_squares1 += qPow(log_returns[i].first, 2);
        mean2 += log_returns[i].second;
        mean_of_squares2 += qPow(log_returns[i].second, 2);
        prod_mean += (log_returns[i].first * log_returns[i].second);
    }
    mean1 = mean1 / log_returns.size();
    mean_of_squares1 = mean_of_squares1 / log_returns.size();
    mean2 = mean2 / log_returns.size();
    mean_of_squares2 = mean_of_squares2 / log_returns.size();
    prod_mean = prod_mean / log_returns.size();

    float coefficient = (prod_mean) - (mean1 * mean2) / qSqrt((mean_of_squares1 - qPow(mean1, 2)) * (mean_of_squares2 - qPow(mean2, 2)));

    return 0.0;
}



QMap<QDateTime, float> StockData::getMovingAvgSeries(QString type) {
    if(!(type == "fifty-day" || type == "two-hundred-day")) {
        qDebug() << "error: moving average range not supported; supports 'fifty-day' and 'two-hundred-day' moving average.";
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

    qDebug() << "getting moving average...";

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
        //qDebug() << "moving average at " << it.key() << " is " << moving_avg;

        window.removeFirst();
    }

    return moving_avg_series;
}
