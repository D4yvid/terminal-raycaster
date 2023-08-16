SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

CC = gcc
CFLAGS = -Wall -Werror -Wpedantic -Wextra \
		 -Wno-unused-variable -Wno-unused-parameter \
		 -Wno-sign-compare

TARGET = terminal-raycaster

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
