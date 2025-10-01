#ifndef GPSPORTAUTODETECTOR_H
#define GPSPORTAUTODETECTOR_H

#include <QObject>
#include <QSerialPortInfo>

struct GpsDeviceEntry {
    QString vendor;
    QString vid;
    QString pid;
    QStringList keywords;
    QString comment;
};

class GpsPortAutoDetector : public QObject
{
    Q_OBJECT
public:
    explicit GpsPortAutoDetector(QObject *parent = nullptr);
    void FindPorts();
    bool isCOMPortGPS(const QSerialPortInfo& portInfo) const;
    void createDefaultGpsJson(const QString& filePath);
    void loadGpsDatabase(const QString& filePath);
    const QList<QSerialPortInfo>& getDetectedPorts() const { return detectedPorts; }
    QString getGpsPortName() const {return gpsPortName;}

private:
    QList<QSerialPortInfo> detectedPorts;
    QList<GpsDeviceEntry> gpsDatabase;
    QString gpsPortName;
signals:

};

#endif // GPSPORTAUTODETECTOR_H
