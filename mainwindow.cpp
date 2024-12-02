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
#include "editportfoliodialog.h"

QT_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), curr_portfolio()
{

    setWindowTitle("Portfolio Maker");

    //api for handling stock data
    AV_api = new AlphaVantageAPI("TIME_SERIES_DAILY");
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

    portfolio_label = new QLabel("## Portfolio:");
    portfolio_label->setTextFormat(Qt::MarkdownText);
    control_layout->addWidget(portfolio_label);


    portfolio_table = new QTableWidget();
    portfolio_table->setMinimumSize(300, 200);
    portfolio_table->setMaximumSize(400, 300);
    portfolio_table->setColumnCount(3);
    portfolio_table->setStyleSheet("QTableWidget { background-color: #444444; }");
    QTableWidgetItem *name = new QTableWidgetItem("Stock Name");
    portfolio_table->setHorizontalHeaderItem(0, name);
    QTableWidgetItem *returns = new QTableWidgetItem("Expected Return");
    portfolio_table->setHorizontalHeaderItem(1, returns);
    QTableWidgetItem *risk = new QTableWidgetItem("Risk");
    portfolio_table->setHorizontalHeaderItem(2, risk);
    control_layout->addWidget(portfolio_table);

    //add edit button
    edit_portfolio_button = new QPushButton("edit");
    QObject::connect(edit_portfolio_button, &QPushButton::clicked, [&]() {
        QStringList stocks(curr_portfolio.getStocks());
        EditPortfolioDialog dialog;
        connect(&dialog, &EditPortfolioDialog::stockSelectionDeleted, this, &MainWindow::removeStocksFromPortfolio);
        dialog.setStockList(stocks);

        if (dialog.exec() == QDialog::Accepted) {
            QStringList remainingStocks = dialog.getSelectedStocks();
            qDebug() << "Remaining stocks:" << remainingStocks;
        }
    });

    //add compare button
    compare_button = new QPushButton("compare");
    QObject::connect(compare_button, &QPushButton::clicked,this, &MainWindow::compareStocks);

    control_layout->addWidget(compare_button);

    control_layout->addWidget(edit_portfolio_button);

    dashboard_layout->addLayout(control_layout);

    //tabs for displaying stock close price time series
    chart_viewer = new QTabWidget();
    chart_viewer->setMinimumSize(600, 400);
    chart_viewer->setTabsClosable(true);
    connect(chart_viewer, &QTabWidget::tabCloseRequested, chart_viewer, &QTabWidget::removeTab);
    connect(chart_viewer, &QTabWidget::tabCloseRequested, this, &MainWindow::removeStockWhenChartClosed);

    connect(portfolio_table, &QTableWidget::itemSelectionChanged, this, &MainWindow::changeDisplayedChart);

    dashboard_layout->addWidget(chart_viewer);
    main_layout->addLayout(dashboard_layout);
}

MainWindow::~MainWindow() {}

//get filename for saving helper
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

//================================ public slots ============================//

//slot for initiating api call
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



//slots for adding new (requested) stock data to portfolio and dashboard
void MainWindow::addRequestedStockData(QString ticker) {
    StockData *stock = AV_api->getStockData(ticker);
    curr_portfolio.insert(stock);

    int r = portfolio_table->rowCount();
    portfolio_table->insertRow(r);
    qDebug() << r;

    //add ticker symbol
    QTableWidgetItem *stock_entry = new QTableWidgetItem();
    stock_entry->setData(Qt::DisplayRole, ticker);
    stock_entry->setFlags(stock_entry->flags() & ~Qt::ItemIsEditable);
    portfolio_table->setItem(r, 0, stock_entry);
    //portfolio_table->closePersistentEditor(stock_entry);
    //portfolio_table->setRowHidden(r, false);

    //add expected return
    QTableWidgetItem *expected_return_item = new QTableWidgetItem();
    float expected_return = stock->getExpectedReturn();
    expected_return = qRound(expected_return * pow(10, 4)) / pow(10, 4); //round val to 1000ths
    expected_return_item->setData(Qt::DisplayRole, expected_return);
    expected_return_item->setFlags(expected_return_item->flags() & ~Qt::ItemIsEditable);
    portfolio_table->setItem(r, 1, expected_return_item);

    //add risk
    QTableWidgetItem *risk_item = new QTableWidgetItem();
    float risk = stock->getRisk();
    risk = qRound(risk * pow(10, 4)) / pow(10, 4);
    risk_item->setData(Qt::DisplayRole, risk);
    risk_item->setFlags(risk_item->flags() & ~Qt::ItemIsEditable);
    portfolio_table->setItem(r, 2, risk_item);

    this->renderRequestedStockData(ticker);
}

//helper extension for addRequested to make charts
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

    chart_viewer->addTab(tabWidget, ticker);
    chart_viewer->setCurrentWidget(tabWidget);

}


//slot for setting displayed chart to match corresponding selected stock in table
void MainWindow::changeDisplayedChart() {
    QList<QTableWidgetSelectionRange> selected_ranges = portfolio_table->selectedRanges();
    if(selected_ranges.isEmpty()) {
        qDebug() << "selection change without new selection";
        return;
    }
    int selected_row = selected_ranges[selected_ranges.length() - 1].bottomRow();
    QString selected_stock = portfolio_table->item(selected_row, 0)->text();
    int num_pages = chart_viewer->count();
    for(int i = 0; i < num_pages; ++i) {
        if(chart_viewer->tabText(i) == selected_stock) {
            chart_viewer->setCurrentIndex(i);
        }
    }
}


