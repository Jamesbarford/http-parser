OUT = build
TARGET = parse_headers.out
CC = cc
CFLAGS = -Wall -Wextra -Werror -Wpedantic -g -O0

$(OUT)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

all: $(TARGET)

clean:
	rm $(TARGET)
	rm $(OUT)/*.o

OBJ_LIST = $(OUT)/http_parser.o \
           $(OUT)/main.o

$(TARGET): $(OBJ_LIST)
	$(CC) -o $(TARGET) $(OBJ_LIST)	

$(OUT)/http_parser.o: ./http_parser.c ./http_parser.h
$(OUT)/main.o: ./main.c ./http_parser.h
