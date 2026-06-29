CC = gcc
AR = gcc-ar
CFLAGS = #-Wall -Wextra

SRC_DIR = src
BUILD_DIR = build

SRCS = tf.c arena.c matrix.c
SRCS_FILES = $(addprefix $(SRC_DIR)/, $(SRCS))
OBJS = $(SRCS_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TARGET = $(BUILD_DIR)/ccontrol
LIBRARY_STATIC = $(BUILD_DIR)/libcontrols.a

.PHONY: all clean test

all: $(TARGET)

$(TARGET): main.c $(LIBRARY_STATIC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) main.c -o $(TARGET) -L$(BUILD_DIR) -lcontrols

$(LIBRARY_STATIC): $(OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
