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
    qDebug() << "readLoop запущен с портом" << portName << "и baud" << baudRate;
    QSerialPort gps;
    gps.setPortName(portName);
    gps.setBaudRate(baudRate);
    gps.setDataBits(QSerialPort::Data8);
    gps.setParity(QSerialPort::NoParity);
    gps.setStopBits(QSerialPort::OneStop);
    gps.setFlowControl(QSerialPort::NoFlowControl);

    if (!gps.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть порт:" << gps.errorString();
        return;
    }

    qDebug() << "GPS подключен к" << portName;

    while (running) {
        if (gps.waitForReadyRead(5000)) {
            QByteArray chunk = gps.readAll();
            if (!chunk.isEmpty()) {
                qDebug() << "Данные отправлены:" << chunk;
                emit GetDataReceived(chunk);
            }
        } else {
            qDebug() << "Нет данных GPS";
            gps.close();



            if(!gps.isOpen()){
                qDebug() << "Нет подкючения к GPS";
                bool is_port_available = false;

                while(!is_port_available){
                auto detectedPorts = QSerialPortInfo::availablePorts();
                for (const QSerialPortInfo& portInfo : detectedPorts) {
                    if(portInfo.portName()=="COM2"){is_port_available = true;
                        break;
                    }
                }
                Sleep(5000);
                if(!is_port_available)
                gps.setPortName("COM2");
                gps.open(QIODevice::ReadOnly);
                }
            }

        }



    }

    gps.close();
    qDebug() << "readLoop завершён";
}
