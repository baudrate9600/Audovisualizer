/*
 * Author:  Olasoji Makinwa 
 * Date:    5 january 2020 
 * Description: 
 * This program converts audio from the time domain to the frequency domain using the fourier transform.
 * The transformed data is displayed on a 16x8 led matrix, this is done with shift register to drive and 
 * sink the current 
 */
#define __AVR_ATmega328P__

#define F_CPU 16000000
#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../include/fft.h"

 
  #define SER  2
#define RCLK  3
#define SRCLK 4
#define DSER  11
#define DRCLK  6
#define DSRCLK 13



void spi(void)
{
   
    SPCR |= (1 << MSTR);
    SPSR |= (1 << SPI2X);
    SPCR |= (1 << SPE);
    //SPCR = 0x50; // SPI enabled as Master, Mode0 at 4 MHz
}
void send(uint8_t led, uint16_t drain){
    PORTD &= ~(1 << DRCLK);
    SPDR = led;
    while (!(SPSR & (1 << SPIF)));
    SPDR = (drain >> 8);
    while (!(SPSR & (1 << SPIF)));
    SPDR = (drain & 0xFF);
    while (!(SPSR & (1 << SPIF))); 
    PORTD |= (1 << DRCLK);
}
  volatile int height[16];

ISR(TIMER1_COMPA_vect){
   
    for(int i = 0; i < 16; i++){
            send((1<<height[i]) -1,(1<<i));
        }
    for(int i = 0; i < 16; i++){
            send(0,0);
    }
   
}
 
int main(){
    //SETUP
    int adc;
    //int height[16];
    float hanning[32]; 
    scomplex samples[32];
    unsigned int reversedArray[32];
    for (int i = 0; i < 32; i++){
        reversedArray[i] = reversedNumber(i, 5);
    }   
    for (int i = 0; i < 32; i++){
      hanning[i]= 0.5-0.5 * cos(6.28318530718*i / 32.0);
    }
    DDRB |= (1 << PB3) | (1 << PB5);
    ADCSRA |= ( 1 << ADPS2 ) ;
    ADCSRA |= (1 << ADEN); /* enable ADC */
    DDRB |= ( 1 << PB2 );
    PORTB &= ~(1 << PB2);
    DDRD |=  (1 << DRCLK ) ;
    spi();

    //timer settings

    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
     OCR1A = 624;
     sei();
    //LOOP
    while(1){
        for(int i = 0; i < 32; i++){
                      //Reads the analog value from pin PC0
            ADMUX = (0xf0 & ADMUX) | PC0 ; 
            
            ADCSRA |= (1 << ADSC ); // start ADC conversion ./
            //wait til conversion is finnished 
            while(ADCSRA & (1 <<ADSC));
            adc = ADCL;
            adc  = (ADCH<<8) | (adc & 0xff);     
            //time domain so complex is zero  
            samples[i].imag = 0; 
            //Subtract 512 (dc offset) from adc 
            samples[reversedArray[i]].real = (adc - 512)*hanning[i] ;            
        }  
        fft(samples,32,5);
    for(uint16_t i = 0; i < 16; i++){
          height[i] = imagnitude(samples[i]) * 0.03125;
         // dwriteShift(1 << i, (1 << height[i])-1 );
        //  send((1 << height[i])-1,1<<i);
  
    }

    }
}
