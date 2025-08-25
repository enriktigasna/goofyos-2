CC = gcc
CFLAGS = -nostdlib -static -fno-builtin -Wall
TARGET = init
SRC = init.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)