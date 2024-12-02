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

    // Add 'Simulate GBM' button
    QPushButton *simulateGBMButton = new QPushButton("Simulate GBM");
    connect(simulateGBMButton, &QPushButton::clicked, this, &MainWindow::simulateGBM);
    control_layout->addWidget(simulateGBMButton);

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

