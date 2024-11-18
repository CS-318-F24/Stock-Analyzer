#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QtWidgets>

#include "alphavantageapi.h"
#include "filedownloader.h"
#include "mainwindow.h"
#include "stockgraph.h"
#include "parser.h"

#include "parser.h"
// ...

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    AV_api = new AlphaVantageAPI();
    connect(AV_api,
            &AlphaVantageAPI::savedRequestedStockData,
            this,
            &MainWindow::loadRequestedStockData);

    // Window layout and control panel layout
    app = new QWidget();
    setCentralWidget(app);
    control_panel = new QWidget();
    main_layout = new QHBoxLayout(app);

    left_layout = new QVBoxLayout(control_panel);

    stock_picker = new QLineEdit("Enter stock ticker symbol");
    connect(stock_picker, &QLineEdit::returnPressed, this, &MainWindow::fetchData);
    left_layout->addWidget(stock_picker);

    main_layout->addWidget(control_panel, 1, Qt::AlignCenter);

    QString filePath = "Users/mthedlund/0318Project/stock_data/AAPL.json";
    Parser parser; // Create an instance of the Parser class
    QMap<QString, float> stockData = parser.parseStockDataFromJson(filePath); // Use the member function

    StockGraph *graph = new StockGraph();
    graph->plotData(stockData, "SampleTicker");
}


MainWindow::~MainWindow() {}

void MainWindow::loadRequestedStockData()
{
    QFile stock_data_file(
        QString("/Users/mthedlund/0318Project/stock_data/%1.json")
            .arg(curr_ticker));
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
    if (json_doc.isObject()) {
        json_obj = json_doc.object();
        qDebug() << "JSON Object";
    }

    //what to do here???

    QJsonArray json_array;
    if (json_doc.isArray()) {
        json_array = json_doc.array();
        qDebug() << "JSON Array:";
    }

    stock_data_file.close();
}

void MainWindow::fetchData()
{
    curr_ticker = stock_picker->text();
    AV_api->requestStockData(curr_ticker);
}
