CXX = g++ -Wall -std=c++0x
CXXFLAGS = `pkg-config --cflags opencv`
LDFLAGS = `pkg-config --libs opencv` -lpthread 
TARGET = main_parallel
SOURCE = $(TARGET).cpp #feature.cpp recognizer.cpp

TARGET:
	$(CXX) $(CXXFLAGS) $(SOURCE) $(LDFLAGS) -o $(TARGET)   

.PHONY: clean
clean:
	rm -f *.o *~ $(TARGET)