#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <alphavantageapi.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

    AlphaVantageAPI *AV_api;

    QWidget *app;
    QHBoxLayout *main_layout; //layout for this window
    QVBoxLayout *left_layout;

    QWidget *control_panel;
    QLineEdit *stock_picker;
    QString curr_ticker;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void fetchData();
    void loadRequestedStockData();

};
#endif // MAINWINDOW_H
