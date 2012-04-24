CC = g++
CFLAGS = -Wall -g -c
LIBS = -lglui -lm
LINKER = g++
RM = rm
TARGET = semb

SOURCES := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SOURCES))

$(TARGET): $(OBJS)  
	$(LINKER) $(LIBS) -o ../$@ $(OBJS)

.cpp.o:
	$(CC) $(CFLAGS) $< 

clean:
	$(RM) *.o ../$(TARGET) 
