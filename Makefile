OBJS	= main.o
SOURCE	= main.cpp
HEADER	= main.h
CC		= g++
FLAGS	= -g -c -Wall -Wextra -pedantic
OUT 	= hinfosvc

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++17

clean:
	rm -f $(OBJS) $(OUT)

zip:
	zip xkrama06.zip $(SOURCE) $(HEADER) Makefile Readme.md