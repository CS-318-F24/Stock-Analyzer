#include <QtWidgets>
#include <QByteArray>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

#include "mainwindow.h"
#include "alphavantageapi.h"
#include "filedownloader.h"
#include "stockdata.h"
#include "stockdataelement.h"

QT_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    AV_api = new AlphaVantageAPI();
    connect(AV_api, &AlphaVantageAPI::savedRequestedStockData, this, &MainWindow::renderRequestedStockData);

    //window layout and control panel layout
    app = new QWidget();
    setCentralWidget(app);
    main_layout = new QHBoxLayout(app);

    control_panel = new QWidget();
    left_layout = new QVBoxLayout(control_panel);

    stock_picker = new QLineEdit("Enter stock ticker symbol (e.g. 'AAPL')");
    connect(stock_picker, &QLineEdit::returnPressed, this, &MainWindow::fetchData);
    stock_picker->setFixedSize(240, 20);
    left_layout->addWidget(stock_picker);

    main_layout->addWidget(control_panel, 1, Qt::AlignLeft);
}

MainWindow::~MainWindow() {}

void MainWindow::fetchData() {
    curr_ticker = stock_picker->text();
    AV_api->requestStockData(curr_ticker);
}


void MainWindow::renderRequestedStockData() {
    QLineSeries *lineSeries = new QLineSeries();
    QMap<Date, StockDataElement> *source_data = AV_api->stock_data_store[curr_ticker]->getTimeSeries();
    for(QMap<Date, StockDataElement>::const_iterator it = source_data->constBegin(); it != source_data->constEnd(); ++it) {
        if(it == source_data->constBegin()) {
            continue;
        }
        auto prev = std::prev(it);
        Date prev_date(prev.key());
        lineSeries->append(prev.value().getClose(), it.value().getClose());
    }

    QChart *close_time_series = new QChart();
    close_time_series->legend()->hide();
    close_time_series->addSeries(lineSeries);
    QString title = QString("%1 Close Price").arg(curr_ticker);
    close_time_series->setTitle(title);
    close_time_series->createDefaultAxes();

    QChartView *close_time_series_view = new QChartView(close_time_series);
    close_time_series_view->setRenderHint(QPainter::Antialiasing);

    main_layout->addWidget(close_time_series_view, 0, Qt::AlignRight);

}


void MainWindow::loadRequestedStockData() {

    QFile stock_data_file(QString("/Users/ottoq/Documents/Middlebury/Computer_Science/CS318/stock_data/%1.json").arg(curr_ticker));
    if (!stock_data_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error opening file for writing!";
        return;
    }

    QByteArray raw_data = stock_data_file.readAll();
    QJsonDocument json_doc = QJsonDocument::fromJson(raw_data);
    if (json_doc.isNull()) {
        qDebug() << "Error parsing JSON";
        return;
    }

    QJsonObject json_obj;
    if(json_doc.isObject()) {
        json_obj = json_doc.object();
        qDebug() << "JSON Object";
    }

    //what to do here???

    stock_data_file.close();

}

