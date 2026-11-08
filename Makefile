CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDLIBS = -lm
OBJ = main.o artemis.o validaciones.o
TARGET = artemis

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDLIBS)

%.o: %.c artemis.h validaciones.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
