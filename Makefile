CXX = g++
CXXFLAGS = -g
objects = main.o

TARGET = main

all: $(TARGET)

$(TARGET): main.o
	$(CXX) -o brightness_sync $(TARGET).cpp

$(TARGET).o: main.cpp
	$(CXX) -c $(TARGET).cpp

clean:
	$(RM) -f $(TARGET).o $(TARGET)
