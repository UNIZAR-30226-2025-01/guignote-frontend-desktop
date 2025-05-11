#ifndef BOTONACCION_H
#define BOTONACCION_H

#include <QPushButton>

class BotonAccion : public QPushButton {
    Q_OBJECT
public:
    explicit BotonAccion(const QString& texto,
                         std::function<void()> callback = nullptr, QWidget* parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    std::function<void()> callback;
};

#endif // BOTONACCION_H
