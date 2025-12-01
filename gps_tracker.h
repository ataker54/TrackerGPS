#ifndef GPS_TRACKER_H
#define GPS_TRACKER_H

#include <QObject>
#include "gps_data.h"

class GPSReceiver;
class GPSParser;

class gps_tracker : public QObject {
    Q_OBJECT
public:
    explicit gps_tracker(QObject *parent = nullptr);

    void start(const QString &portName, int baudRate);
    void stop();

signals:
    void gpsUpdated(const GpsData &data);

private slots:
    void handleRawData(const QByteArray &chunk);
    void handleParsedData(const GpsData &data);

private:
    GPSReceiver *receiver = nullptr;
    GPSParser   *parser   = nullptr;
};






#endif // GPS_TRACKER_H
