#ifndef PARSER_H
#define PARSER_H

#include <QMap>
#include <QString>

class Parser
{
public:
    Parser();
    QMap<QString, float> parseStockDataFromJson(const QString &filePath);
};

#endif // PARSER_H
