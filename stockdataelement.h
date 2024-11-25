#ifndef STOCKDATAELEMENT_H
#define STOCKDATAELEMENT_H

#include <QString>
#include <QDateTime>

//============= StockDataElement ===============//
class StockDataElement
{
    float open;
    float high;
    float low;
    float close;
    float volume;

    QDateTime date;

public:
    StockDataElement(QDateTime _date, float _open, float _high, float _low, float _close, float _volume);
    StockDataElement();
    ~StockDataElement();

    float getOpen() const;
    float getHigh() const;
    float getLow() const;
    float getClose() const;
    float getVolume() const;
};

#endif // STOCKDATAELEMENT_H
