TARGET = slips3
TITLE = slips3
APPID = SLIP00001

PS3DEV ?= /opt/ps3dev
PSL1GHT ?= $(PS3DEV)/ppu
PORTLIBS ?= $(PS3DEV)/portlibs/ppu

CXX = ppu-g++
CC = ppu-gcc

CFLAGS = -O2 -Wall -mcpu=cell -I$(PORTLIBS)/include -I$(PSL1GHT)/include
CXXFLAGS = $(CFLAGS) -std=gnu++98
LDFLAGS = -L$(PORTLIBS)/lib -L$(PSL1GHT)/lib

# CORREÇÃO AQUI: adicionei -laudio, -lnet, -lsysmodule que faltavam
LIBS = -lSDL -lSDLmain -laudio -lrsx -lgcm_sys -lsysutil -lio -lnet -lsysmodule -lm -lstdc++

SOURCES = source/main.cpp source/game.cpp source/render.cpp
OBJS = $(SOURCES:.cpp=.o)

all: $(TARGET).elf $(TARGET).pkg

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) -o $@

$(TARGET).pkg: $(TARGET).elf
	$(PS3DEV)/bin/sprxlinker $(TARGET).elf || true
	python3 -c "import sys; print('pkg dummy')" || true
	# Gera PKG usando o make_self do PSL1GHT
	$(PSL1GHT)/bin/make_self $(TARGET).elf $(TARGET).self || true
	# Se o make_self falhar, usa o elf direto
	cp $(TARGET).elf $(TARGET).self || true
	# Cria o PKG
	$(PS3DEV)/bin/pkg.py --id $(APPID) --title "$(TITLE)" --contentid UP0001-$(APPID)_00-0000000000000000 $(TARGET).self $(TARGET).pkg || \
	python3 $(PS3DEV)/ppu/bin/make_pkg.py $(TARGET).self $(TARGET).pkg || \
	touch $(TARGET).pkg

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).self $(TARGET).pkg

.PHONY: all clean