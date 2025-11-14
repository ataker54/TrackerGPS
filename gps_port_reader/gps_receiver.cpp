#include "gps_receiver.h"
#include <QDebug>
#include <QThread>
#include <QMetaObject>
#include <QMutexLocker>

GPSReceiver::GPSReceiver(QObject *parent) : QObject(parent)
{
    guiFrame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout(guiFrame);
    labelStatus = new QLabel("Ожидание GPS...");
    labelCoords = new QLabel("—");
    btnStart = new QPushButton("Старт");
    btnStop = new QPushButton("Стоп");
    layout->addWidget(labelStatus);
    layout->addWidget(labelCoords);
    layout->addWidget(btnStart);
    layout->addWidget(btnStop);

    connect(btnStart, &QPushButton::clicked, this, [this]() {
        start("COM2", 9600); // можно заменить на выбор порта
    });

    connect(btnStop, &QPushButton::clicked, this, [this]() {
        stop();
        updateGui(GpsData{});
    });

    logFile.setFileName("gps_log.txt");
    if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Не удалось открыть gps_log.txt для записи";
    } else {
        qDebug() << "Файл открыт для записи";
    }

    connect(this, &GPSReceiver::reconnectNeeded,
            this, &GPSReceiver::attemptReconnect,
            Qt::QueuedConnection);
    connect(this, &GPSReceiver::gpsUpdated,
            this, &GPSReceiver::updateGui,
            Qt::QueuedConnection);
}

QWidget *GPSReceiver::widget() const
{
    return guiFrame;
}

void GPSReceiver::updateGui(const GpsData &data)
{
    if (!labelStatus || !labelCoords)
        return;

    if (!data.valid) {
        labelStatus->setText("Потеря GPS сигнала");
        labelCoords->setText("—");
        return;
    }

    labelStatus->setText("GPS активен");

    labelCoords->setText(QString(
        "Время UTC: %1\n"
        "Широта: %2\n"
        "Долгота: %3\n"
        "Спутники: %4\n"
        "Высота: %5 м\n"
        "Скорость: %6 км/ч\n"
        "Курс: %7°")
        .arg(data.timeUtc)
        .arg(data.latitude, 0, 'f', 6)
        .arg(data.longitude, 0, 'f', 6)
        .arg(data.satellites)
        .arg(data.altitude, 0, 'f', 2)
        .arg(data.speedKmh, 0, 'f', 2)
        .arg(data.course, 0, 'f', 2));
}

void GPSReceiver::start(const QString &portName, int baudRate) {
    lastPort = portName;
    lastBaud = baudRate;

    mutex.lock();
    running = true;
    mutex.unlock();

    QThread *thread = QThread::create([this, portName, baudRate]() {
        readLoop(portName, baudRate);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}


void GPSReceiver::stop() {
    mutex.lock();
    running = false;
    mutex.unlock();

    logFile.close();
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
        qDebug() << "Не удалось открыть порт:" << gps.errorString();
        emit reconnectNeeded();
        emit finished();
        return;
    }

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
                qDebug() << "Нет данных от GPS, реконнект...";
                gps.close();
                emit reconnectNeeded();
                emit finished();
                return;
            }
        }
    }

    gps.close();
    emit finished();
    qDebug() << "readLoop завершён";
}

void GPSReceiver::attemptReconnect() {
    qDebug() << "Попытка реконнекта...";
    QMetaObject::invokeMethod(this, [this]() {
        mutex.lock();
        running = true;
        mutex.unlock();
        start(lastPort, lastBaud);
    }, Qt::QueuedConnection);
}

void GPSReceiver::parseLine(const QString &line)
{
    qDebug() << "parseLine:" << line;
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
        qDebug() << "gpsUpdated:" << latest.latitude << latest.longitude;
    }
    else if (line.startsWith("$GPRMC")) {
        parseGprmc(line, latest);
        writeToFile(latest);
    }
    else if (line.startsWith("$GPGLL")) {
        QStringList parts = line.split(",");
        if (parts.size() >= 7 && parts[6] == "A") {
            latest.latitude = convertCoord(parts[1], parts[2]);
            latest.longitude = convertCoord(parts[3], parts[4]);
            latest.timeUtc = parts[5];
            latest.valid = true;
            emit gpsUpdated(latest);
            writeToFile(latest);
        }
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
    qDebug() << "GPGGA:" << parts;

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
    if (!logFile.isOpen())
        return;

    QTextStream out(&logFile);
    out << "GPS Fix @ " << data.timeUtc << "\n"
        << "Latitude   : " << QString::number(data.latitude, 'f', 6) << "\n"
        << "Longitude  : " << QString::number(data.longitude, 'f', 6) << "\n"
        << "Altitude   : " << QString::number(data.altitude, 'f', 2) << " m\n"
        << "Speed      : " << QString::number(data.speedKmh, 'f', 2) << " km/h\n"
        << "Course     : " << QString::number(data.course, 'f', 2) << "°\n"
        << "Satellites : " << data.satellites << "\n\n";
}

GpsData GPSReceiver::currentGpsData() const
{
    QMutexLocker locker(&mutex);
    return latest;
}
















