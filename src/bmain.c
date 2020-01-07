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

//Pin connections 

//Shift register that drives the leds (SN74HC595N)
#define  led_ser    PD2
#define  led_rclk   PD3
#define  led_srclk  PD4
//Shift register that draines the leds (TPIC6B595)
#define drain_ser   PD5
#define drain_rclk  PD6
#define drain_srclk PD7
#define SCK  PB5
#define SDA  PB3
#define NUM_SAMPLES 32 


/*
void ledBar(uint8_t height, int y){
    PORTD &= ~((1 << led_rclk) | (1 << drain_rclk));
   
    for(int i = 16; i >= 0; i--){
         PORTD &= ~((1 << led_srclk ) |(1 << drain_srclk));
        if(i <8){
            PORTD &= ~(1 << led_srclk );
        }else{
            if(  height > (i-8) ){
                PORTD |= ( 1 << led_ser );
            }else{
                PORTD &= ~( 1 << led_ser ); 
            }
            PORTD |= (1 << led_srclk);
        }
         if( (1 << y) & ( 1 << i)){
            PORTD |= ( 1 << drain_ser );
        }else{
            PORTD &= ~( 1 << drain_ser ); 
        }
        PORTD |= (1 << drain_srclk); 
    }
   

   
    PORTD |= ( 1 << drain_rclk) | ( 1 << led_rclk);
}
*/

void spi(void)
{
   
    SPCR |= (1 << MSTR);
    SPSR |= (1 << SPI2X);
    SPCR |= (1 << SPE);
    //SPCR = 0x50; // SPI enabled as Master, Mode0 at 4 MHz
}
void send(uint8_t led, uint16_t drain){
    PORTD &= ~(1 << drain_rclk);
    SPDR = led;
    while (!(SPSR & (1 << SPIF)));
    SPDR = (drain >> 8);
    while (!(SPSR & (1 << SPIF)));
    SPDR = (drain & 0xFF);
    while (!(SPSR & (1 << SPIF))); 
    PORTD |= (1 << drain_rclk);
}

volatile   int height[16];
volatile int k = 0;
/*ISR(TIMER1_COMPA_vect){
   
    for(int i = 0; i < 16; i++){
            send((1<<height[i]) -1,(1<<i));
        }
   
}*/
 
int main(){
    //Output pins 
   
    
    DDRD |=  (1 << drain_rclk ) ;
    DDRB |= ( 1 << SDA) | ( 1 <<SCK);
    DDRB |= ( 1 << PB2 );
    DDRC &= ~(1 << PC0);
    //Setup the adc 
    //ADMUX |= (1 << REFS0) ; AREF
	//ADCSRA |= (1 << ADPS0); /* ADC clock prescaler /128 */
	ADCSRA |= (1 << ADEN); /* enable ADC */

    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);
    TIMSK1 |= (1 << OCIE1A);
     OCR1A = 624;
    
    //init spi 
    PORTB &= ~(1 << PB2);
    spi();
  //  sei();
   
    int bitspan = 5; //log2(32)
    
    //This array is for the cooleytukey algorithm
    unsigned int reversedArray[NUM_SAMPLES];
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        reversedArray[i] = reversedNumber(i, bitspan);
    }   
    //Array to store the complex numbers from the fft 
    scomplex samples[NUM_SAMPLES];
    //height[0] =0;
    int adc;
    int low_bit;
    int high_bit;
    while(1){
          
        for(int i = 0; i < NUM_SAMPLES; i++){
          
            ADCSRA |= (1 << ADSC ); // start ADC conversion ./
            //wait til conversion is finnished 
            while(ADCSRA & (1 <<ADSC));
            low_bit  = ADCL;
	        high_bit = ADCH;

            adc = (high_bit << 8) | low_bit;
            //time domain so complex is zero  
            samples[i].imag = 0; 
            //Subtract 512 (dc offset) from adc 
            samples[reversedArray[i]].real = adc - 512;
        }       
      
        
        //Apply the fourier transform on the samples
        fft(samples,NUM_SAMPLES,bitspan);
        //nyquist sampling theorem, samples above N/2 are mirrored 
        for(int i =0; i < NUM_SAMPLES / 2;i++ ){
      
            height[i] = imagnitude(samples[i]) * 0.03125;//
            send(i,i);
             
        }
       

    }
 
}
