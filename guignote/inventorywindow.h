#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QGraphicsOpacityEffect>

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
    // Para las animaciones de fade al cambiar pestaña
    QPropertyAnimation *fadeOutAnim;
    QPropertyAnimation *fadeInAnim;

    QGraphicsOpacityEffect *deckEffect;
    QGraphicsOpacityEffect *matEffect;

    QButtonGroup *deckGroup;            // ← Grupo para barajas
    QButtonGroup *matGroup;             // ← Grupo para tapetes
    void setupUI();

private slots:
    void onTabChanged(int index);

};



#endif // INVENTORYWINDOW_H
