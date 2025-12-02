#ifndef GPS_TRACKER_H
#define GPS_TRACKER_H

#include <QObject>
#include "gps_data.h"

class gps_tracker : public QObject {
    Q_OBJECT
public:
    explicit gps_tracker(const QString &portName,
                         int baudRate,
                         QObject *parent = nullptr);
private slots:
    void handleParsedData(const GpsData &data);

signals:
    void gpsUpdated(const GpsData &data);

private:
    class GPSReceiver *receiver;
    class GPSParser   *parser;
};






#endif // GPS_TRACKER_H
