BUILD_DIR = ./build

TARGET = tftScreen

C_SOURCES = ./src/fonts.c \
./src/ili9341.c \
./src/main.c

LVGL_DIR := ../lvgl/src

# Tüm alt klasörleri bul
LVGL_SUBDIRS := $(shell find $(LVGL_DIR) -type d)

# Include path'lerini oluştur
LVGL_INCLUDES := $(patsubst %,-I%,$(LVGL_SUBDIRS))

# Tüm .c dosyalarını bul
LVGL_SOURCES := $(shell find $(LVGL_DIR) -name "*.c")
UI_SRC := $(wildcard ./ui/*.c)

C_SOURCES += $(UI_SRC)
C_SOURCES += $(LVGL_SOURCES)

INCLUDES = -I./inc/ \
-I./lvgl/ \
-I./ui/

INCLUDES += $(LVGL_INCLUDES)

C_FLAGS = -lgpiod -lpthread

$(TARGET):
	mkdir -p $(BUILD_DIR)
	gcc $(C_SOURCES) $(INCLUDES) $(C_FLAGS) -o $(BUILD_DIR)/$(TARGET)