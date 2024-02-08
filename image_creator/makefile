CC = gcc
CFLAGS = -Wall -O2
BUILD = build
EXE = image_creator
TARGET = $(BUILD)/image_creator
SRC = $(EXE).c



all : $(TARGET) always


$(TARGET): $(SRC) always
	$(CC) $< -o $@ $(CFLAGS)

always:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)/*

run:
	./$(TARGET)