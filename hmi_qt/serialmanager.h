#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();
    void enviarComando(const QByteArray &comando);
    void autoConectar();

signals:
    // Señales
    void conexionEstablecida(QString puerto);
    void errorConexion(QString error);
    void logMensaje(QString mensaje);
    void temperaturaRecibida(float temp);
    void estadoRecibido(int estado);

private slots:
    void leerDatos();

private:
    QSerialPort *placaSTM;
};

#endif // SERIALMANAGER_H
