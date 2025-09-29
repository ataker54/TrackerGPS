#ifndef GPS_TRACKER_H
#define GPS_TRACKER_H

#include <QMainWindow>
#include <gpsportautodetector.h>

QT_BEGIN_NAMESPACE
namespace Ui { class gps_tracker; }
QT_END_NAMESPACE

class gps_tracker : public QMainWindow
{
    Q_OBJECT

public:
    gps_tracker(QWidget *parent = nullptr);
    ~gps_tracker();

private slots:
    void btn_click();

private:
    Ui::gps_tracker *ui;

};
#endif // GPS_TRACKER_H
