#ifndef PWMOSCILLOSCOPE_H
#define PWMOSCILLOSCOPE_H

#include <QDialog>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QTimer>
#include <QVBoxLayout>

class PwmOscilloscope : public QDialog
{
    Q_OBJECT
public:
    explicit PwmOscilloscope(QWidget *parent = nullptr);
    void setEstadoActual(int estado);

public slots:
    void iniciarOsciloscopio();

private slots:
    void actualizarGraficasPWM();

private:
    QLineSeries *serieCinta;
    QLineSeries *serieValvula;
    QTimer *timerGraficas;
    qreal tiempoX;
    int estadoSimulado;
    qreal dutyValvulaMemoria; // Static virtual para que recuerde por dónde iba la rampa.
};

#endif // PWMOSCILLOSCOPE_H
