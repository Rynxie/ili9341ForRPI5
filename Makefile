BUILD_DIR = ./build

TARGET = tftScreen

C_SOURCES = ./src/fonts.c \
./src/ili9341.c \
./src/main.c


INCLUDES = -I./inc/ 
INCLUDES += $(LVGL_INCLUDES)

C_FLAGS = -lgpiod -lpthread -lwiringPi -lX11

$(TARGET):
	mkdir -p $(BUILD_DIR)
	gcc $(C_SOURCES) $(INCLUDES) $(C_FLAGS) -o $(BUILD_DIR)/$(TARGET)