#include "stockgraph.h"
#include <QVBoxLayout>
#include <QDateTime>

//example data point for this  stockData["2023-01-01"] = 100.5;
// in main     StockGraph *graph = new StockGraph();
// graph->plotData(stockData, "SampleTicker");



StockGraph::StockGraph(QWidget *parent)
    : QWidget(parent), chart(new QChart()), series(new QLineSeries()), chartView(new QChartView(chart))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartView);
}

StockGraph::~StockGraph() {}

void StockGraph::plotData(const QMap<QString, float> &data, const QString &ticker)
{
    series->clear();
    series->setName(ticker);

    for (auto it = data.begin(); it != data.end(); ++it) {
        QDateTime date = QDateTime::fromString(it.key(), "yyyy-MM-dd");
        qreal timestamp = date.toMSecsSinceEpoch();
        series->append(timestamp, it.value());
    }

    chart->removeAllSeries();
    chart->addSeries(series);
    chart->setTitle("Stock Prices for " + ticker);
    chart->createDefaultAxes();

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setFormat("dd-MM-yyyy");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Closing Price");
    series->attachAxis(axisY);
}
