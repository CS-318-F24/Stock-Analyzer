#include "stockdataelement.h"


StockDataElement::StockDataElement(float _open, float _high, float _low, float _close, float _volume) {
    open = _open;
    high = _high;
    low = _low;
    close = _close;
    volume = _volume;
}

StockDataElement::StockDataElement() {}

StockDataElement::~StockDataElement() {}
