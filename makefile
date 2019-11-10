PORT=/dev/ttyUSB0
MCU=atmega328p
CFLAGS=-g -Wall -mcall-prologues -mmcu=$(MCU) -Os -lm
LDFLAGS=-Wl,-gc-sections -Wl,-relax
CC=avr-gcc
TARGET=Audiovisualizer
OBJECT_FILES= src/main.o src/fft.o
baud=57600
SRC_DIR = src 
OBJ_DIR = obj
SRC = $(wildcard $(SRC_DIR)/*.c)


all: $(TARGET).hex

clean:
	rm -f *.o *.hex *.obj *.hex

%.hex: %.obj
	avr-objcopy -R .eeprom -O ihex $< $@

%.obj: $(OBJECT_FILES)
	$(CC) $(CFLAGS) $(OBJECT_FILES) $(LDFLAGS) -o $@

program: $(TARGET).hex
	avrdude -p $(MCU) -c arduino -b$(baud) -P $(PORT) -U flash:w:$(TARGET).hex