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

    //curr_ticker = "AAPL";
    //AV_api->fetchData();
    //curr_ticker = "IBM";
    //AV_api->fetchData();
    //curr_ticker = "PEP";

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

    //tabs for displaying more than one stock
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
    StockData *stock_to_render = AV_api->getStockData(curr_ticker);
    QMap<QDateTime, StockDataElement> source_data = stock_to_render->getTimeSeries();
    QMap<QDateTime, float> fifty_day_ma = stock_to_render->getMovingAvgSeries();
    qDebug() << "attained moving average.";

    QLineSeries *close_price_line_series = new QLineSeries();
    QLineSeries *fifty_day_ma_line_series = new QLineSeries();
    for(QMap<QDateTime, float>::const_iterator it = fifty_day_ma.constBegin(); it != fifty_day_ma.constEnd(); ++it) {
        qreal close_price = source_data[it.key()].getClose();
        close_price_line_series->append(it.key().toMSecsSinceEpoch(), close_price);

        fifty_day_ma_line_series->append(it.key().toMSecsSinceEpoch(), it.value());
    }

    qDebug() << "fifty day line series";

    QChart *close_time_series = new QChart();
    close_time_series->legend()->hide();
    close_time_series->addSeries(close_price_line_series);
    close_time_series->addSeries(fifty_day_ma_line_series);
    QString title = QString("%1 Close Price").arg(curr_ticker);
    close_time_series->setTitle(title);
    // close_time_series->createDefaultAxes();

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    axisX->setTickCount(10); // Number of major ticks
    close_time_series->addAxis(axisX, Qt::AlignBottom);
    close_price_line_series->attachAxis(axisX);
    fifty_day_ma_line_series->attachAxis(axisX);


    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Price");
    close_time_series->addAxis(axisY, Qt::AlignLeft);
    close_price_line_series->attachAxis(axisY);
    fifty_day_ma_line_series->attachAxis(axisY);

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

