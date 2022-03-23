CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = server
OBJS = ./src/server/*.cpp ./src/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./build/$(TARGET) -pthread

clean:
	rm -rf ./build/$(OBJS) $(TARGET)