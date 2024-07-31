# Makefile
CC = gcc
CFLAGS = -Wall -g
TARGET = hw3
SRCS = hw3_client.c
HEADERS = hw3_client.h
OBJS = $(SRCS:.c=.o)

# make  
all: $(TARGET)

# compile executable file 
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# compile object files 
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# make clean
clean:
	rm -f $(OBJS) $(TARGET)

# make rebuild
rebuild: clean all
