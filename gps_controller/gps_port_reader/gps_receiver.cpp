#include "gps_receiver.h"
#include <QDebug>
#include <Windows.h>
#include <gps_port_autodetector.h>

GPSReceiver::GPSReceiver(QObject *parent) : QObject(parent)
{

}

bool GPSReceiver::start(const QString &portName, int baudRate) {
    running = true;
    readLoop(portName, baudRate);
    return true;
}


void GPSReceiver::stop() {
    running = false;
}


void GPSReceiver::readLoop(const QString &portName, int baudRate) {
    running = true;

    QSerialPort gps;
    gps.setBaudRate(baudRate);
    gps.setDataBits(QSerialPort::Data8);
    gps.setParity(QSerialPort::NoParity);
    gps.setStopBits(QSerialPort::OneStop);
    gps.setFlowControl(QSerialPort::NoFlowControl);

    QString targetPort = portName;

    if (targetPort.isEmpty()) {
        GpsPortAutoDetector detector;
        detector.FindPorts();
        auto gpsPorts = detector.getGpsPorts();
        if (!gpsPorts.isEmpty()) {
            targetPort = gpsPorts.first().portName();
            qDebug() << "Автоматически выбран порт:" << targetPort;
        } else {
            qWarning() << "GPS‑порт не найден!";
            return;
        }
    }

    gps.setPortName(targetPort);

    if (!gps.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть порт:" << gps.errorString();
    } else {
        qDebug() << "GPS подключен к" << targetPort;
    }

    while (running) {
        if (gps.isOpen() && gps.waitForReadyRead(2000)) {
            QByteArray chunk = gps.readAll();
            if (!chunk.isEmpty()) {
                emit getDataReceived(chunk);
            }
        } else {
            qDebug() << "Нет данных или порт закрыт, пробуем реконнект...";
            gps.close();

            bool reconnected = false;
            while (running && !reconnected) {
                auto detectedPorts = QSerialPortInfo::availablePorts();
                for (const QSerialPortInfo &portInfo : detectedPorts) {
                    if (portInfo.portName() == targetPort) {
                        gps.setPortName(portInfo.portName());
                        if (gps.open(QIODevice::ReadOnly)) {
                            qDebug() << "Реконнект к" << portInfo.portName();
                            reconnected = true;
                            break;
                        }
                    }
                }
                if (!reconnected) {
                    qDebug() << "Ожидание устройства...";
                    Sleep(5000);
                }
            }
        }
    }

    gps.close();
    qDebug() << "Цикл завершён";
}
