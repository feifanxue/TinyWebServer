CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../code/sql/*.cpp ../code/main.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient  -L /usr/local/lib /usr/local/lib/libjsoncpp.a

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)




