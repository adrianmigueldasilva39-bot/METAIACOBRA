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
	python3 $(PS3DEV)/bin/sfo.py sfo.xml PARAM.SFO

$(TARGET).self: $(TARGET).elf
	$(PS3DEV)/bin/make_self $(TARGET).elf $(TARGET).self

# make_pkg: caminho ainda não confirmado, ver a saída do "Procurar ferramentas"
$(TARGET).pkg: $(TARGET).self PARAM.SFO
	python3 $(PS3DEV)/bin/make_pkg.py --id $(APPID) --title "$(TITLE)" --sfo PARAM.SFO $(TARGET).self $(TARGET).pkg

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).self $(TARGET).pkg PARAM.SFO

.PHONY: all clean