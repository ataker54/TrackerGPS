#include "gps_receiver.h"
#include <QDebug>
#include <QThread>

GPSReceiver::GPSReceiver(QObject *parent) : QObject(parent)
{
    logFile.setFileName("gps_log.txt");
    logFile.open(QIODevice::Append | QIODevice::Text);

    reconnectTimer = new QTimer(this);
    reconnectTimer->setInterval(reconnectIntervalMs);
    connect(reconnectTimer, &QTimer::timeout, this, &GPSReceiver::attemptReconnect);
}

void GPSReceiver::start(const QString &portName, int baudRate)
{
    lastPort = portName;
    lastBaud = baudRate;

    mutex.lock();
    running = true;
    mutex.unlock();

    readLoop(portName, baudRate);
}

void GPSReceiver::stop()
{
    mutex.lock();
    running = false;
    mutex.unlock();

    if (reconnectTimer->isActive())
        reconnectTimer->stop();

    logFile.close();
}

void GPSReceiver::readLoop(const QString &portName, int baudRate)
{
    QSerialPort gps;
    gps.setPortName(portName);
    gps.setBaudRate(baudRate);
    gps.setDataBits(QSerialPort::Data8);
    gps.setParity(QSerialPort::NoParity);
    gps.setStopBits(QSerialPort::OneStop);
    gps.setFlowControl(QSerialPort::NoFlowControl);

    if (!gps.open(QIODevice::ReadOnly)) {
        qDebug() << "❌ Не удалось открыть порт:" << gps.errorString();
        if (!reconnectTimer->isActive())
            reconnectTimer->start();
        emit finished();
        return;
    }

    if (reconnectTimer->isActive())
        reconnectTimer->stop();

    qDebug() << "✅ GPS подключен к" << portName;

    QByteArray buffer;
    int noDataCounter = 0;

    while (true) {
        mutex.lock();
        bool keepRunning = running;
        mutex.unlock();
        if (!keepRunning) break;

        if (gps.waitForReadyRead(1000)) {
            QByteArray chunk = gps.readAll();
            if (!chunk.isEmpty()) {
                buffer += chunk;
                noDataCounter = 0;

                QStringList lines = QString::fromLatin1(buffer).split("\n");
                buffer.clear();

                for (const QString &line : lines) {
                    QString trimmed = line.trimmed();
                    if (!trimmed.isEmpty())
                        parseLine(trimmed);
                }
            }
        } else {
            noDataCounter++;
            if (noDataCounter >= 5) {
                if (!reconnectTimer->isActive()) {
                    qDebug() << "⏱️ Запуск таймера реконнекта";
                    reconnectTimer->start();
                }
                qDebug() << "⚠️ Нет данных от GPS, реконнект...";
                gps.close();
                if (!reconnectTimer->isActive())
                    reconnectTimer->start();
                emit finished();
                return;
            }
        }
    }

    mutex.lock();
    running = false;
    mutex.unlock();
    gps.close();
    emit finished();
    qDebug() << "📴 readLoop завершён";

}

void GPSReceiver::attemptReconnect()
{
    qDebug() << "🔁 Таймер реконнекта сработал";
        QMetaObject::invokeMethod(this, [this]() {
            mutex.lock();
            running = true;
            mutex.unlock();
            start(lastPort, lastBaud);
        }, Qt::QueuedConnection);
}

void GPSReceiver::parseLine(const QString &line)
{
    QMutexLocker locker(&mutex);

    if (line.startsWith("$GPGGA")) {
        GpsData parsed = parseGpgga(line);
        latest.latitude = parsed.latitude;
        latest.longitude = parsed.longitude;
        latest.altitude = parsed.altitude;
        latest.timeUtc = parsed.timeUtc;
        latest.valid = parsed.valid;
        latest.satellites = parsed.satellites;
        writeToFile(latest);
        emit gpsUpdated(latest);
    }
    else if (line.startsWith("$GPRMC")) {
        parseGprmc(line, latest);
        writeToFile(latest);
    }
}

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
    data.satellites = parts[7].toInt();
    return data;
}

void GPSReceiver::parseGprmc(const QString &line, GpsData &data)
{
    QStringList parts = line.split(",");
    if (parts.size() < 9)
        return;

    if (parts[2] == "A") {
        data.latitude = convertCoord(parts[3], parts[4]);
        data.longitude = convertCoord(parts[5], parts[6]);
        data.speedKmh = parts[7].toDouble() * 1.852;
        data.course = parts[8].toDouble();
        data.timeUtc = parts[1];
        data.date = parts[9];
        data.valid = true;
    }
}

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

    if (entry.altitude != 0.0 && entry.speedKmh != 0.0 && isWriteToFile) {
        QTextStream out(&logFile);
        out << "GPS Fix @ " << entry.timeUtc << "\n"
            << "Latitude   : " << QString::number(entry.latitude, 'f', 6) << "\n"
            << "Longitude  : " << QString::number(entry.longitude, 'f', 6) << "\n"
            << "Altitude   : " << QString::number(entry.altitude, 'f', 2) << " m\n"
            << "Speed      : " << QString::number(entry.speedKmh, 'f', 2) << " km/h\n\n";

        fixMap.remove(entry.timeUtc);
    }
}

GpsData GPSReceiver::currentGpsData() const
{
    QMutexLocker locker(&mutex);
    return latest;
}
















