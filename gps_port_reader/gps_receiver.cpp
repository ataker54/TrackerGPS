#include "gps_receiver.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include "gps_port_autodetector.h"

GPSReceiver::GPSReceiver(QObject *parent) : QObject(parent) {}

void GPSReceiver::start(const QString &port_name,int baudRate, int durationMs)
{
    if (port_name.isEmpty()) {
        qDebug() << "GPS-порт не найден.";
        return;
    }

    //настройка порта
    serial.setPortName(port_name);
    serial.setBaudRate(baudRate);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    serial.setReadBufferSize(1024);

    if (!serial.open(QIODevice::ReadOnly)) {
        qDebug() << "Не удалось открыть порт" << port_name << ":" << serial.errorString();
        return;
    }

    qDebug() << "GPSReceiver started on" << port_name;

    logFile.setFileName("gps_data.txt");
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл gps_data.txt";
        return;
    }

    QTextStream out(&logFile);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(6);

    QElapsedTimer timer;
    timer.start();
    QByteArray buffer;

    //запись данных в файл
    while (timer.elapsed() < durationMs) {
        if (serial.waitForReadyRead(1000)) {
            buffer += serial.readAll();
            QList<QByteArray> lines = buffer.split('\n');
            buffer.clear();

            for (QByteArray &lineRaw : lines) {
                QString line = QString::fromLatin1(lineRaw).trimmed();
                if (line.isEmpty())
                    continue;

                GpsData data;
                if (line.contains("GGA"))
                    data = parseGpgga(line);
                else if (line.contains("RMC"))
                    parseGprmc(line, data);

                if (data.valid) {
                    qDebug().nospace() << "\n GPS Fix:"
                                       << "\n UTC Time   : " << data.timeUtc
                                       << "\n Latitude   : " << QString::number(data.latitude, 'f', 6)
                                       << "\n Longitude  : " << QString::number(data.longitude, 'f', 6)
                                       << "\n Altitude   : " << QString::number(data.altitude, 'f', 2) << " m"
                                       << "\n Speed      : " << QString::number(data.speedKmh, 'f', 2) << " km/h\n";

                    writeToFile(data);
                }
            }
        } else {
            qDebug() << "Ожидание данных...";
        }

        QThread::msleep(100);
    }

    logFile.close();
    serial.close();
    qDebug() << "Данные записаны в gps_data.txt";
}

//преобразование NMEA данных в десятичный формат
double GPSReceiver::convertCoord(const QString &coord, const QString &dir)
{
    if (coord.isEmpty() || (dir != "N" && dir != "S" && dir != "E" && dir != "W"))
        return 0.0;

    int degLen = (dir == "N" || dir == "S") ? 2 : 3;
    int deg = coord.left(degLen).toInt();
    double min = coord.mid(degLen).toDouble();
    double decimal = deg + min / 60.0;
    if (dir == "S" || dir == "W")
        decimal *= -1;
    return decimal;
}

//парсинг строк $GPGGA
GpsData GPSReceiver::parseGpgga(const QString &line)
{
    GpsData data;
    QStringList parts = line.split(",");
    if (parts.size() < 10)
        return data;

    data.latitude = convertCoord(parts[2], parts[3]);
    data.longitude = convertCoord(parts[4], parts[5]);
    data.altitude = parts[9].toDouble();
    data.timeUtc = parts[1];
    data.valid = (parts[6] != "0");
    return data;
}

//парсинг строк $GPRMC
void GPSReceiver::parseGprmc(const QString &line, GpsData &data)
{
    QStringList parts = line.split(",");
    if (parts.size() < 9)
        return;

    if (parts[2] == "A") {
        data.latitude = convertCoord(parts[3], parts[4]);
        data.longitude = convertCoord(parts[5], parts[6]);
        data.speedKmh = parts[7].toDouble() * 1.852;
        data.timeUtc = parts[1];
        data.valid = true;
    }
}

//функция для читаемого вывода
void GPSReceiver::writeToFile(const GpsData &data)
{
    static QMap<QString, GpsData> fixMap;

    GpsData &entry = fixMap[data.timeUtc];
    if (data.altitude != 0.0)
        entry.altitude = data.altitude;
    if (data.speedKmh != 0.0)
        entry.speedKmh = data.speedKmh;
    entry.latitude = data.latitude;
    entry.longitude = data.longitude;
    entry.timeUtc = data.timeUtc;
    entry.valid = true;

    if (entry.altitude != 0.0 && entry.speedKmh != 0.0) {
        QTextStream out(&logFile);
        out << "GPS Fix @ " << entry.timeUtc << "\n"
            << "Latitude   : " << QString::number(entry.latitude, 'f', 6) << "\n"
            << "Longitude  : " << QString::number(entry.longitude, 'f', 6) << "\n"
            << "Altitude   : " << QString::number(entry.altitude, 'f', 2) << " m\n"
            << "Speed      : " << QString::number(entry.speedKmh, 'f', 2) << " km/h\n\n";

        fixMap.remove(entry.timeUtc);
    }
}

