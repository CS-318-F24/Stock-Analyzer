#include "stockdataelement.h"


//============== StockDataElement ===============//
StockDataElement::StockDataElement(QDateTime _date, float _open, float _high, float _low, float _close, float _volume) {
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

//saving helper operator
QDataStream &operator<<(QDataStream &out, const StockDataElement &stock_data_element) {
    out << stock_data_element.date << stock_data_element.open << stock_data_element.high << stock_data_element.low << stock_data_element.close << stock_data_element.volume;
    return out;
}

//opening helper operator
QDataStream &operator>>(QDataStream &in, StockDataElement &stock_data_element) {
    in >> stock_data_element.date >> stock_data_element.open >> stock_data_element.high >> stock_data_element.low >> stock_data_element.close >> stock_data_element.volume;
    return in;
}

