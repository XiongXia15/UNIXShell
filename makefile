CFILES = $(wildcard *.cpp)
OBJS = $(CFILES:.c=)
CC=g++
CFLAGS = -Wall -g

all: Cshell
Cshell: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $^

clean: $(OBJS) Cshell
	rm $^
