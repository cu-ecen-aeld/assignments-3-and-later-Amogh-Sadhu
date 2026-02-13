CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -Wall -Werror -g
TARGET = aesdsocket
LDFLAGS ?= -lpthread -lrt

SRCS = aesdsocket.c linkedListlib.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET) 

install-deps:
	sudo apt-get update && sudo apt-get install -y netcat-openbsd valgrind
	
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

 clean :
	rm -rf $(TARGET) *.o
