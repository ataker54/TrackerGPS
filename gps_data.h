#ifndef GPS_DATA_H
#define GPS_DATA_H

#include <QString>
#include <QStringList>

struct GpsData {
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;
    QString timeUtc;
    QString date;
    bool valid = false;
    int satellites = 0;
    double speedKmh = 0.0;
    double course = 0.0;

    QString toString() const {
            QStringList parts;

            if (valid) {
                if (latitude != 0.0 || longitude != 0.0)
                    parts << QString("Координаты: %1, %2")
                              .arg(latitude, 0, 'f', 6)
                              .arg(longitude, 0, 'f', 6);

                if (!timeUtc.isEmpty())
                    parts << QString("UTC: %1").arg(timeUtc);

                if (!date.isEmpty())
                    parts << QString("Дата: %1").arg(date);

                if (altitude != 0.0)
                    parts << QString("Высота: %1 м").arg(altitude, 0, 'f', 1);

                if (satellites > 0)
                    parts << QString("Спутники: %1").arg(satellites);

                if (speedKmh > 0.0)
                    parts << QString("Скорость: %1 км/ч").arg(speedKmh, 0, 'f', 1);

                if (course > 0.0)
                    parts << QString("Курс: %1°").arg(course, 0, 'f', 1);
            } else {
                parts << "Нет валидных данных";
            }

            return parts.join(" | ");
        }
};

#endif // GPS_DATA_H
