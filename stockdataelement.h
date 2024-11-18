#ifndef STOCKDATAELEMENT_H
#define STOCKDATAELEMENT_H

#include <QString>


//=============== Date ===============//
class Date
{

public:
    Date(QString &_date);
    Date();
    ~Date();

    QString toString() const;

    bool operator<(const Date &date2) const;

private:

    QString date;
};



//============= StockDataElement ===============//
class StockDataElement
{
    float open;
    float high;
    float low;
    float close;
    float volume;

    Date date;

public:
    StockDataElement(Date _date, float _open, float _high, float _low, float _close, float _volume);
    StockDataElement();
    ~StockDataElement();

    float getOpen() const;
    float getHigh() const;
    float getLow() const;
    float getClose() const;
    float getVolume() const;
};

#endif // STOCKDATAELEMENT_H
