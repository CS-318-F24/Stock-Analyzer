#include "parser.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

Parser::Parser() {}

QMap<QString, float> Parser::parseStockDataFromJson(const QString &filePath) {
    QMap<QString, float> stockData;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return stockData;
    }

    QByteArray jsonData = file.readAll();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);

    if (document.isNull() || !document.isObject()) {
        qWarning() << "Invalid JSON format";
        return stockData;
    }

    QJsonObject jsonObject = document.object();
    QJsonObject timeSeries = jsonObject.value("Time Series (Daily)").toObject();

    for (auto it = timeSeries.begin(); it != timeSeries.end(); ++it) {
        QString dateKey = it.key();
        QJsonObject dailyData = it.value().toObject();
        float closingPrice = dailyData.value("4. close").toString().toFloat();
        stockData.insert(dateKey, closingPrice);
    }

    return stockData;
}
