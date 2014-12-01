CPP=g++

FLAGS=-std=c++11 -Wall -Wextra -pedantic -g

ANALYZE_FLAGS = --enable=all --std=c99
ANALYZE_FILTER = 2>&1 | grep --color=always -Ev "\-\-check\-config|never used" 1>&2 | true
SRC_FILES = $(filter-out $(TEST_SRC_FILES), $(wildcard *.cpp))
HEADER_FILES = $(filter-out $(TEST_HEADER_FILES), $(wildcard *.h))

all: ircbot

main.o: main.cpp
	$(CPP) $(FLAGS) -c main.cpp -o main.o
	
ircbot: main.o
	$(CPP) $(FLAGS) main.o -o ircbot
	
analyze:
	@cppcheck $(ANALYZE_FLAGS) $(SRC_FILES) $(HEADER_FILES) $(ANALYZE_FILTER)

clean:
	rm -f main.o ircbot

