QT += serialport

GPS_DATA_DIR = $$PWD

INCLUDEPATH += $$GPS_DATA_DIR

HEADERS += \
$$GPS_DATA_DIR/gps_data.h