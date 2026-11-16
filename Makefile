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

MAKE_SELF = $(PS3DEV)/bin/make_self
SFO_PY = $(PS3DEV)/bin/sfo.py
MAKE_PKG = $(shell find $(PS3DEV) -name make_pkg.py -type f 2>/dev/null | head -1)

all: $(TARGET).elf $(TARGET).self $(TARGET).pkg

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

$(TARGET).self: $(TARGET).elf
	$(MAKE_SELF) $(TARGET).elf $(TARGET).self

PARAM.SFO: sfo.xml
	python3 $(SFO_PY) sfo.xml PARAM.SFO

$(TARGET).pkg: $(TARGET).self PARAM.SFO
	@if [ -z "$(MAKE_PKG)" ]; then echo "make_pkg.py nao encontrado nesta imagem"; exit 1; fi
	python3 $(MAKE_PKG) --id $(APPID) --title "$(TITLE)" --sfo PARAM.SFO $(TARGET).self $(TARGET).pkg

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).self $(TARGET).pkg PARAM.SFO

.PHONY: all clean