#include "pwmoscilloscope.h"

PwmOscilloscope::PwmOscilloscope(QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle("Monitor de Señales PWM (Osciloscopio SCADA)");
    this->resize(600, 450);
    tiempoX = 0;
    estadoSimulado = 0;
    dutyValvulaMemoria = 0;

    QVBoxLayout *layout = new QVBoxLayout(this);

    // Gráfica de la Cinta (Verde/Cyan)
    QChart *chartCinta = new QChart();
    chartCinta->setTitle("Señal PWM - Motor de la Cinta (Canal 1)");
    serieCinta = new QLineSeries();
    serieCinta->setColor(Qt::cyan);
    chartCinta->addSeries(serieCinta);
    chartCinta->createDefaultAxes();
    chartCinta->axes(Qt::Vertical).first()->setRange(-10, 110); // Margen extra para que se vea el pulso completo
    chartCinta->axes(Qt::Horizontal).first()->setRange(0, 6);
    chartCinta->legend()->hide();

    QChartView *viewCinta = new QChartView(chartCinta);
    viewCinta->setRenderHint(QPainter::Antialiasing); // Suavizado
    layout->addWidget(viewCinta);

    // Gráfica de la Válvula (Naranja)
    QChart *chartValvula = new QChart();
    chartValvula->setTitle("Señal PWM - Servoválvula de Llenado (Canal 2)");
    serieValvula = new QLineSeries();
    serieValvula->setColor(QColor(255, 69, 0)); // Naranja de advertencia
    chartValvula->addSeries(serieValvula);
    chartValvula->createDefaultAxes();
    chartValvula->axes(Qt::Vertical).first()->setRange(-10, 110);
    chartValvula->axes(Qt::Horizontal).first()->setRange(0, 6);
    chartValvula->legend()->hide();

    QChartView *viewValvula = new QChartView(chartValvula);
    viewValvula->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(viewValvula);

    // Arrancamos el timer a 100ms. Refresca la gráfica 10 veces por segundo.
    timerGraficas = new QTimer(this);
    connect(timerGraficas, &QTimer::timeout, this, &PwmOscilloscope::actualizarGraficasPWM);
}

void PwmOscilloscope::setEstadoActual(int estado)
{
    estadoSimulado = estado;
}

void PwmOscilloscope::iniciarOsciloscopio()
{
    timerGraficas->start(100);
    this->show();
}

void PwmOscilloscope::actualizarGraficasPWM()
{
    // No estamos intentando leer la señal PWM real a 1000 Hz por el USB. Hacer eso
    // colapsaría el ancho de banda y bloquearía QT.
    // Lo que hacemos es simular matemáticamente el Duty Cycle basándonos en los estados
    // lógicos del autómata.

    qreal dutyCinta = 0;

    if(estadoSimulado == 0) {
        dutyCinta = 80;
        dutyValvulaMemoria = 0;
    }
    else if (estadoSimulado == 1) {
        dutyCinta = 0;
        if(dutyValvulaMemoria < 100) dutyValvulaMemoria += 10.0;
    }
    else if (estadoSimulado == 2) {
        dutyCinta = 0;
        dutyValvulaMemoria = 100;
    }
    else if (estadoSimulado == 3) {
        dutyCinta = 0;
        if(dutyValvulaMemoria > 0) dutyValvulaMemoria -= 10.0;
    }
    else if (estadoSimulado == 4 || estadoSimulado == 5) {
        dutyCinta = 0;
        dutyValvulaMemoria = 0;
    }

    // Dibujo geométrico (Cero deformaciones visuales ni aliasing)
    qreal T = 0.1; // Avance del reloj (100ms reales)

    // GENERADOR CINTA
    // En lugar de pintar puntito a puntito, pintamos los vértices del pulso cuadrado
    // multiplicando el tiempo por el porcentaje del Duty Cycle.
    if (dutyCinta == 0 || dutyCinta == 100) {
        serieCinta->append(tiempoX, dutyCinta);
        serieCinta->append(tiempoX + T, dutyCinta);
    } else {
        qreal t_on = T * (dutyCinta / 100.0);
        serieCinta->append(tiempoX, 100);           // Flanco de subida
        serieCinta->append(tiempoX + t_on, 100);    // Meseta
        serieCinta->append(tiempoX + t_on, 0);      // Flanco bajada
        serieCinta->append(tiempoX + T, 0);         // Valle
    }

    // --- GENERADOR VÁLVULA ---
    if (dutyValvulaMemoria == 0 || dutyValvulaMemoria == 100) {
        serieValvula->append(tiempoX, dutyValvulaMemoria);
        serieValvula->append(tiempoX + T, dutyValvulaMemoria);
    } else {
        qreal t_on = T * (dutyValvulaMemoria / 100.0);
        serieValvula->append(tiempoX, 100);
        serieValvula->append(tiempoX + t_on, 100);
        serieValvula->append(tiempoX + t_on, 0);
        serieValvula->append(tiempoX + T, 0);
    }

    tiempoX += T;

    // Efecto osciloscopio: barrido continuo (ventana de 3 segundos).
    if (tiempoX > 3.0) {
        serieCinta->chart()->axes(Qt::Horizontal).first()->setRange(tiempoX - 3.0, tiempoX);
        serieValvula->chart()->axes(Qt::Horizontal).first()->setRange(tiempoX - 3.0, tiempoX);
    }
}
