#define __AVR_ATmega328P__
#define F_CPU 16000000
#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include "../include/fft.h"

#define SCK (1 << PB5)
#define SDA (1 << PB3)
#define DC (1 << PB2)
#define RESET (1 << PB1)
// ST7735 ROUTINES
#define SWRESET 0x01 // software reset
#define SLPOUT 0x11  // sleep out
#define DISPOFF 0x28 // display off
#define DISPON 0x29  // display on
#define CASET 0x2A   // column address set
#define RASET 0x2B   // row address set
#define RAMWR 0x2C   // RAM write
#define MADCTL 0x36  // axis control
#define COLMOD 0x3A  // color mode
// 1.8" TFT display constants
#define XSIZE 128
#define YSIZE 160
#define XMAX XSIZE - 1
#define YMAX YSIZE - 1
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

void spi(void)
{
    //SPCR |= (1 << SPE);
    //	SPCR |= (1 << MSTR);
    //SPSR |= (1 << SPI2X);
    SPCR = 0x50; // SPI enabled as Master, Mode0 at 4 MHz
}
//Write Byte (8 bits)
void writeByte(uint8_t byte)
{
    SPDR = byte;
    while (!(SPSR & (1 << SPIF)))
        ;
}
//Write Word (16 bits)
void writeWord(uint16_t word)
{

    SPDR = (word >> 8);
    while (!(SPSR & (1 << SPIF)))
        ;
    SPDR = (word & 0xFF);
    while (!(SPSR & (1 << SPIF)))
        ;
}
void resetTFT()
{
    PORTB &= ~RESET;
    _delay_ms(1);
    PORTB |= RESET;
    _delay_ms(200);
}
void writeCommand(uint8_t byte)
{
    PORTB &= ~DC;
    SPDR = byte;
    while (!(SPSR & (1 << SPIF)))
        ;
    PORTB |= DC;
}
void initDisplay(void)
{
    resetTFT();
    writeCommand(SLPOUT);
    _delay_ms(150);
    writeCommand(COLMOD);
    writeByte(0x05);
    writeCommand(DISPON);
}
//Set address Window to write pixels to
void setAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    writeCommand(CASET);
    writeWord(x0);
    writeWord(x1);
    writeCommand(RASET);
    writeWord(y0);
    writeWord(y1);
}
void Write565(int data, unsigned int count)
{
    writeCommand(RAMWR);
    for (; count > 0; count--)
    {
        SPDR = (data >> 8);
        while (!(SPSR & (1 << SPIF)))
            ;
        SPDR = (data & 0xFF);
        while (!(SPSR & (1 << SPIF)))
            ;
    }
}
void DrawPixel(uint8_t x, uint8_t y, int color)
{
    setAddressWindow(x, y, x, y);
    writeCommand(RAMWR);
    Write565(color, 1);
}
void clearScreen(int color)
{
    setAddressWindow(0, 0, YMAX, XMAX);
    writeCommand(RAMWR);
    Write565(color, 20480);
}
void HLine (uint8_t x0, uint8_t x1, uint8_t y, int color)
// draws a horizontal line in given color
{
 uint8_t width = x1-x0+1;
 setAddressWindow(x0,y,x1,y);
 Write565(color,width);
}
void VLine (uint8_t x, uint8_t y0, uint8_t y1, int color)
// draws a vertical line in given color
{
 uint8_t height = y1-y0+1;
 setAddressWindow(x,y0,x,y1);
 Write565(color,height);
}
void initAD(){
	ADMUX |= (1 << REFS0) ;
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); /* ADC clock prescaler /128 */
	ADCSRA |= (1 << ADEN); /* enable ADC */
}

int readAnalog(int pin){
	int adc;
	ADMUX = (0xf0 & ADMUX) | pin ;
	ADCSRA |= (1 << ADSC | 1 << ADIF); /* start ADC conversion */
	while(ADCSRA & (1 <<ADIF));
	
	adc = ADC;
	return adc;
	
}
#define N 64
int main()
{
    DDRB |= SDA | SCK | RESET | DC; //Set As Output
    DDRC &= ~(1 << PC0); 
    int bitspan = 6;
    unsigned int reversedArray[256];

    for (int i = 0; i < N; i++)
    {
        reversedArray[i] = reversedNumber(i, bitspan);
    }
    scomplex samples[N];
    

    PORTB &= ~RESET;
    spi();
    initDisplay();
    initAD();
    writeCommand(MADCTL);
    writeByte(0x60);
    clearScreen(BLACK);
    int width = YMAX - N;
    int offset = width/2.0f;

    for(int i =  0; i < N; i++){
        samples[i].imag = 0; 
        samples[reversedArray[i]].real = readAnalog(PC0);
        _delay_us(25);
    }
    fft(samples,N,5);
  
    for(int i = 1; i < N;i++){
        VLine(offset+i,0, 0.001 * magnitude(samples[i]),YELLOW);
    }
    while (1)
    {
    }
    
}

