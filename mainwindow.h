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

    QWidget *app;
    QVBoxLayout *main_layout; //layout for this window
    QHBoxLayout *control_layout;

    QLabel *app_title;
    QLabel *search_label;
    QTabWidget *tabs;

    // QWidget *control_panel;
    QLineEdit *stock_picker;
    QString curr_ticker;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void fetchData(); //for stock data requests from line edit input

    void renderRequestedStockData();
    void loadRequestedStockData();

    /*
     * saveData();
     *
     */

};
#endif // MAINWINDOW_H
