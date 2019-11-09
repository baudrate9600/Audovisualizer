#define __AVR_ATmega32U4__ 
#define F_CPU 16000000
#include <avr/io.h> 
#include <util/delay.h>

#define SCK     (1 << PB5) 
#define SDA     (1 << PB3)
#define DC      (1 << PB2)
#define RESET   (1 << PB1)
// ST7735 ROUTINES
#define SWRESET 0x01 // software reset
#define SLPOUT 0x11 // sleep out
#define DISPOFF 0x28 // display off
#define DISPON 0x29 // display on
#define CASET 0x2A // column address set
#define RASET 0x2B // row address set
#define RAMWR 0x2C // RAM write
#define MADCTL 0x36 // axis control
#define COLMOD 0x3A // color mode
// 1.8" TFT display constants
#define XSIZE 128
#define YSIZE 160
#define XMAX XSIZE-1
#define YMAX YSIZE-1
// Color constants
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x0400
#define LIME 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

void spi(void){
    //SPCR |= (1 << SPE);
//	SPCR |= (1 << MSTR);
	//SPSR |= (1 << SPI2X);
     SPCR = 0x50; // SPI enabled as Master, Mode0 at 4 MHz 
}
void writeData(uint8_t byte){
    SPDR = byte;
    while(!(SPSR & (1<<SPIF)));
}
void resetTFT(){
    PORTB &= ~RESET;
    _delay_ms(1); 
    PORTB |= RESET; 
    _delay_ms(200);
}
void writeCommand(uint8_t byte){
    PORTB &= ~DC;
    SPDR = byte;
    while(!(SPSR & (1<<SPIF)));
    PORTB |= DC;

}
void initDisplay(void){
    resetTFT();
    writeCommand(SLPOUT);
    _delay_ms(150);
    writeCommand(COLMOD);
    writeData(0x05);
    writeCommand(DISPON);
    
}

int main(){
   DDRB |= SDA | SCK | RESET | DC;//Set As Output 
   PORTB &= ~RESET;
   spi();
   initDisplay();
}
