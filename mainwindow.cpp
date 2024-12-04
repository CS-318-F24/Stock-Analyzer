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
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // Set initial size for the main window
    this->resize(screenWidth * 0.6, screenHeight * 0.6);

    // Set the maximum size of the main window to the available screen size
    this->setMaximumSize(screenWidth, screenHeight);

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
    app_title->setMaximumHeight(30);
    app_title->setTextFormat(Qt::MarkdownText);
    app_title->setAlignment(Qt::AlignLeft);
    main_layout->addWidget(app_title);

    //main dashboard layout
    dashboard_layout = new QHBoxLayout();

    //control layout
    control_layout = new QVBoxLayout();

    search_label = new QLabel("#### Enter Stock Ticker Symbol:");
    search_label->setMaximumHeight(30);
    search_label->setMaximumWidth(500);
    search_label->setTextFormat(Qt::MarkdownText);
    control_layout->addWidget(search_label);

    stock_picker = new QLineEdit();
    stock_picker->setMaximumHeight(30);
    stock_picker->setMaximumWidth(500);
    stock_picker->setPlaceholderText("ex: AAPL");
    connect(stock_picker, &QLineEdit::returnPressed, this, &MainWindow::fetchData);
    control_layout->addWidget(stock_picker);

    fund_stats = new QHBoxLayout();

    available_funds_text = new QLabel("Funds:");
    available_funds_text->setMaximumHeight(30);
    fund_stats->addWidget(available_funds_text);

    available_funds = new QLineEdit();
    available_funds->setMaximumHeight(30);
    available_funds->setPlaceholderText("0.0");
    QIntValidator *intValidator = new QIntValidator();
    available_funds->setValidator(intValidator);
    fund_stats->addWidget(available_funds);

    update_available_funds = new QPushButton("Update");
    update_available_funds->setMaximumHeight(30);
    connect(update_available_funds, &QPushButton::clicked, this, &MainWindow::updateAvailableFunds);
    fund_stats->addWidget(update_available_funds);
    fund_stats->addStretch();

    control_layout->addLayout(fund_stats);

    allocation_stats = new QHBoxLayout();

    percent_allocated_text = new QLabel("Amount allocated:");
    percent_allocated_text->setMaximumHeight(30);
    allocation_stats->addWidget(percent_allocated_text);

    percent_allocated = new QLineEdit();
    percent_allocated->setMaximumHeight(30);
    percent_allocated->setPlaceholderText("0.0");
    allocation_stats->addWidget(percent_allocated);
    allocation_stats->addStretch();

    control_layout->addLayout(allocation_stats);

    portfolio_label = new QLabel("## Portfolio:");
    portfolio_label->setMaximumHeight(30);
    portfolio_label->setMaximumWidth(500);
    portfolio_label->setTextFormat(Qt::MarkdownText);
    control_layout->addWidget(portfolio_label);


    portfolio_table = new QTableWidget();
    portfolio_table->resize(300, 200);
    portfolio_table->setMaximumSize(500, 300);
    portfolio_table->setColumnCount(5);
    portfolio_table->setStyleSheet("QTableWidget { background-color: #444444; }");
    QTableWidgetItem *name = new QTableWidgetItem("Stock Name");
    portfolio_table->setHorizontalHeaderItem(0, name);
    QTableWidgetItem *returns = new QTableWidgetItem("Expected Return");
    portfolio_table->setHorizontalHeaderItem(1, returns);
    QTableWidgetItem *risk = new QTableWidgetItem("Risk");
    portfolio_table->setHorizontalHeaderItem(2, risk);
    QTableWidgetItem *allocation = new QTableWidgetItem("Allocation");
    portfolio_table->setHorizontalHeaderItem(3, allocation);
    QTableWidgetItem *amountInStock = new QTableWidgetItem("Amount($)");
    portfolio_table->setHorizontalHeaderItem(4, amountInStock);
    control_layout->addWidget(portfolio_table);

    //add edit button
    edit_portfolio_button = new QPushButton("edit");
    edit_portfolio_button->setMaximumWidth(500);
    QObject::connect(edit_portfolio_button, &QPushButton::clicked, [&]() {
        QStringList stocks(curr_portfolio.getStockList());
        EditPortfolioDialog dialog;
        connect(&dialog, &EditPortfolioDialog::stockSelectionDeleted, this, &MainWindow::removeStocksFromPortfolio);
        dialog.setStockList(stocks);

        if (dialog.exec() == QDialog::Accepted) {
            QStringList remainingStocks = dialog.getSelectedStocks();
            qDebug() << "Remaining stocks:" << remainingStocks;
        }
    });
    control_layout->addWidget(edit_portfolio_button);


    //add compare button
    compare_button = new QPushButton("compare");
    compare_button->setMaximumWidth(500);
    QObject::connect(compare_button, &QPushButton::clicked,this, &MainWindow::compareStocks);
    control_layout->addWidget(compare_button);

    dashboard_layout->addLayout(control_layout);


    //========= chart layout =========//
    QScrollArea *chart_scroll_area = new QScrollArea;
    QWidget *chart_scroll_content = new QWidget;
    chart_layout = new QVBoxLayout(chart_scroll_area);

    //tabs for displaying stock close price time series
    chart_viewer = new QTabWidget();
    chart_viewer->resize(450, 400);
    chart_viewer->setTabsClosable(true);
    connect(chart_viewer, &QTabWidget::tabCloseRequested, chart_viewer, &QTabWidget::removeTab);
    connect(chart_viewer, &QTabWidget::tabCloseRequested, this, &MainWindow::removeStockWhenChartClosed);
    chart_layout->addWidget(chart_viewer);

    //Initialize and add compare_viewer
    compare_viewer = new QTabWidget();
    compare_viewer->resize(450, 400);
    compare_viewer->setTabsClosable(true);
    connect(compare_viewer, &QTabWidget::tabCloseRequested, compare_viewer, &QTabWidget::removeTab);
    chart_layout->addWidget(compare_viewer);

    // Initialize and add GBM_viewer
    GBM_viewer = new QTabWidget();
    GBM_viewer->resize(450, 400);
    GBM_viewer->setTabsClosable(true);
    connect(GBM_viewer, &QTabWidget::tabCloseRequested, GBM_viewer, &QTabWidget::removeTab);
    chart_layout->addWidget(GBM_viewer);

    connect(portfolio_table, &QTableWidget::itemSelectionChanged, this, &MainWindow::changeDisplayedChart);

    chart_scroll_area->setWidget(chart_scroll_content);
    chart_scroll_area->show();

    dashboard_layout->addWidget(chart_scroll_area);
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

    float userInput = 0.0;
    // Checks that total allocation < 100%

    while (true) {
        bool ok;
        userInput = QInputDialog::getDouble(this, "Allocation Amount",QString("What proportion (0-1) of your funds would you like to allocate to %1").arg(ticker),0,0,1,2,&ok,Qt::WindowFlags(),0.01);

        // user presses cancel
        if (!ok) {
            QMessageBox::warning(this, "Input required", "Please enter a value");
            continue;
        }

        if (userInput + curr_portfolio.getInvestmentUsed() <= 1) {
            break;
        }
        else {
            float investment_remaining = 1 - curr_portfolio.getInvestmentUsed();
            QMessageBox::warning(this, "Invalid input", QString("Cannot allocate more than 100% of funds. You have %1 remaining.").arg(investment_remaining));
        }
    }

    curr_portfolio.allocateInvestment(ticker, userInput);

    percent_allocated->setText(QString::number(curr_portfolio.getInvestmentUsed()));

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

    QTableWidgetItem *allocation_item = new QTableWidgetItem();
    allocation_item->setData(Qt::DisplayRole, userInput);
    portfolio_table->setItem(r, 3, allocation_item);

    QTableWidgetItem *fund_item = new QTableWidgetItem();
    fund_item->setData(Qt::DisplayRole, curr_portfolio.getAvailableFunds() * userInput);
    portfolio_table->setItem(r, 4, fund_item);

    this->renderRequestedStockData(ticker);
    this->simulateGBM(ticker);
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

    for (int i = 0; i < GBM_viewer->count(); ++i) {
        if (GBM_viewer->tabText(i).startsWith(selected_stock)) {
            GBM_viewer->setCurrentIndex(i);
            break;
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
                GBM_viewer->removeTab(r);
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

    for (int i = 0; i < GBM_viewer->count(); ++i) {
        if (GBM_viewer->tabText(i).startsWith(ticker)) {
            GBM_viewer->removeTab(i);
            break;
        }
    }
}


void MainWindow::updateAvailableFunds(int new_amount) {
    curr_portfolio.setAvailableFunds(available_funds->text().toInt());

    int funds_available = curr_portfolio.getAvailableFunds();

    available_funds->setText(QString("%1").arg(funds_available));

    for (int row = 0; row < portfolio_table->rowCount(); ++row) {
        QTableWidgetItem *item = new QTableWidgetItem();
        double allocated_percent = portfolio_table->item(row,3)->text().toDouble();
        item->setData(Qt::DisplayRole, allocated_percent * funds_available);
        portfolio_table->setItem(row,4,item);
    }
}


//slot for adding stock comparison graph --> connected to 'compare' button
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

    //create chart view
    QChartView *combinedChartView = new QChartView(combinedChart);
    combinedChartView->setRenderHint(QPainter::Antialiasing);
    //combinedChartView->setFixedSize(400, 600); //change this later

    QWidget *comparisonTab = new QWidget();
    QHBoxLayout *tabLayout = new QHBoxLayout(comparisonTab);
    tabLayout->addWidget(combinedChartView);

    compare_viewer->addTab(comparisonTab, QString("Comparison: %1 vs %2").arg(firstTabName, secondTabName));
    compare_viewer->setCurrentWidget(comparisonTab);

    /*
 * Also include covariant and correlation. Through portfolio class.
 * Comparison of Comparison Does not work. Don't allow
 * Can we have graph view take up 2/3 of screen instead of half
 *                               "%1:\n"
                              "High: %2\n"
                              "Low: %3\n"
                              "Date Range: %4 to %5\n\n"
                              "%6:\n"
                              "High: %7\n"
                              "Low: %8\n"
                              "Date Range: %9 to %10")
 */
    compareStats = new QLabel();
    compareStats->setText(QString(
                              "%1:\n"
                              "High: %2\n"
                              "Low: %3\n\n"
                              "%4:\n"
                              "High: %5\n"
                              "Low: %6\n"
                              "\nTotal\n"
                              "Covariance: %7\n"
                              "Correlation: %8\n"
                              )
                              .arg(firstTabName)
                              .arg(maxYFirst)
                              .arg(minYFirst)
                              //.arg(minDateFirst.toString("yyyy-MM-dd"))
                              //.arg(maxDateFirst.toString("yyyy-MM-dd"))
                              .arg(secondTabName)
                              .arg(maxYSecond)
                              .arg(minYSecond)
                              .arg(StockPortfolio::covariant(curr_portfolio.getStock(firstTabName), curr_portfolio.getStock(secondTabName)))
                              .arg(StockPortfolio::correlation(curr_portfolio.getStock(firstTabName), curr_portfolio.getStock(secondTabName))));
                              //.arg(minDateSecond.toString("yyyy-MM-dd"))
                              //.arg(maxDateSecond.toString("yyyy-MM-dd")));

    tabLayout->addWidget(compareStats);
}


//render GBM projections
void MainWindow::simulateGBM(QString ticker) {
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

    GBM_viewer->addTab(tabWidget, ticker + "_GBM");
    GBM_viewer->setCurrentWidget(tabWidget);
}



//______________________________________________________
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
