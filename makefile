
CC = clang
CFLAGS = -Wall -Wextra -ggdb
RAYLIB_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

main: FORCE
	$(CC) $(CFLAGS) -o main main.cpp $(RAYLIB_FLAGS)

clean:
	rm main

FORCE:
