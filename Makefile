# making leaner with overrides - 20200407, 20201224

TARGET = ./bin/lmsmonitor

# need https support so ssl and crypto added
LIBS =  -pthread -lrt -L./lib -lssl -lcrypto

CC = g++

CFLAGS4 = -g -Wall -std=c11 -Ofast -lrt -fPIC -fno-rtti -mfpu=neon-vfpv4  \
-Wstringop-truncation -Wno-unused-variable -Wno-unused-parameter \
-fpermissive -Wunused-function -Wstringop-truncation -Wunused-but-set-variable -Wstringop-overflow \
-mfloat-abi=hard -march=armv7-a -mtune=cortex-a7 -ffast-math -pipe -I. -I./src

CFLAGS4 = -g -Wall -std=c11 -Ofast -lrt -fPIC -fno-rtti -mfpu=neon-vfpv4  \
-Wstringop-truncation -Wno-unused-variable -Wno-unused-parameter \
-fpermissive -Wunused-function -Wstringop-truncation -Wunused-but-set-variable -Wstringop-overflow \
-mfloat-abi=hard -ffast-math -pipe -I. -I./src

CFLAGS3 = -g -Wall -std=c11 -Ofast -lrt -fPIC -fno-rtti  -Wno-unused-variable \
-Wno-unused-parameter -Wstringop-truncation -mfpu=vfp -mfloat-abi=hard \
-fpermissive -Wunused-function -Wstringop-truncation -Wunused-but-set-variable -Wstringop-overflow \
-march=armv6zk -mtune=arm1176jzf-s -funsafe-math-optimizations -ffast-math -pipe -I. -I./src

CFLAGS3 = -g -Wall -std=c11 -Ofast -fPIC -fno-rtti  -Wno-unused-variable \
-Wno-unused-parameter -Wstringop-truncation -mfpu=vfp -mfloat-abi=hard \
-fpermissive -Wunused-function -Wstringop-truncation -Wunused-but-set-variable -Wstringop-overflow \
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
	$(CC) $(CFLAGS3) -c $< -o $@

std:
    %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS3) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f capture/*.o
	-rm -f src/*.o
	-rm -f *.o
	-rm -f $(TARGET)
