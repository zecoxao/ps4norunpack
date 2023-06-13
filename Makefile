CC=gcc
CFLAGS=-Wall
LDFLAGS=
SOURCES=main.c
EXECUTABLE=ps4norextract
all:
	$(CC) $(CFLAGS) $(SOURCES) $(LDFLAGS) -o $(EXECUTABLE)
clean:
	rm -rf $(EXECUTABLE)