//slot for removing stocks from portfolio and dashboard after editing
void MainWindow::removeStocksFromPortfolio(QList<QString> stocks_to_delete) {
    qDebug() << stocks_to_delete;
    QSet<QString> set_to_delete;
    for(QString stock : stocks_to_delete) {
        curr_portfolio.remove(stock);
        set_to_delete.insert(stock);
    }

    //iterate over number of stocks to be deleted because tab count changes after each removal
    for(int i = 0; i < stocks_to_delete.size(); ++i) {
        for(int r = 0; r < chart_viewer->count(); ++r) {
            if(set_to_delete.contains(chart_viewer->tabText(r))) {
                chart_viewer->removeTab(r);
            }
        }
    }

    //iterate over number of stocks to be deleted because row count changes after each removal
    for(int i = 0; i < stocks_to_delete.size(); ++i) {
        for(int r = 0; r < portfolio_table->rowCount(); ++r) {
            if(set_to_delete.contains(portfolio_table->item(r, 0)->text())) {
                portfolio_table->removeRow(r);
            }
        }
    }
}


//slot for removing stock table entry when the corresponding stock chart is removed
void MainWindow::removeStockWhenChartClosed(int index) {
    qDebug() << index;
    QString ticker = portfolio_table->item(index, 0)->text();
    curr_portfolio.remove(ticker);

    portfolio_table->removeRow(index); //see if this works without specifying stock name
}


//===================== *in dev, work in progress * ====================//
/*
void MainWindow::simulateGBM() {
    QString ticker = stock_picker->text();
    float T = 1.0;  // years
    int steps = 252;

    QChart *chart = new QChart();
    chart->setTitle("Simulated GBM Paths");
    chart->legend()->hide();

    // Generate multiple paths (5)
    for (int i = 0; i < 5; ++i) {
        QVector<QPointF> gbmPath = AV_api->simulateGBM(ticker, T, steps);
        if (gbmPath.isEmpty()) {
            qDebug() << "GBM simulation failed for ticker:" << ticker;
            continue;
        }

        // Create a series for each path
        QLineSeries *series = new QLineSeries();
        for (const auto &point : gbmPath) {
            series->append(point);
        }
        chart->addSeries(series);
    }

    // Create axes
    QValueAxis *axisX = new QValueAxis;
    axisX->setTitleText("Time (Years)");
    axisX->setLabelFormat("%.2f");
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setTitleText("Price");
    axisY->setLabelFormat("%.2f");
    chart->addAxis(axisY, Qt::AlignLeft);

    for (QAbstractSeries *series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    // Create a chart view
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Add the chart view to a new tab
    QWidget *tabWidget = new QWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout(tabWidget);
    tabLayout->addWidget(chartView);
    tabs->addTab(tabWidget, "GBM Predictions");
    tabs->setCurrentWidget(tabWidget);
}
*/




//old, may deprecate
void MainWindow::loadRequestedStockData() {

    QFile stock_data_file = QFile("/Users/mthedlund/0318Project/stock_data/%1.json");

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
    int tabCount = chart_viewer->count();

    if (tabCount < 2) {
        QMessageBox::warning(this, "More Tabs Needed", "You need at least two tabs to compare.");
        return;
    }

    QStringList tabNames;
    for (int i = 0; i < tabCount; ++i) {
        tabNames << chart_viewer->tabText(i);
    }
    bool ok;

    QString firstTabName = QInputDialog::getItem(this, "Select First Tab", "Choose the first tab to compare:", tabNames, 0, false, &ok);
    if (firstTabName.isEmpty() || !ok) {
        QMessageBox::warning(this, "Canceled", "Canceled");
        return;
    }

    QString secondTabName = QInputDialog::getItem(this, "Select Second Tab", "Choose the second tab to compare:", tabNames, 1, false, &ok);
    if (secondTabName.isEmpty() || firstTabName == secondTabName) {
        QMessageBox::warning(this, "Invalid Selection", "You must select two different tabs to compare.");
        return;
    }
    if(!ok){
        QMessageBox::warning(this, "Canceled", "Canceled");
        return;
    }

    int firstTabIndex = tabNames.indexOf(firstTabName);
    int secondTabIndex = tabNames.indexOf(secondTabName);

    QWidget *firstTab = chart_viewer->widget(firstTabIndex);
    QWidget *secondTab = chart_viewer->widget(secondTabIndex);

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
    combinedChartView->setFixedSize(400, 600); //change this later

    QWidget *comparisonTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(comparisonTab);

    layout->addWidget(combinedChartView);
/*
 * Also include covariant and correlation. Through portfolio class.
 * Move where graph is displayed to being under single graph.
 * Find spot for comparison data
 * Fix sizing of graphs to fit screen.
 *
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
*/
    chart_viewer->addTab(comparisonTab, QString("Comparison: %1 vs %2").arg(firstTabName, secondTabName));
    chart_viewer->setCurrentWidget(comparisonTab);
}
