# kamstrup-mqtt
Receives data from Kamstrup energy meter and sends it over MQTT

WARNING: The serial connection seems unstable. It may be because the serial output from the kamstrup is not properly compatible with the NodeMCU serial port. The code should be tested with an MBUS-converter to make sure.

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

Example mqtt.items for OpenHab2:
```
String kamstrupStatus "[%s]" {mqtt="<[mosquitto:house/electricity/status:state:default]"}

Number kamstrupActivePowerPlus "[%dW]" {mqtt="<[mosquitto:house/electricity/power/activePowerPlus:state:default]"}
Number kamstrupActivePowerMinus "[%dW]" {mqtt="<[mosquitto:house/electricity/power/activePowerMinus:state:default]"}
Number kamstrupReactivePowerPlus "[%dW]" {mqtt="<[mosquitto:house/electricity/power/reactivePowerPlus:state:default]"}
Number kamstrupReactivePowerMinus "[%dW]" {mqtt="<[mosquitto:house/electricity/power/reactivePowerMinus:state:default]"}

Number kamstrupVoltageL1 "Voltage [%dV]" {mqtt="<[mosquitto:house/electricity/voltage/l1:state:default]"}
Number kamstrupVoltageL2 "Voltage [%dV]" {mqtt="<[mosquitto:house/electricity/voltage/l2:state:default]"}
Number kamstrupVoltageL3 "Voltage [%dV]" {mqtt="<[mosquitto:house/electricity/voltage/l3:state:default]"}

Number kamstrupCurrentL1 "Current [%.2fA]" {mqtt="<[mosquitto:house/electricity/current/l1:state:default]"}
Number kamstrupCurrentL2 "Current [%.2fA]" {mqtt="<[mosquitto:house/electricity/current/l2:state:default]"}
Number kamstrupCurrentL3 "Current [%.2fA]" {mqtt="<[mosquitto:house/electricity/current/l3:state:default]"}

Number kamstrupActiveImport "[%.2fkWh]" {mqtt="<[mosquitto:house/electricity/energy/activeImport:state:default]"}
Number kamstrupActiveExport "[%.2fkWh]" {mqtt="<[mosquitto:house/electricity/energy/activeExport:state:default]"}
Number kamstrupReactiveImport "[%.2fkWh]" {mqtt="<[mosquitto:house/electricity/energy/reactiveImport:state:default]"}
Number kamstrupReactiveExport "[%.2fkWh]" {mqtt="<[mosquitto:house/electricity/energy/reactiveExport:state:default]"}
```

![Schematics](https://raw.githubusercontent.com/jalla2000/kamstrup-mqtt/master/schematics.png)

![Graphing example in Grafana](https://raw.githubusercontent.com/jalla2000/kamstrup-mqtt/master/screenshot.png)
