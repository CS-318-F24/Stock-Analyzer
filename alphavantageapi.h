#ifndef ALPHAVANTAGEAPI_H
#define ALPHAVANTAGEAPI_H


#include <QObject>
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "stockdata.h"
#include "stockdataelement.h"
#include "filedownloader.h"

class AlphaVantageAPI : public QObject
{
    Q_OBJECT

    QString time_series_type;

    FileDownloader *json_ctrl;

    bool json_saving;
    QString save_loc;

    QMap<QString, StockData*> stock_data_store;

public:
    AlphaVantageAPI(QString _time_series_type="TIME_SERIES_DAILY", bool json_saving=true);
    ~AlphaVantageAPI();

    void requestStockData(QString ticker);

    void setSaving(bool _json_saving);
    void setSaveLocation(QString _save_location);

    StockData *getStockData(QString ticker);

    QString type();

    int numStoredStocks();
    QVector<QString> stockList();

    QVector<QPointF> simulateGBM(QString ticker, float T, int steps);
    float calculateVolatility(QMap<QDateTime, StockDataElement> timeSeries);
    float calculateAverageReturn(QMap<QDateTime, StockDataElement> timeSeries);

signals:
    void savedRequestedStockData(QString ticker);

public slots:
    void addTimeSeries();
    void saveJSON(QString ticker);

};

#endif // ALPHAVANTAGEAPI_H
