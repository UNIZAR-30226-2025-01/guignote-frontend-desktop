#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class InventoryWindow : public QDialog
{
    Q_OBJECT
public:
    explicit InventoryWindow(QWidget *parent = nullptr);
    ~InventoryWindow();

private:
    QListWidget *sidebar;
    QStackedWidget *stackedWidget;
    QPushButton *closeButton;

    void setupUI();
};

#endif // INVENTORYWINDOW_H
