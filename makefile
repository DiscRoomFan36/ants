
CC = clang
CFLAGS = -Wall -Wextra -ggdb
RAYLIB_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11


SOURCE=src
BUILD=build


run: clean $(BUILD)/main
	$(BUILD)/main



$(BUILD)/main: $(BUILD)/main.o $(BUILD)/noise.o $(BUILD)/raylib_extentions.o $(BUILD)/cells.o $(BUILD)/ant.o $(BUILD)/common.o                           | build
	$(CC) $(CFLAGS) -o $(BUILD)/main   $(BUILD)/main.o $(BUILD)/noise.o $(BUILD)/raylib_extentions.o $(BUILD)/cells.o $(BUILD)/ant.o $(BUILD)/common.o   $(RAYLIB_FLAGS)


# ------------------ *.o's ------------------

$(BUILD)/main.o: $(SOURCE)/main.cpp                                                         | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/main.o $(SOURCE)/main.cpp


$(BUILD)/noise.o: $(SOURCE)/noise.cpp                                                   | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/noise.o $(SOURCE)/noise.cpp

$(BUILD)/raylib_extentions.o: $(SOURCE)/raylib_extentions.cpp                                  | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/raylib_extentions.o $(SOURCE)/raylib_extentions.cpp

$(BUILD)/cells.o: $(SOURCE)/cells.cpp                                                | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/cells.o $(SOURCE)/cells.cpp

$(BUILD)/ant.o: $(SOURCE)/ant.cpp                                                       | build
	$(CC) $(CFLAGS) -c -o $(BUILD)/ant.o $(SOURCE)/ant.cpp


# ------------------ single header files ------------------

# $(BUILD)/context.o: $(SOURCE)/context.h                                      | build
# 	$(CC) $(CFLAGS) -x c -DCONTEXT_IMPLEMENTATION -c -o $(BUILD)/context.o $(SOURCE)/context.h

$(BUILD)/common.o: $(SOURCE)/common.h                                      | build
	$(CC) $(CFLAGS) -x c++ -DCOMMON_IMPLEMENTATION -c -o $(BUILD)/common.o $(SOURCE)/common.h



build:
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
