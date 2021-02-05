# making leaner with overrides - 20200407, 20201224

TARGET = ./bin/lmsmonitor

# need https support so ssl and crypto added
LIBS =  -pthread -lrt -L./lib -lssl -lcrypto

CC = g++

CFLAGS = -g -Wall -std=c11 -Ofast -fPIC -fno-rtti  -Wno-unused-variable \
-Wno-unused-parameter -Wstringop-truncation -mfpu=vfp -mfloat-abi=hard \
-fpermissive -Wunused-function -Wno-stringop-truncation -Wunused-but-set-variable -Wstringop-overflow \
-funsafe-math-optimizations -ffast-math -pipe -I. -I./src

CAPTURE_BMP = -DCAPTURE_BMP -I./src -I./capture

bin:
	mkdir bin

.PHONY: default all clean

default: $(TARGET)

all: bin std default

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard ./src/*.cpp)) $(patsubst %.cc, %.o, $(wildcard ./src/*.cc)) $(patsubst %.c, %.o, $(wildcard ./src/*.c))
HEADERS = $(wildcard ./src/*.h) $(wildcard ./font/*.h)

%.o: %cc $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

std:
    %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS)  -g -Wall $(LIBS) -o $@

clean:
	-rm -f capture/*.o
	-rm -f src/*.o
	-rm -f *.o
	-rm -f $(TARGET)
