APP_DIR = Application
BUILD_DIR = $(APP_DIR)/build

APP_C_SOURCES =  \
Application/eeprom.c \
Application/hal_extras.c \
Application/SerialDriver.c

APP_CXX_SOURCES = \
Application/application.cpp \
Application/CountdownTimer.cpp \
Application/ModbusRTU.cpp

$(BUILD_DIR)/git_revision.h: .git/index Makefile
	@mkdir -p $(BUILD_DIR)
	git log --pretty=format:'static const char git_hash[58] = "%H";' -n 1 > $@

all: $(BUILD_DIR)/git_revision.h
	$(MAKE) -C WB-MR2 APP_C_SOURCES="$(realpath $(APP_C_SOURCES))" APP_CXX_SOURCES="$(realpath $(APP_CXX_SOURCES))" APP_DIR="$(realpath $(APP_DIR))"
	
clean:
	-rm -fR $(BUILD_DIR)
	$(MAKE) -C WB-MR2 clean