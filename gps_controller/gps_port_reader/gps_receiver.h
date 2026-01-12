#ifndef GPSRECEIVER_H
#define GPSRECEIVER_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QTimer>
#include <QVector>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

class GPSReceiver : public QObject
{
    Q_OBJECT

public:
    explicit GPSReceiver(QObject *parent = nullptr);

    void startInThread(const QString &portName, int baudRate = 9600);
    void stopInThread();

signals:
    void getDataReceived(const QByteArray &data);

private:
    void readLoop(const QString &portName, int baudRate);

    bool running = false;

};


#endif // GPSRECEIVER_H



