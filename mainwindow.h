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
    QVBoxLayout *main_layout;

    QHBoxLayout *dashboard_layout;

    QVBoxLayout *control_layout;
    QLabel *search_label;
    QLineEdit *stock_picker;
    QLabel *portfolio_label;
    QTableWidget *portfolio_table;

    QPushButton *edit_portfolio_button;

    QTabWidget *chart_viewer;

    QHBoxLayout *fund_stats;
    QLineEdit *available_funds_text;
    QLineEdit *available_funds;
    QPushButton *update_available_funds;

    QHBoxLayout *allocation_stats;
    QLineEdit *percent_allocated_text;
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


    //old, may deprecate
    void loadRequestedStockData();

    void updateAvailableFunds(int new_amount);

};
#endif // MAINWINDOW_H
