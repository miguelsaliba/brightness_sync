CXX = g++
CXXFLAGS = -g
objects = main.o

TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CXX) -o brightness_sync $(TARGET).cpp

$(TARGET).o: $(TARGET).cpp
	$(CXX) -c $(TARGET).cpp

clean:
	$(RM) -f $(TARGET).o $(TARGET)
