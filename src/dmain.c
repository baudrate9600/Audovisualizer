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

#define NUM_SAMPLES

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
volatile float fftOut[32];
volatile int frame[16];
volatile int display[16];
volatile float avg[16];
volatile int counter;
volatile int counter2; 
volatile int logindex[32];
int moveto(int old, int new){
    if(old == new) return new;
    else{
        if(old > new){
            return -1; 
        }else{
            return +1;
        }
    } 
}
ISR(TIMER1_COMPA_vect){
     
    for(int i = 0; i < 16; i++){
            
            send((1<<frame[i]-1)-1,(1<<(15-i)));
        }
    for(int i = 0; i < 16; i++){
            send(0,0);
    }
   
}
ISR(TIMER0_COMPA_vect){
    counter++;
    counter2++;
    if(counter2 == 100){
        for(int i = 0; i < 16; i++){
        if(display[i] < frame[i]){
            frame[i]--;
        }else{
            frame[i] = display[i];
        }    
    }
    counter2 = 0;
    }
    
    if(counter ==200){
        for(int i = 0; i < 16; i++){
            int f =  (round(fftOut[logindex[i]])  * 0.0125);
            display[i]=f;
        }   
        counter = 0;
    }
    
    
} 


int main(){
    //SETUP
    int adc;
     counter = 0;
    
    //int height[16];
    float hanning[64]; 
    scomplex samples[64];
    unsigned int reversedArray[64];
    for (int i = 0; i < 64; i++){
        reversedArray[i] = reversedNumber(i, 6);
    }   
    for (int i = 0; i < 64; i++){
      hanning[i]= 0.5 - 0.5  * cos(6.28318530718*i / 64.0);
    }
    for(int i = 0; i < 16; i++){
        logindex[i] = round(31*log10(1+9*i/15.0));
    }
    DDRB |= (1 << PB3) | (1 << PB5);
    ADCSRA |= ( 1 << ADPS2 ) | (1<<ADPS0);
    ADCSRA |= (1 << ADEN); /* enable ADC */
    ADMUX = (0xf0 & ADMUX) | PC0 ; 
    DDRB |= ( 1 << PB2 );
    PORTB &= ~(1 << PB2);
    DDRD |=  (1 << DRCLK ) ;
    spi();

    //timer settings
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 312;

    //Timer0
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS02);
    TIMSK0 |= (1 << OCIE0A); 
    OCR0A = 30; 


     sei();
    float mean =0; 
    
    //LOOP
    float test[32]; 
    for(int i = 0; i < 32; i++){
        test[i] = 300 * sin(6.28318530718 * 100 * 0.000025 * i );
    }
    while(1){
        mean =0;
        for(int i = 0; i < 64; i++){
            cli();
                      //Reads the analog value from pin PC0
            ADCSRA |= (1 << ADSC ); // start ADC conversion ./
            //wait til conversion is finnished 
            while(ADCSRA & (1 <<ADSC));
            adc = ADCL;
            adc  = (ADCH<<8) | (adc & 0xff);   
            //Subtract 512 (dc offset) from adc 
            samples[reversedArray[i]].real = (adc-512)*hanning[reversedArray[i]] ;    
            //time domain so complex is zero  
            samples[i].imag = 0; 
            sei();
           
            
        }  
    //mean/32.0;
    //Remove DC offset from signal by taking the mean and subtracting it from the samples
    //for(int i =1; i < 32;i++){
     //   samples[reversedArray[i]].real = (samples[reversedArray[i]].real -mean)*hanning[i];
    //}
  //  samples[0].real =0;
    //Apply the fourier transform on the samples
    fft(samples,64,6);
    for(int i = 0; i < 32 ; i++){
        fftOut[i] = magnitude(samples[i])  ;
    }
    
}
}
