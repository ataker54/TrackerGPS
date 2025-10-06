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

class json_file_creator
{
public:
    explicit json_file_creator(const QString& filePath = "gps_database.json");

    void createDefaultDatabase();
    QList<GpsDeviceEntry> loadDatabase();

private:
    QString jsonFilePath;
};

#endif // JSONFILECREATOR_H

