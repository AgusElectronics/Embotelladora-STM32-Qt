#include "serialmanager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent) : QObject(parent)
{
    placaSTM = new QSerialPort(this);
    placaSTM->setBaudRate(QSerialPort::Baud115200); // 115200 baudios para ir sobrados de velocidad y evitar cuellos de botella.

    // Conectamos la interrupción de hardware de Qt con nuestra función de lectura.
    connect(placaSTM, &QSerialPort::readyRead, this, &SerialManager::leerDatos);
}

SerialManager::~SerialManager()
{
    // Destructor: Fundamental cerrar el puerto educadamente antes de matar el programa.
    // Si no hacemos esto, Windows bloquea el COM y nos obliga a reiniciar la placa o el PC para volver a conectarnos (ya nos pasó).
    if(placaSTM->isOpen()) placaSTM->close();
}

void SerialManager::autoConectar()
{
    QString puertoPlaca = "";
    bool placaEncontrada = false;

    // AUTO-DETECCIÓN DE PLACA:
    // Esto lo añadimos porque era un dolor de cabeza tener que cambiar el código fuente (COM3, COM8...)
    // cada vez que cambiábamos lo probabamos en un ordenador diferente. Ahora el software busca la placa solo.

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString descripcion = info.description();

        qDebug() << "Revisando:" << info.portName() << "-" << descripcion;

        // 1. Si vemos que es el ST-Link, pasamos de él porque es el que alimenta la stm.
        if (descripcion.contains("STLink") || descripcion.contains("ST-Link")) {
            continue;
        }

        // 2. Buscamos nuestro puerto de datos "Que windows lo llama dispositivo serie usb".
        // Ponemos también "STM" por si acaso lo conectamos en los PCs de clase y lo detecta distinto.
        if (descripcion.contains("serie USB") || descripcion.contains("Serial USB") || descripcion.contains("STM")) {
            puertoPlaca = info.portName();
            placaEncontrada = true;
            break; // Ale, lo hemos encontrado y salimos del bucle.
        }
    }

    if(placaEncontrada) {
        placaSTM->setPortName(puertoPlaca);
        if(placaSTM->open(QIODevice::ReadWrite)) {
            emit conexionEstablecida("Conexión HMI establecida con éxito en " + puertoPlaca);
        } else {
            emit errorConexion("ERROR: " + puertoPlaca + " detectado pero bloqueado (¿Monitor Serie del CubeIDE abierto?).");
        }
    } else {
        emit errorConexion("ERROR CRÍTICO: No se ha detectado ninguna placa conectada.");
    }
}

void SerialManager::enviarComando(const QByteArray &comando)
{
    if(placaSTM->isOpen()) {
        placaSTM->write(comando); // Mandamos comandos de 1 solo carácter ('E' o 'R') para no sobrecargar el USB del micro y para usar Buf(0) y que nos sea mas sencillo en la placa.
    }
}

void SerialManager::leerDatos()
{
    // Usamos un while con canReadLine() porque descubrimos que a veces el PC lee tan rápido
    // que pilla los mensajes a medias. Con readLine() nos aseguramos de leer hasta encontrar el '\r\n' que mandamos desde C.
    while(placaSTM->canReadLine()) {
        QByteArray datos = placaSTM->readLine().trimmed(); // trimmed() limpia espacios y saltos de línea basura
        QString mensaje = QString::fromUtf8(datos);

        if(mensaje.startsWith("T:")) {
            // Recortamos desde el carácter 2 en adelante (nos saltamos el "T:") y lo pasamos a float.
            float temp = mensaje.mid(2).toFloat();
            emit temperaturaRecibida(temp);
            continue; // Ya hemos procesado la temperatura, saltamos a la siguiente línea del buffer.
        }

        // Si no es temperatura, es un estado
        int nuevoEstado = -1;
        if(mensaje.contains("0")) nuevoEstado = 0;
        else if(mensaje.contains("1")) nuevoEstado = 1;
        else if(mensaje.contains("2")) nuevoEstado = 2;
        else if(mensaje.contains("3")) nuevoEstado = 3;
        else if(mensaje.contains("4")) nuevoEstado = 4;
        else if(mensaje.contains("5")) nuevoEstado = 5;

        if(nuevoEstado != -1) {
            emit estadoRecibido(nuevoEstado);
        }
    }
}
