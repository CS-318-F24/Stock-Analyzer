#ifndef EDITPORTFOLIODIALOG_H
#define EDITPORTFOLIODIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QObject>

class EditPortfolioDialog : public QDialog
{
    Q_OBJECT

    QListWidget *stockListWidget;
    QPushButton *deleteButton;


public:
    EditPortfolioDialog(QWidget *parent = nullptr);
    void setStockList(const QStringList &stocks);
    QStringList getSelectedStocks() const;

signals:
    void stockSelectionDeleted(QStringList stocks);

private slots:
    void deleteSelectedStocks();

};

#endif // EDITPORTFOLIODIALOG_H
