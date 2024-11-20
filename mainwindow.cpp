#include <QtWidgets>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QValueAxis>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>


#include "mainwindow.h"
#include "alphavantageapi.h"
#include "filedownloader.h"
#include "stockdata.h"
#include "stockdataelement.h"

QT_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    setWindowTitle("Stock Data Viewer");

    AV_api = new AlphaVantageAPI("TIME_SERIES_DAILY");
    connect(AV_api, &AlphaVantageAPI::savedRequestedStockData, this, &MainWindow::renderRequestedStockData);

    //window layout and control panel layout
    app = new QWidget();
    setCentralWidget(app);
    main_layout = new QVBoxLayout(app);

    app_title = new QLabel("Stock Data Viewer");
    app_title->setAlignment(Qt::AlignCenter);
    main_layout->addWidget(app_title);

    control_layout = new QHBoxLayout();
    search_label = new QLabel("Enter Stock Ticker Symbol:");
    control_layout->addWidget(search_label);

    stock_picker = new QLineEdit();
    stock_picker->setPlaceholderText("ex: AAPL");
    connect(stock_picker, &QLineEdit::returnPressed, this, &MainWindow::fetchData);
    control_layout->addWidget(stock_picker);

    main_layout->addLayout(control_layout);

    tabs = new QTabWidget();
    tabs->setTabsClosable(true);
    connect(tabs, &QTabWidget::tabCloseRequested, tabs, &QTabWidget::removeTab);
    main_layout->addWidget(tabs);
}

MainWindow::~MainWindow() {}

void MainWindow::fetchData() {
    curr_ticker = stock_picker->text();
    AV_api->requestStockData(curr_ticker);
}


void MainWindow::renderRequestedStockData() {
    QLineSeries *lineSeries = new QLineSeries();
    QMap<QDateTime, StockDataElement> *source_data = AV_api->stock_data_store[curr_ticker]->getTimeSeries();
    for(QMap<QDateTime, StockDataElement>::const_iterator it = source_data->constBegin(); it != source_data->constEnd(); ++it) {
        if(it == source_data->constBegin()) {
            continue;
        }
        qreal close_price = it.value().getClose();

        //qDebug() << it.key() << it.value().getClose();

        lineSeries->append(it.key().toMSecsSinceEpoch(), close_price);
    }

    QChart *close_time_series = new QChart();
    close_time_series->legend()->hide();
    close_time_series->addSeries(lineSeries);
    QString title = QString("%1 Close Price").arg(curr_ticker);
    close_time_series->setTitle(title);
    // close_time_series->createDefaultAxes();

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    axisX->setTickCount(10); // Number of major ticks
    close_time_series->addAxis(axisX, Qt::AlignBottom);
    lineSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Price");
    close_time_series->addAxis(axisY, Qt::AlignLeft);
    lineSeries->attachAxis(axisY);

    QChartView *close_time_series_view = new QChartView(close_time_series);
    close_time_series_view->setFixedSize(800, 600);
    close_time_series_view->setRenderHint(QPainter::Antialiasing);

    // QDateTimeAxis *axisX = new QDateTimeAxis();
    // axisX->setFormat("yyyy-MM-dd"); // Display format
    // axisX->setTickCount(10); // Number of major ticks
    // close_time_series->setAxisX(axisX, lineSeries);

    // main_layout->addWidget(close_time_series_view, 2, Qt::AlignCenter);
    QWidget *tabWidget = new QWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->addWidget(close_time_series_view);

    tabs->addTab(tabWidget, curr_ticker);
    tabs->setCurrentWidget(tabWidget);

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

