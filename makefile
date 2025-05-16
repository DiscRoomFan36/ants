
CC = clang
CFLAGS = -Wall -Wextra -ggdb
RAYLIB_FLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

run: main
	./main

# force because i don't want to list all the files
main: FORCE
	$(CC) $(CFLAGS) -o main main.cpp $(RAYLIB_FLAGS)

clean:
	rm main

FORCE:
