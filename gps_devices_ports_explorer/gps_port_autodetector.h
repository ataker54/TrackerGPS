#ifndef GPSPORTAUTODETECTOR_H
#define GPSPORTAUTODETECTOR_H

#include <QObject>
#include <QSerialPortInfo>
#include <json_file_creator.h>

class GpsPortAutoDetector : public QObject
{
    Q_OBJECT
public:
    explicit GpsPortAutoDetector(QObject *parent = nullptr);
    void FindPorts();
    bool isCOMPortGPS(const QSerialPortInfo& portInfo) const; // fix: private?
    void createDefaultGpsJson(const QString& filePath);// fix: private?
    void loadGpsDatabase(const QString& filePath);// fix: private?
    const QList<QSerialPortInfo>& getDetectedPorts() const { return detectedPorts; } // fix: формироровать список QSerialPortInfo, которые определены как GPS
    QString getGpsPortName() const {return gpsPortName;}

private:
    QList<QSerialPortInfo> detectedPorts;
    QList<GpsDeviceEntry> gpsDatabase;
    QString gpsPortName;
signals:

};

#endif // GPSPORTAUTODETECTOR_H
