#include "gps_tracker.h"
#include <QDebug>

gps_tracker::gps_tracker(QWidget *parent) : QWidget(parent)
{
    btnStart = new QPushButton("Старт GPS", this);
    btnStop = new QPushButton("Стоп GPS", this);
    gpsView = new QTextEdit(this);
    gpsView->setReadOnly(true);

    layout = new QVBoxLayout(this);
    layout->addWidget(btnStart);
    layout->addWidget(btnStop);
    layout->addWidget(gpsView);
    setLayout(layout);

    connect(btnStart, &QPushButton::clicked, this, &gps_tracker::startGps);
    connect(btnStop, &QPushButton::clicked, this, &gps_tracker::stopGps);
}

gps_tracker::~gps_tracker()
{
    stopGps();
}

void gps_tracker::startGps()
{
    if (gpsThread) return;

    gpsThread = new QThread(this);
    receiver = new GPSReceiver();
    receiver->isWriteToDebug = false;
    receiver->isWriteToFile = false;

    receiver->moveToThread(gpsThread);

    connect(gpsThread, &QThread::started, [=]() {
        receiver->start("COM6", 9600); // замените на нужный порт
    });

    connect(receiver, &GPSReceiver::gpsUpdated, this, &gps_tracker::updateGpsView);
    connect(receiver, &GPSReceiver::finished, gpsThread, &QThread::quit);
    connect(gpsThread, &QThread::finished, receiver, &QObject::deleteLater);
    connect(gpsThread, &QThread::finished, gpsThread, &QObject::deleteLater);

    gpsThread->start();
    qDebug() << "✅ GPS поток запущен.";
}

void gps_tracker::stopGps()
{
    if (receiver) {
        receiver->stop();
        receiver = nullptr;
    }

    if (gpsThread) {
        gpsThread->quit();
        gpsThread->wait();
        gpsThread = nullptr;
        qDebug() << "🛑 GPS поток остановлен.";
    }

    gpsView->clear();
}

void gps_tracker::updateGpsView(const GpsData &data)
{
    QString text;
    text += "UTC Time   : " + data.timeUtc + "\n";
    text += "Date       : " + data.date + "\n";
    text += "Latitude   : " + QString::number(data.latitude, 'f', 6) + "\n";
    text += "Longitude  : " + QString::number(data.longitude, 'f', 6) + "\n";
    text += "Altitude   : " + QString::number(data.altitude, 'f', 2) + " m\n";
    text += "Speed      : " + QString::number(data.speedKmh, 'f', 2) + " km/h\n";
    text += "Course     : " + QString::number(data.course, 'f', 2) + "°\n";
    text += "Satellites : " + QString::number(data.satellites) + "\n";
    text += "Valid Fix  : " + QString(data.valid ? "Yes" : "No") + "\n";

    gpsView->setPlainText(text);
}









