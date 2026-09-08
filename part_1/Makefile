CC = cc
CFLAGS = -Wall -Wextra -O2 -std=c11

BIN = benchmark

all: $(BIN)

$(BIN): main.c
	$(CC) $(CFLAGS) -o $(BIN) main.c

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all run clean
