#ifndef VENTANASALIRPARTIDA_H
#define VENTANASALIRPARTIDA_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class VentanaInfo : public QWidget {
    Q_OBJECT
public:
    explicit VentanaInfo(const QString& msg, std::function<void()> onSalir, QWidget* parent = nullptr);

private:
    std::function<void()> onSalirCallback;
};

#endif // VENTANASALIRPARTIDA_H
