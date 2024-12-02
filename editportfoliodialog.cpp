#include "editportfoliodialog.h"

EditPortfolioDialog::EditPortfolioDialog(QWidget *parent) {
    stockListWidget = new QListWidget(this);
    stockListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    deleteButton = new QPushButton("delete", this);
    connect(deleteButton, &QPushButton::clicked, this, &EditPortfolioDialog::deleteSelectedStocks);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(stockListWidget);
    layout->addWidget(deleteButton);

    setLayout(layout);
    setWindowTitle("Edit Portfolio");
}

//set which stocks are shown in dialog
void EditPortfolioDialog::setStockList(const QStringList &stocks) {
    stockListWidget->addItems(stocks);
}

//getter for selected stocks in list
QStringList EditPortfolioDialog::getSelectedStocks() const {
    QStringList selectedStocks;
    for (QListWidgetItem *item : stockListWidget->selectedItems()) {
        selectedStocks << item->text();
    }
    return selectedStocks;
}

//slot for deleting selected stocks
void EditPortfolioDialog::deleteSelectedStocks() {
    QList<QString> stocks_to_delete;
    for (QListWidgetItem *item : stockListWidget->selectedItems()) {
        stocks_to_delete.append(item->text());
        delete stockListWidget->takeItem(stockListWidget->row(item));
    }

    //signal to main window
    emit stockSelectionDeleted(stocks_to_delete);

}
