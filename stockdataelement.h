#ifndef STOCKDATAELEMENT_H
#define STOCKDATAELEMENT_H

#include <QString>

typedef QString Date;

class StockDataElement:
{
    float open;
    float high;
    float low;
    float close;
    float volume;

public:
    StockDataElement(float _open, float _high, float _low, float _close, float _volume);
    StockDataElement();
    ~StockDataElement();

    Date date;
};

#endif // STOCKDATAELEMENT_H
