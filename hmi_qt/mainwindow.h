#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include "serialmanager.h"
#include "pwmoscilloscope.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnSeta_clicked();
    void on_btnRearme_clicked();
    void on_btnGraficas_clicked();

    void imprimirConsola(QString mensaje);
    void procesarTemperatura(float temp);
    void procesarEstado(int nuevoEstado);

private:
    Ui::MainWindow *ui;

    // Instancias de nuestros submódulos
    SerialManager *gestorSerial;
    PwmOscilloscope *osciloscopio;

    int botellasCompletadas;
    int estadoAnterior;

    QPixmap imgEstado0;
    QPixmap imgEstado1;
    QPixmap imgEstado2;
    QPixmap imgEstado3;
    QPixmap imgEmergencia;

    void actualizarInterfazVisual(int estado);
};

#endif // MAINWINDOW_H
