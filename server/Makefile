CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror -g
TARGET = aesdsocket
LDFLAGS ?= -lpthread
s
all: $(TARGET) 

install-deps:
	sudo apt-get update && sudo apt-get install -y netcat-openbsd valgrind
	
aesdsocket.o : aesdsocket.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET) : aesdsocket.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

 clean :
	rm -rf $(TARGET) *.o
