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

    setWindowTitle("Portfolio Maker");

    //api for handling stock data
    AV_api = new AlphaVantageAPI("TIME_SERIES_DAILY");
    connect(AV_api, &AlphaVantageAPI::savedRequestedStockData, this, &MainWindow::renderRequestedStockData);
    connect(AV_api, &AlphaVantageAPI::savedRequestedStockData, this, &MainWindow::addRequestedStockData);

    //main window layout
    app = new QWidget(this);
    setCentralWidget(app);
    main_layout = new QVBoxLayout(app);

    //app window title
    app_title = new QLabel("# Portfolio Maker");
    app_title->setTextFormat(Qt::MarkdownText);
    app_title->setAlignment(Qt::AlignLeft);
    main_layout->addWidget(app_title);

    //main dashboard layout
    dashboard_layout = new QHBoxLayout();

    //control panel layout
    control_layout = new QVBoxLayout();

    search_label = new QLabel("#### Enter Stock Ticker Symbol:");
    search_label->setTextFormat(Qt::MarkdownText);
    control_layout->addWidget(search_label);

    stock_picker = new QLineEdit();
    stock_picker->setPlaceholderText("ex: AAPL");
    connect(stock_picker, &QLineEdit::returnPressed, this, &MainWindow::fetchData);
    control_layout->addWidget(stock_picker);
    dashboard_layout->addLayout(control_layout);

    portfolio_label = new QLabel("## Portfolio:");
    portfolio_label->setTextFormat(Qt::MarkdownText);
    control_layout->addWidget(portfolio_label);

    portfolio_viewer = new QTableWidget();
    portfolio_viewer->setMinimumSize(300, 175);
    portfolio_viewer->setColumnCount(3);
    QTableWidgetItem *name = new QTableWidgetItem("Stock Name:");
    portfolio_viewer->setHorizontalHeaderItem(0, name);
    QTableWidgetItem *returns = new QTableWidgetItem("Expected Returns:");
    portfolio_viewer->setHorizontalHeaderItem(1, returns);
    QTableWidgetItem *risk = new QTableWidgetItem("Risk(covariance):");
    portfolio_viewer->setHorizontalHeaderItem(2, risk);
    control_layout->addWidget(portfolio_viewer);

    //tabs for displaying stock close price time series
    tabs = new QTabWidget();
    tabs->setMinimumSize(600, 400);
    tabs->setTabsClosable(true);
    connect(tabs, &QTabWidget::tabCloseRequested, tabs, &QTabWidget::removeTab);
    dashboard_layout->addWidget(tabs);

    main_layout->addLayout(dashboard_layout);
}

MainWindow::~MainWindow() {}

//file save and open helpers
void MainWindow::saveFileNamePrompt() {
    AV_api->setSaving(true);
    QString fileName = QFileDialog::getSaveFileName(nullptr);
    if (!(fileName.isEmpty())) {
        qDebug() << fileName;
        AV_api->setSaveLocation(fileName);
        return;
    } else {
        return;
    }
}

void MainWindow::fetchData() {
    //dialog prompt for saving or not saving
    QDialog dialog;
    QDialogButtonBox *saveDialog = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, &dialog);

    connect(saveDialog, &QDialogButtonBox::accepted, this, &MainWindow::saveFileNamePrompt);
    connect(saveDialog, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(saveDialog, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    int click_val = dialog.exec();

    if(click_val == 0) {
        AV_api->setSaving(false);
    }

    AV_api->requestStockData(stock_picker->text());
}



//slots for adding new (requested) stock data to dashboard
void MainWindow::addRequestedStockData(QString ticker) {
    StockData *stock = AV_api->getStockData(ticker);
    int row_count = portfolio_viewer->rowCount();
    portfolio_viewer->setRowCount(++row_count);
    row_count = portfolio_viewer->rowCount();

    QTableWidgetItem *stock_entry = new QTableWidgetItem(ticker);
    portfolio_viewer->setItem(row_count, 0, stock_entry);


    //add expected return
    //risk
}

void MainWindow::renderRequestedStockData(QString ticker) {
    qDebug() << ticker;
    StockData *stock_to_render = AV_api->getStockData(ticker);

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
    QString title = QString("%1 Close Price").arg(ticker);
    close_time_series->setTitle(title);
    // close_time_series->createDefaultAxes();

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    axisX->setTickCount(5); // Number of major ticks
    close_time_series->addAxis(axisX, Qt::AlignBottom);
    close_price_line_series->attachAxis(axisX);
    fifty_day_ma_line_series->attachAxis(axisX);


    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Price");
    close_time_series->addAxis(axisY, Qt::AlignLeft);
    close_price_line_series->attachAxis(axisY);
    fifty_day_ma_line_series->attachAxis(axisY);

    QChartView *close_time_series_view = new QChartView(close_time_series);
    close_time_series_view->setMinimumSize(400, 300);
    //close_time_series_view->setMaximumSize(600, 400);
    close_time_series_view->setRenderHint(QPainter::Antialiasing);

    QWidget *tabWidget = new QWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->addWidget(close_time_series_view);

    tabs->addTab(tabWidget, ticker);
    tabs->setCurrentWidget(tabWidget);

}


void MainWindow::loadRequestedStockData() {

    QFile stock_data_file = QFile("/Users/ottoq/Documents/Middlebury/Computer_Science/CS318/stock_data/%1.json");
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

