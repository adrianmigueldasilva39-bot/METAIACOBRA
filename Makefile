TARGET = slips3
TITLE = slips3
APPID = SLIP00001

PS3DEV ?= /usr/local/ps3dev
PSL1GHT ?= $(PS3DEV)/ppu
PORTLIBS ?= $(PS3DEV)/portlibs/ppu

CXX = ppu-g++
CC = ppu-gcc

CFLAGS = -O2 -Wall -mcpu=cell -I$(PORTLIBS)/include -I$(PSL1GHT)/include
CXXFLAGS = $(CFLAGS) -std=gnu++98
LDFLAGS = -L$(PORTLIBS)/lib -L$(PSL1GHT)/lib

LIBS = -lSDL -lSDLmain -laudio -lrsx -lgcm_sys -lsysutil -lio -lnet -lsysmodule -lm -lstdc++

SOURCES = source/main.cpp source/game.cpp source/render.cpp
OBJS = $(SOURCES:.cpp=.o)

all: $(TARGET).elf $(TARGET).pkg

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

PARAM.SFO: sfo.xml
	python3 $(PSL1GHT)/bin/sfo.py sfo.xml PARAM.SFO || \
	python3 $(PS3DEV)/ppu/bin/sfo.py sfo.xml PARAM.SFO || \
	$(PSL1GHT)/bin/sfo sfo.xml PARAM.SFO

$(TARGET).pkg: $(TARGET).elf PARAM.SFO
	$(PSL1GHT)/bin/make_self $(TARGET).elf $(TARGET).self || cp $(TARGET).elf $(TARGET).self
	python3 $(PS3DEV)/ppu/bin/make_pkg.py --id $(APPID) --title "$(TITLE)" --sfo PARAM.SFO $(TARGET).self $(TARGET).pkg || touch $(TARGET).pkg

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).self $(TARGET).pkg PARAM.SFO

.PHONY: all clean