#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    botellasCompletadas = 0;
    estadoAnterior = 0;

    // Tuvimos un problema al principio usando rutas absolutas (C:/Usuarios/...), ya que al pasar el proyecto a otro PC no cargaban.
    // Lo solucionamos usando el sistema de recursos interno de Qt (.qrc), así las fotos viajan incrustadas dentro del .exe.
    imgEstado0.load(":/IMAGENES/Estado0.png");
    imgEstado1.load(":/IMAGENES/Estado1.png");
    imgEstado2.load(":/IMAGENES/Estado2.png");
    imgEstado3.load(":/IMAGENES/Estado3.png");
    imgEmergencia.load(":/IMAGENES/Emergencia.png");

    // Instanciamos los módulos independientes. Así evitamos el anti-patrón de "God Class" (Clase Dios)
    // que nos comentó el profesor y hacemos programación modular de verdad.
    osciloscopio = new PwmOscilloscope(this);
    gestorSerial = new SerialManager(this);

    // Conectamos los cables entre los módulos y la interfaz gráfica.
    connect(gestorSerial, &SerialManager::logMensaje, this, &MainWindow::imprimirConsola);
    connect(gestorSerial, &SerialManager::conexionEstablecida, this, &MainWindow::imprimirConsola);
    connect(gestorSerial, &SerialManager::errorConexion, this, &MainWindow::imprimirConsola);

    connect(gestorSerial, &SerialManager::temperaturaRecibida, this, &MainWindow::procesarTemperatura);
    connect(gestorSerial, &SerialManager::estadoRecibido, this, &MainWindow::procesarEstado);

    gestorSerial->autoConectar();

    imprimirConsola("INICIANDO SISTEMA HMI...");
    actualizarInterfazVisual(0);
}

MainWindow::~MainWindow()
{
    delete ui; // gestorSerial y osciloscopio se borran solos de la RAM al ser hijos (parent = this)
}

void MainWindow::procesarTemperatura(float temp)
{
    ui->barTemp->setValue(static_cast<int>(temp));
    ui->barTemp->setFormat(QString::number(temp, 'f', 1) + " °C");
}

void MainWindow::procesarEstado(int nuevoEstado)
{
    if(nuevoEstado != estadoAnterior) {

        // CORRECCIÓN DE PRIORIDAD ABSOLUTA
        // Tuvimos un bug: si dábamos a la Seta (estado 4), pero había mensajes antiguos haciendo cola en el USB,
        // la interfaz saltaba un milisegundo al 4 y volvía a estados antiguos.
        // Con esto blindamos la emergencia: si estamos en Seta, ignoramos TODO excepto el rearme (0).
        if(estadoAnterior == 4 && nuevoEstado != 0) {
            return;
        }

        // Lógica del contador de botellas (Flanco de bajada)
        // Decidimos contar la botella en la transición del 3 al 0. Así aseguramos que el ciclo
        // se ha completado físicamente. Si alguien pulsa la seta en el estado 2, la botella no se cuenta.
        if(estadoAnterior == 3 && nuevoEstado == 0){
            botellasCompletadas++;
            ui->lblContador->setText("Botellas: " + QString::number(botellasCompletadas));
            imprimirConsola("¡BOTELLA COMPLETADA!");
        }

        actualizarInterfazVisual(nuevoEstado);
        osciloscopio->setEstadoActual(nuevoEstado); // Actualizamos el osciloscopio
        estadoAnterior = nuevoEstado;
    }
}

void MainWindow::on_btnSeta_clicked()
{
    imprimirConsola("!!! SETA DE EMERGENCIA PULSADA !!!");

    // Por diseño de seguridad, forzamos el reseteo del contador aquí mismo. Botella en curso = botella perdida.
    botellasCompletadas = 0;
    ui->lblContador->setText("Botellas: 0");

    gestorSerial->enviarComando("E");
}

void MainWindow::on_btnRearme_clicked()
{
    imprimirConsola("Enviando orden de REARME a la placa...");
    gestorSerial->enviarComando("R");
}

void MainWindow::on_btnGraficas_clicked()
{
    imprimirConsola("Abriendo módulo de gráficas PWM...");
    osciloscopio->iniciarOsciloscopio();
}

void MainWindow::imprimirConsola(QString mensaje)
{
    // Añadimos timestamp para tener un registro histórico (Log) real de los eventos en el SCADA.
    QString hora = QDateTime::currentDateTime().toString("hh:mm:ss");
    ui->txtConsole->append("[" + hora + "] " + mensaje);
}

void MainWindow::actualizarInterfazVisual(int estado)
{
    // Reseteamos todos a gris. Mucho más seguro que ir apagando uno a uno.
    QString cssApagado = "background-color: #dddddd; color: #888888; border: 1px solid #aaa;";
    ui->lblSt0->setStyleSheet(cssApagado);
    ui->lblSt1->setStyleSheet(cssApagado);
    ui->lblSt2->setStyleSheet(cssApagado);
    ui->lblSt3->setStyleSheet(cssApagado);
    ui->lblStE->setStyleSheet(cssApagado);

    switch(estado) {
    case 0:
        ui->lblSt0->setStyleSheet("background-color: #00aaff; color: white; font-weight: bold;");
        ui->lblImagen->setPixmap(imgEstado0);
        imprimirConsola("> Estado 0: Esperando botella.");
        break;
    case 1:
        ui->lblSt1->setStyleSheet("background-color: #55aaff; color: white; font-weight: bold;");
        ui->lblImagen->setPixmap(imgEstado1);
        imprimirConsola("> Estado 1: Abriendo válvula...");
        break;
    case 2:
        ui->lblSt2->setStyleSheet("background-color: #ffff00; color: black; font-weight: bold;");
        ui->lblImagen->setPixmap(imgEstado2);
        imprimirConsola("> Estado 2: Llenado rápido.");
        break;
    case 3:
        ui->lblSt3->setStyleSheet("background-color: #aa00ff; color: white; font-weight: bold;");
        ui->lblImagen->setPixmap(imgEstado3);
        imprimirConsola("> Estado 3: Cerrando válvula...");
        break;
    case 4:
    case 5:
        // Decisión de diseño: Fusionamos 4 y 5 visualmente porque para el operario la planta requiere
        // supervisión urgente, independientemente de la causa.
        ui->lblStE->setStyleSheet("background-color: red; color: white; font-weight: bold; border: 2px solid black;");
        ui->lblImagen->setPixmap(imgEmergencia);

        // Sin embargo, en el histórico de consola SÍ distinguimos el origen para mantenimiento.
        if(estado == 4) imprimirConsola("!!! PARADA: Seta activada por operario.");
        if(estado == 5) imprimirConsola("!!! ALARMA TÉRMICA: Máquina pausada automáticamente por exceso de calor.");
        break;
    }
}
