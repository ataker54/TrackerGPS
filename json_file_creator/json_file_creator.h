#ifndef JSONFILECREATOR_H
#define JSONFILECREATOR_H

#include <QString>
#include <QStringList>
#include <QList>

struct GpsDeviceEntry {
    QString vendor;
    QString vid;
    QString pid;
    QString comment;
    QStringList keywords;
};

class JsonFileCreator
{
public:
    explicit JsonFileCreator(const QString& filePath = "gps_database.json");

    void createDefaultDatabase();
    QList<GpsDeviceEntry> loadDatabase();

private:
    QString jsonFilePath;
};

#endif // JSONFILECREATOR_H

