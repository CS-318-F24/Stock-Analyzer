#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <alphavantageapi.h>

#include "stockportfolio.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    AlphaVantageAPI *AV_api;

    StockPortfolio curr_portfolio;

    //layout for app window
    QWidget *app;
    QLabel *app_title;
    QLabel *compareStats;

    QVBoxLayout *main_layout;

    QHBoxLayout *dashboard_layout;

    QVBoxLayout *control_layout;
    QVBoxLayout *chart_layout;
    QLabel *search_label;
    QLineEdit *stock_picker;
    QLabel *portfolio_label;
    QTableWidget *portfolio_table;

    QPushButton *edit_portfolio_button;
    QPushButton *compare_button;

    QTabWidget *chart_viewer;
    QTabWidget *compare_viewer;
    QTabWidget *GBM_viewer;

    QHBoxLayout *fund_stats;
    QLabel *available_funds_text;
    QLineEdit *available_funds;
    QPushButton *update_available_funds;

    QHBoxLayout *allocation_stats;
    QLabel *percent_allocated_text;
    QLineEdit *percent_allocated;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void fetchData(); //for stock data requests from line edit input
    void saveFileNamePrompt();

    void renderRequestedStockData(QString ticker);
    void addRequestedStockData(QString ticker);


    void removeStocksFromPortfolio(QList<QString> stocks_to_delete);

    void removeStockWhenChartClosed(int index);

    void changeDisplayedChart(); //displays the chart specified by selected stocks in portfolio viewer

    void compareStocks();

    void simulateGBM(QString ticker);

    void updateAvailableFunds(int new_amount);


    //old, may deprecate
    void loadRequestedStockData();

};
#endif // MAINWINDOW_H
