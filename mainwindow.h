#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <alphavantageapi.h>

#include "stockportfolio.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    AlphaVantageAPI AV_api;
    int call_count;
    int call_limit;

    QString last_dir;

    StockPortfolio curr_portfolio;

    //menu actions
    QMenu *file_menu;
    QAction *save;
    QAction *open;

    //layout for app window
    QWidget *app;
    QLabel *app_title;
    QLabel *compareStats;

    QVBoxLayout *main_layout;

    QHBoxLayout *dashboard_layout;

    QVBoxLayout *control_layout;
    QSplitter *chart_layout;
    QLabel *search_label;
    QLineEdit *stock_picker;
    QLabel *portfolio_label;
    QTableWidget *portfolio_table;
    QLabel *portfolio_return_label;
    float portfolio_return;

    //buttons for portfolio editing and comparing
    QPushButton *edit_portfolio_button;
    QPushButton *compare_button;

    //charts
    QTabWidget *chart_viewer;
    QTabWidget *compare_viewer;
    QTabWidget *GBM_viewer;

    //allocation interface
    QHBoxLayout *fund_stats;
    QLabel *available_funds_text;
    QLineEdit *available_funds;
    QPushButton *update_available_funds;

    QHBoxLayout *allocation_stats;
    QLabel *percent_allocated_text;
    QLineEdit *percent_allocated;

    void updateExpectedReturn();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString saveFileNamePrompt();
    QString openFileNamePrompt();

public slots:
    void fetchData(); //for stock data requests from line edit input
    void savePortfolio();
    void loadPortfolio();

    void addRequestedStockData(QString ticker);

    void addStockDataToTable(QString ticker);
    void renderRequestedStockData(QString ticker);
    void simulateGBM(QString ticker);

    void compareStocks();

    void removeStocksFromPortfolio(QList<QString> stocks_to_delete);

    void removeStockWhenChartClosed(int index);

    void changeDisplayedChart(); //displays the chart specified by selected stocks in portfolio viewer

    void updateAvailableFunds();

};
#endif // MAINWINDOW_H
