#include "stockdataelement.h"



//========== Date implementation ===========//
Date::Date(QString &_date) {
    date = _date;
}

Date::Date() {}

Date::~Date() {}

QString Date::toString() const{
    return date;
}

bool Date::operator<(const Date &other) const {
    return this->date < other.date;
}


//============== StockDataElement ===============//
StockDataElement::StockDataElement(Date _date, float _open, float _high, float _low, float _close, float _volume) {
    open = _open;
    high = _high;
    low = _low;
    close = _close;
    volume = _volume;

    date = _date;
}

StockDataElement::StockDataElement() {}

StockDataElement::~StockDataElement() {}


float StockDataElement::getOpen() const{
    return open;
}

float StockDataElement::getHigh() const{
    return high;
}

float StockDataElement::getLow() const{
    return low;
}

float StockDataElement::getClose() const{
    return close;
}

float StockDataElement::getVolume() const{
    return volume;
}
