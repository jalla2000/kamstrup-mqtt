# esp8266-arduino.mk

Requirements
============

 * [Arduino](https://arduino.cc) >= 1.6.7
 * [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino)

Installation
============

1. Go to your Arduino sketch:

    ```
    cd path-to-your-sketch
    ```

1. Clone this repository

    ```
    git clone https://github.com/neomilium/esp8266-arduino.mk.git
    ```

    Note: if you are already in a Git repository, you could use it as submodule:

    ```
    git submodule add https://github.com/neomilium/esp8266-arduino.mk.git
    ```

1. Copy minimal makefile

    ```
    cp esp8266-arduino.mk/Makefile.sample Makefile
    ```

Usage
=====

 * (Re)Build your sketch

    ```
    make
    ```

 * Upload using serial

    ```
    make upload
    ```
