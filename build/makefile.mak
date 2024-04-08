CC = g++
CFLAGS = -O1 -Wall -std=c++17 -Wno-missing-braces -I../include/
LDFLAGS = -L../lib/
LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRC_DIR = ../src
BUILD_DIR = ../build
BIN_DIR = ../exe

SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

TARGET = $(BIN_DIR)/Game.exe

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean run

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)