TARGET ?= $(notdir $(shell pwd))
BUILD_PATH ?= $(shell realpath build)
SERIAL_PORT ?= /dev/ttyUSB0

TARGET_BIN ?= $(BUILD_PATH)/$(TARGET).ino.bin
TARGET_INO ?= $(TARGET).ino

ARDUINO_PATH ?= $(shell ls -1dt /opt/arduino-* | head -1)

ARDUINO_USER_SKETCHBOOK ?= $(shell grep --color=none 'sketchbook.path' $(HOME)/.arduino15/preferences.txt | sed -e 's/^sketchbook\.path=//')
ARDUINO_USER_PACKAGES ?= $(HOME)/.arduino15/packages
ARDUINO_USER_LIBRARIES ?= $(ARDUINO_USER_SKETCHBOOK)/libraries

BUILDER_OPT_FQBN ?= '-fqbn=esp8266:esp8266:d1_mini:CpuFrequency=80,UploadSpeed=921600,FlashSize=4M3M'
BUILDER_OPT_IDE_VERSION ?= '-ide-version=10608'
BUILDER_OPT_LOGGER ?= '-logger=human'

BUILDER_OPTS ?= $(BUILDER_OPT_LOGGER) \
		-hardware "$(ARDUINO_PATH)/hardware" \
		-hardware "$(ARDUINO_USER_PACKAGES)" \
		-tools "$(ARDUINO_PATH)/tools-builder" \
		-tools "$(ARDUINO_PATH)/hardware/tools/avr" \
		-tools "$(ARDUINO_USER_PACKAGES)" \
		-built-in-libraries "$(ARDUINO_PATH)/libraries" \
		-libraries "$(ARDUINO_USER_LIBRARIES)" \
		-libraries "../libraries" \
		$(BUILDER_OPT_FQBN) \
		$(BUILDER_OPT_IDE_VERSION) \
		-build-path "$(BUILD_PATH)" \
		-warnings=none \
		-prefs=build.warn_data_percentage=75 \
		-verbose

.PHONY: dump-prefs upload

all: clean $(TARGET_BIN)

$(TARGET_BIN):
	mkdir -p $(BUILD_PATH)
	$(ARDUINO_PATH)/arduino-builder \
		-compile \
		$(BUILDER_OPTS) \
		"$(TARGET_INO)"


dump-prefs:
	mkdir -p $(BUILD_PATH)
	$(ARDUINO_PATH)/arduino-builder \
		-dump-prefs \
		$(BUILDER_OPTS) \
		"$(TARGET_INO)"

upload: $(TARGET_BIN)
	$(ARDUINO_USER_PACKAGES)/esp8266/tools/esptool/0.4.13/esptool -vv -cd nodemcu -cb 921600 -cp $(SERIAL_PORT) -ca 0x00000 -cf $(TARGET_BIN)

clean:
	rm -rf $(BUILD_PATH)
