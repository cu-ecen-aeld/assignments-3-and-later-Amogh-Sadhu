CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror -g
TARGET = aesdsocket
LDFLAGS ?= -lpthread

all: $(TARGET)

aesdsocket.o : aesdsocket.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET) : aesdsocket.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

 clean :
	rm -rf $(TARGET) *.o