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

    QMap<QString, StockData> stock_data_store;

public:
    AlphaVantageAPI(QString _time_series_type="TIME_SERIES_DAILY");
    ~AlphaVantageAPI();

    void requestStockData(QString ticker);

    StockData getStockData(QString ticker);

    QString type();

    int numStoredStocks();
    QVector<QString> stockList();


signals:
    void savedRequestedStockData(QString ticker);

public slots:
    void addTimeSeries();

};

#endif // ALPHAVANTAGEAPI_H
