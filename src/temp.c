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
volatile float fftOut[16];
volatile int display[16];
volatile float avg[16];
volatile int counter;
volatile int sample_count = 0; 
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
            
            send((1<<display[i]-1)-1,(1<<(15-i)));
        }
    for(int i = 0; i < 16; i++){
            send(0,0);
    }
   
}
ISR(TIMER0_COMPA_vect){
    counter++;
    
    if(counter ==100){
        for(int i = 0; i < 16; i++){
            int f = round(fftOut[i] *  0.0078125);
           if(f > display[i]){
               display[i]=f;
           }else{
               display[i]--;
           }
           
        }   
        counter = 0;
    }
} 
ISR(TIMER2_COMPA_vect){
    
}



int main(){
    //SETUP
    int adc;
     counter = 0;
    
    //int height[16];
    float hanning[32]; 
    scomplex samples[32];
    unsigned int reversedArray[32];
    for (int i = 0; i < 32; i++){
        reversedArray[i] = reversedNumber(i, 5);
    }   
    for (int i = 0; i < 32; i++){
      hanning[i]= 0.54 - 0.5  * cos(6.28318530718*i / 32.0);
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

      // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

    // 40000 Hz (16000000/((49+1)*8))
    OCR2A = 49;
    // CTC
    TCCR2A |= (1 << WGM21);
    // Prescaler 8
    TCCR2B |= (1 << CS21);
    // Output Compare Match A Interrupt Enable
    TIMSK2 |= (1 << OCIE2A);

     sei();
    float mean =0; 
    //LOOP
    
    
    while(1){
        mean =0;
        for(int i = 0; i < 32; i++){
            cli();
                      //Reads the analog value from pin PC0
            ADCSRA |= (1 << ADSC ); // start ADC conversion ./
            //wait til conversion is finnished 
            while(ADCSRA & (1 <<ADSC));
            adc = ADCL;
            adc  = (ADCH<<8) | (adc & 0xff);   
            //Subtract 512 (dc offset) from adc 
            //samples[reversedArray[i]].real = (adc-512)*hanning[reversedArray[i]] ;    

            //time domain so complex is zero  
            samples[i].imag = 0; 
            sei();
             
            
        }  
    
    fft(samples,32,5);
    for(uint16_t i = 0; i < 16; i++){
        fftOut[i] = (magnitude(samples[i]))  ;
    }
    
}
}
