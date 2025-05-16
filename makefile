
CC = clang
CFLAGS = -Wall -Wextra -ggdb
RAYLIB_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11


SOURCE=src
BUILD=build


run: clean $(BUILD)/main
	$(BUILD)/main


$(BUILD)/main: build
	$(CC) $(CFLAGS) -o $(BUILD)/main $(SOURCE)/main.cpp $(RAYLIB_FLAGS)

build:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
