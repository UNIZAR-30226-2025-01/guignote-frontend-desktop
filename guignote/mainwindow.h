#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qlabel.h>

// Definimos la clase como parte del namespace de Q, para poder acceder a los elementos gráficos
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Heredamos de QMainWindow, somos la pantalla principal
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor y destructor (que puede recibir un padre)
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // Definimos los atributos (son Labels, elementos gráficos)
    QLabel *cornerTopLeft;
    QLabel *cornerTopRight;
    QLabel *cornerBottomLeft;
    QLabel *cornerBottomRight;
    QSize ornamentSize;
    // Función para manejar el evento de redimensionamiento de ventana.
    // resizeEvent es un método virtual de QMainWindow, el cual podemos overridear
    void resizeEvent(QResizeEvent *event);

    // Método que ajustará la posición de los adornos cuando se redimensione la ventana
    void repositionOrnaments();


private:
    // Puntero a la interfaz de usuario generada por Qt (los widgets de la interfaz)
    Ui::MainWindow *ui;

};



#endif // MAINWINDOW_H
