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

void ledBar(uint8_t height, int y){
    PORTD &= ~(1 << led_rclk);
   
    for(int i = 7; i >= 0; i--){
         PORTD &= ~( 1 << led_srclk );
        
        if( ( height > i ){
            PORTD |= ( 1 << led_ser );
        }else{
            PORTD &= ~( 1 << led_ser ); 
        }
    
        PORTD |= (1 << led_srclk); 
    }
   
    PORTD |= ( 1 << led_rclk); 

    PORTD &= ~(1 << drain_rclk);
    for(int i = 15; i >= 0; i--){
        PORTD &= ~( 1 << drain_srclk );
        
        if( (1 << y) & ( 1 << i)){
            PORTD |= ( 1 << drain_ser );
        }else{
            PORTD &= ~( 1 << drain_ser ); 
        }
      
        PORTD |= (1 << drain_srclk); 
    }
     
   
    PORTD |= ( 1 << drain_rclk); 
}


int main(){
    //Output pins 
    DDRD |= ( 1 << led_ser ) | ( 1 << led_rclk ) | ( 1 << led_srclk );
    DDRD |= ( 1 << drain_ser ) | (1 << drain_rclk ) | (1 << drain_srclk);
    //input pin
    //PORTC |= ( 1 << PC0);
    while(1){
    for(int i = 0; i < 15; i++){
        ledBar(i, i);
        _delay_ms(100);
    }
    }
 
}
