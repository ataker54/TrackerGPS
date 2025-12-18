QT += serialport

include(gps_devices_ports_explorer/gps_port_autodetector.pri)
include(gps_port_reader/gps_receiver.pri)
include(json_file_creator/json_file_creator.pri)
include(gps_data/gps_data.pri)
include(gps_data_parser/gps_parser.pri)

GPS_CONTROLLER_DIR = $$PWD

INCLUDEPATH += $$GPS_CONTROLLER_DIR
SOURCES += \
$$GPS_CONTROLLER_DIR/gps_controller.cpp\

HEADERS += \
$$GPS_CONTROLLER_DIR/gps_controller.h
