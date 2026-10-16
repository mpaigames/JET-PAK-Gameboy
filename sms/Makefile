# Master System build - needs SDCC and devkitSMS (https://github.com/sverx/devkitSMS)
DEVKITSMS ?= /opt/devkitSMS
IHX2SMS   ?= ihx2sms
CFLAGS = -mz80 --opt-code-speed --peep-file $(DEVKITSMS)/crt0/peep-rules.txt --std-sdcc99 -DPLAT_SMS -I../common -I$(DEVKITSMS)/SMSlib/src
jetpak.sms: main.rel game.rel scene.rel
	sdcc -o jetpak.ihx -mz80 --no-std-crt0 --data-loc 0xC000 $(DEVKITSMS)/crt0/crt0_sms.rel main.rel game.rel scene.rel $(DEVKITSMS)/SMSlib/SMSlib.lib
	$(IHX2SMS) jetpak.ihx jetpak.sms
main.rel: main.c tiles_sms.h ../common/tileids.h ; sdcc $(CFLAGS) -c main.c
game.rel: ../common/game.c ../common/game.h ../common/config.h ; sdcc $(CFLAGS) -c ../common/game.c -o game.rel
scene.rel: ../common/scene.c ../common/tileids.h ; sdcc $(CFLAGS) -c ../common/scene.c -o scene.rel
clean:
	rm -f *.rel *.ihx *.sms *.lst *.map *.sym *.asm *.noi *.lk
