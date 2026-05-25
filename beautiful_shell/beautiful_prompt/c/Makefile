CC = cc
CFLAGS = -Wall -Wextra -march=x86-64 -O2

TARGET = beautiful_prompt
SRC = beautiful_prompt.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
