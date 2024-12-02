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
#include <QtCharts/QValueAxis>
#include <QGraphicsRectItem>



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

    //here
    compare = new QPushButton("compare");
    connect(compare, &QPushButton::clicked, this, &MainWindow::compareStocks);
    //also move compare and maybe only have it as option when 2 stocks are loaded.
    //choose which two to compare and then put them next to eachother in the window.

    main_layout->addWidget(compare);
    //compare method
    //take moving average of some kinda small time period.

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

    QFile stock_data_file(QString("/Users/mthedlund/0318Project/stock_data/%1.json").arg(curr_ticker));
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

void MainWindow::compareStocks() {
    int tabCount = tabs->count();

    if (tabCount < 2) {
        QMessageBox::warning(this, "More Tabs Needed", "You need at least two tabs to compare.");
        return;
    }

    QStringList tabNames;
    for (int i = 0; i < tabCount; ++i) {
        tabNames << tabs->tabText(i);
    }

    QString firstTabName = QInputDialog::getItem(this, "Select First Tab", "Choose the first tab to compare:", tabNames, 0, false);
    if (firstTabName.isEmpty()) {
        return;
    }

    QString secondTabName = QInputDialog::getItem(this, "Select Second Tab", "Choose the second tab to compare:", tabNames, 1, false);
    if (secondTabName.isEmpty() || firstTabName == secondTabName) {
        QMessageBox::warning(this, "Invalid Selection", "You must select two different tabs to compare.");
        return;
    }

    int firstTabIndex = tabNames.indexOf(firstTabName);
    int secondTabIndex = tabNames.indexOf(secondTabName);

    QWidget *firstTab = tabs->widget(firstTabIndex);
    QWidget *secondTab = tabs->widget(secondTabIndex);

    QChartView *firstChartView = firstTab->findChild<QChartView *>();
    QChartView *secondChartView = secondTab->findChild<QChartView *>();

    if (!firstChartView || !secondChartView) {
        QMessageBox::warning(this, "Error", "Unable to retrieve charts from the selected tabs.");
        return;
    }

    QLineSeries *firstSeries = static_cast<QLineSeries *>(firstChartView->chart()->series().first());
    QLineSeries *secondSeries = static_cast<QLineSeries *>(secondChartView->chart()->series().first());

    QDateTime minDateFirst = QDateTime::currentDateTime();
    QDateTime maxDateFirst = QDateTime::fromSecsSinceEpoch(0);
    qreal minYFirst = std::numeric_limits<qreal>::max();
    qreal maxYFirst = std::numeric_limits<qreal>::lowest();

    QDateTime minDateSecond = QDateTime::currentDateTime();
    QDateTime maxDateSecond = QDateTime::fromSecsSinceEpoch(0);
    qreal minYSecond = std::numeric_limits<qreal>::max();
    qreal maxYSecond = std::numeric_limits<qreal>::lowest();

    for (const QPointF &point : firstSeries->points()) {
        QDateTime date = QDateTime::fromMSecsSinceEpoch(point.x());
        qreal y = point.y();

        if (date < minDateFirst) minDateFirst = date;
        if (date > maxDateFirst) maxDateFirst = date;
        if (y < minYFirst) minYFirst = y;
        if (y > maxYFirst) maxYFirst = y;
    }

    for (const QPointF &point : secondSeries->points()) {
        QDateTime date = QDateTime::fromMSecsSinceEpoch(point.x());
        qreal y = point.y();

        if (date < minDateSecond) minDateSecond = date;
        if (date > maxDateSecond) maxDateSecond = date;
        if (y < minYSecond) minYSecond = y;
        if (y > maxYSecond) maxYSecond = y;
    }

    QChart *combinedChart = new QChart();
    combinedChart->setTitle(QString("Comparison of %1 and %2").arg(firstTabName, secondTabName));

    QLineSeries *firstSeriesCopy = new QLineSeries();
    firstSeriesCopy->setName(firstTabName);
    firstSeriesCopy->replace(firstSeries->points());
    combinedChart->addSeries(firstSeriesCopy);

    QLineSeries *secondSeriesCopy = new QLineSeries();
    secondSeriesCopy->setName(secondTabName);
    secondSeriesCopy->replace(secondSeries->points());
    combinedChart->addSeries(secondSeriesCopy);

    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd");
    axisX->setTitleText("Date");
    axisX->setRange(minDateFirst < minDateSecond ? minDateFirst : minDateSecond,
                    maxDateFirst > maxDateSecond ? maxDateFirst : maxDateSecond);
    combinedChart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Price");
    axisY->setRange(std::min(minYFirst, minYSecond), std::max(maxYFirst, maxYSecond));
    combinedChart->addAxis(axisY, Qt::AlignLeft);

    for (QAbstractSeries *series : combinedChart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    QChartView *combinedChartView = new QChartView(combinedChart);
    combinedChartView->setRenderHint(QPainter::Antialiasing);
    combinedChartView->setFixedSize(800, 600);

    QWidget *comparisonTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(comparisonTab);

    layout->addWidget(combinedChartView);

    compareStats = new QLabel();
    compareStats->setText(QString(
                              "%1:\n"
                              "High: %2\n"
                              "Low: %3\n"
                              "Date Range: %4 to %5\n\n"
                              "%6:\n"
                              "High: %7\n"
                              "Low: %8\n"
                              "Date Range: %9 to %10")
                              .arg(firstTabName)
                              .arg(maxYFirst)
                              .arg(minYFirst)
                              .arg(minDateFirst.toString("yyyy-MM-dd"))
                              .arg(maxDateFirst.toString("yyyy-MM-dd"))
                              .arg(secondTabName)
                              .arg(maxYSecond)
                              .arg(minYSecond)
                              .arg(minDateSecond.toString("yyyy-MM-dd"))
                              .arg(maxDateSecond.toString("yyyy-MM-dd")));

    layout->addWidget(compareStats);

    tabs->addTab(comparisonTab, QString("Comparison: %1 vs %2").arg(firstTabName, secondTabName));
    tabs->setCurrentWidget(comparisonTab);
}
