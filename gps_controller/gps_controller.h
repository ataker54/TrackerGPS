#ifndef GPS_CONTROLLER_H
#define GPS_CONTROLLER_H

#include <QThread>
#include <QObject>
#include "gps_data.h"

class gps_controller : public QObject {
    Q_OBJECT
public:
    explicit gps_controller(QObject *parent = nullptr);
    ~gps_controller();

    void start(const QString &portName, int baudRate);
    void stop();

signals:
    void gpsUpdated(const GpsData &data);

    void startReceiver(const QString &portName, int baudRate);
    void stopReceiver();

private slots:
    void handleParsedData(const GpsData &data);

private:
    class GPSReceiver *receiver;
    class GPSParser   *parser;
    class GpsPortAutoDetector *detector;

    QThread receiverThread;
};






#endif // GPS_TRACKER_H
