# Game Boy build - needs GBDK-2020 (https://github.com/gbdk-2020/gbdk-2020)
GBDK_HOME ?= /opt/gbdk
LCC = $(GBDK_HOME)/bin/lcc
SRC = main.c ../common/game.c ../common/scene.c
jetpak.gb: $(SRC) tiles_gb.h ../common/tileids.h
	$(LCC) -DPLAT_GB -I../common -Wm-yn"JETPAK" -o jetpak.gb $(SRC)
clean:
	rm -f *.gb *.o *.lst *.map *.sym *.asm *.ihx *.noi ../common/*.o ../common/*.lst ../common/*.asm ../common/*.sym
