BIN ?= mjpeg
SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
CFLAGS ?= -Wall -g
PREFIX ?= /usr/local/bin

all: $(BIN)

clean:
	rm -rf $(OBJ) $(BIN)

install: $(BIN)
	install -p $(BIN) $(PREFIX)

$(BIN): $(OBJ)
