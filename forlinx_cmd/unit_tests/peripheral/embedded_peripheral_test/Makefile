CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lpthread

TARGET = peripheral_test
SRC = main.c mongoose.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

run:
	./$(TARGET)
