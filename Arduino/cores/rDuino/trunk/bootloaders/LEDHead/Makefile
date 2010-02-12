# Makefile for rDuinoLEDHeadBoot
# $Id$

# program name should not be changed...
PROGRAM    = rDuinoLedHeadBoot

# enter the target CPU frequency
AVR_FREQ   = 16000000L

MCU_TARGET = atmega644p
LDSECTION  = --section-start=.text=0xF800

OBJ        = $(PROGRAM).o
OPTIMIZE   = -O2

DEFS       = 
LIBS       = 

CC         = avr-gcc


# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DF_CPU=$(AVR_FREQ) $(DEFS)
# override LDFLAGS       = -Wl,$(LDSECTION)
override LDFLAGS       = -Wl,-Map,$(PROGRAM).map,$(LDSECTION)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
SIZE           = avr-size

MSG_SIZE_AFTER = Size:

ELFSIZE = $(SIZE) -A $(PROGRAM).elf

all: CFLAGS += '-DMAX_TIME_COUNT=F_CPU>>4' '-DNUM_LED_FLASHES=2' -DBAUD_RATE=57600
all: $(PROGRAM).hex $(PROGRAM).lss sizeafter

sizeafter:
	@if [ -f $(PROGRAM).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi

$(PROGRAM).hex: $(PROGRAM).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	
$(PROGRAM).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	
$(OBJ): $(PROGRAM).c
	avr-gcc $(CFLAGS) $(LDFLAGS) -c -g -O2 -Wall -mmcu=$(MCU_TARGET) $(PROGRAM).c -o $(PROGRAM).o

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.lss: %.elf
	$(OBJDUMP) -h -S $< > $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

clean:
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex
	