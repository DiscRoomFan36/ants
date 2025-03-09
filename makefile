
CC = clang
CFLAGS = -Wall -Wextra -ggdb
RAYLIB_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

main: main.cpp dynamic_array.h ints.h hashmap.h
	$(CC) $(CFLAGS) -o main main.cpp $(RAYLIB_FLAGS)

clean:
	rm main
