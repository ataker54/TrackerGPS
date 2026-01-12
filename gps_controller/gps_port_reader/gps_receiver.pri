QT += serialport

GPS_PORT_READER_DIR = $$PWD

INCLUDEPATH += $$GPS_PORT_READER_DIR

SOURCES += \
$$GPS_PORT_READER_DIR/gps_receiver.cpp\ \
    $$PWD/file_data_writer.cpp

HEADERS += \
$$GPS_PORT_READER_DIR/gps_receiver.h