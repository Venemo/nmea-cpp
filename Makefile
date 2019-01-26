
CC = gcc
CXX = g++
RM = rm
FLAGS = -Og -g3
CFLAGS = $(FLAGS) -std=c11
CXXFLAGS = $(FLAGS) -std=c++11

all: nmeatest

nmea.o: nmea.cpp nmea.h
	$(CXX) $(CXXFLAGS) -c nmea.cpp

nmeatest.o: nmeatest.cpp nmea.h
	$(CXX) $(CXXFLAGS) -c nmeatest.cpp

nmeatest: nmea.o nmeatest.o
	$(CC) $(CFLAGS) -o nmeatest nmea.o nmeatest.o

clean:
	$(RM) -f *.o nmeatest

