#ifndef STOCKGRAPH_H
#define STOCKGRAPH_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QMap>
#include <QString>


    class StockGraph : public QWidget
{
    Q_OBJECT

public:
    explicit StockGraph(QWidget *parent = nullptr);
    ~StockGraph();

    // Method to plot data, expects a map of date (as string) to closing price
    void plotData(const QMap<QString, float> &data, const QString &ticker);

private:
    QChart *chart;
    QLineSeries *series;
    QChartView *chartView;
};

#endif // STOCKGRAPH_H
