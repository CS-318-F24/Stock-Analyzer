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

    QString curr_ticker;
    FileDownloader *json_ctrl;

public:
    AlphaVantageAPI(QString _time_series_type="TIME_SERIES_DAILY");
    ~AlphaVantageAPI();

    void requestStockData(QString ticker);

    QMap<QString, StockData*> stock_data_store;

signals:
    void savedRequestedStockData();

public slots:
    void addTimeSeries();
    void saveJSON();

};

#endif // ALPHAVANTAGEAPI_H
