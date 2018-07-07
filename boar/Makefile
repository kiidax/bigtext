CC = g++
CCFLAGS = -std=c++11
LIBS = -lstdc++ -lboost_system

boar: boar.o main.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

boar.o: boar.h

%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@