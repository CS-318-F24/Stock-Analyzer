#ifndef ALPHAVANTAGEAPI_H
#define ALPHAVANTAGEAPI_H


#include <QObject>
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "filedownloader.h"

class AlphaVantageAPI : public QObject
{
    Q_OBJECT

    QString time_series;
    QString curr_ticker;
    FileDownloader *json_ctrl;

public:
    AlphaVantageAPI(QString time_series="TIME_SERIES_DAILY");
    ~AlphaVantageAPI();

    void requestStockData(QString ticker);

signals:
    void savedRequestedStockData();

public slots:
    void saveJSON();

};

#endif // ALPHAVANTAGEAPI_H
