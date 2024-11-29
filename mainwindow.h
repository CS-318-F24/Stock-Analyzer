#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <alphavantageapi.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    AlphaVantageAPI *AV_api;

    /*
    QMenu *fileMenu;
    QAction openFile;
    QAction saveFile;
    */

    //layout for app window
    QWidget *app;
    QLabel *app_title;
    QVBoxLayout *main_layout;

    QHBoxLayout *dashboard_layout;

    QVBoxLayout *control_layout;
    QLabel *search_label;
    QLineEdit *stock_picker;
    QLabel *portfolio_label;
    QTableWidget *portfolio_viewer;

    QTabWidget *tabs;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void fetchData(); //for stock data requests from line edit input
    void saveFileNamePrompt();

    void renderRequestedStockData(QString ticker);
    void addRequestedStockData(QString ticker);
    void loadRequestedStockData();

    /*
     * saveData();
     *
     */

};
#endif // MAINWINDOW_H
