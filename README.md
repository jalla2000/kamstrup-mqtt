# kamstrup-mqtt
Receives data from Kamstrup energy meter and sends it over MQTT

Includes Makefile for building the sketch from a terminal (arduino-mk)

To build:
make

To upload:
make upload

Remember to modify the following in the sketch before compiling:
* WiFi SSID
* WiFi Password
* MQTT Username
* MQTT Password

libraries/mbusparser contains parser for the serial data from the energy meter.

run_tests.sh compiles and runs test_mbusparser.cpp, used for testing the parser.

![Schematics](https://raw.githubusercontent.com/jalla2000/kamstrup-mqtt/master/schematics.png)
