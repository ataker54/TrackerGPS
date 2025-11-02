#ifndef GPS_TRACKER_H
#define GPS_TRACKER_H

#include <QWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QThread>
#include "gps_receiver.h"

class gps_tracker : public QWidget
{
    Q_OBJECT

public:
    explicit gps_tracker(QWidget *parent = nullptr);
    ~gps_tracker();

private slots:
    void startGps();
    void stopGps();
    void updateGpsView(const GpsData &data);

private:
    QPushButton *btnStart;
    QPushButton *btnStop;
    QTextEdit *gpsView;
    QVBoxLayout *layout;

    QThread *gpsThread = nullptr;
    GPSReceiver *receiver = nullptr;
};





#endif // GPS_TRACKER_H
