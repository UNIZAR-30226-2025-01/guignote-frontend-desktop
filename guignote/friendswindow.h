#ifndef FRIENDSWINDOW_H
#define FRIENDSWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>  // <-- Asegúrate de incluir esta librería

class friendswindow : public QDialog {
    Q_OBJECT

public:
    explicit friendswindow(QWidget *parent = nullptr);

private:
    QPushButton *closeButton;
    QLabel *titleLabel;
    QLineEdit *searchBar;  // <-- Agregar esta línea para declarar la barra de búsqueda
    QVBoxLayout *mainLayout;

    void setupUI();  // Declaración de la función que configura la UI
};

#endif // FRIENDSWINDOW_H
