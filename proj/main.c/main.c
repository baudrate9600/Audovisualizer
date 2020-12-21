/*
 * Author:  Olasoji Makinwa 
 * Date:    5 january 2020 
 * Description: 
 * This program converts audio from the time domain to the frequency domain using the fourier transform.
 * The transformed data is displayed on a 16x8 led matrix, this is done with shift register to drive and 
 * sink the current 
 */

#define F_CPU 16000000
#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "fft.h"
#include "serial_io.h"
 
  #define SER  2
#define RCLK  3
#define SRCLK 4
#define DSER  11
#define DRCLK  6
#define DSRCLK 13
#define VOLTAGE_OFFSET 128 



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

uint8_t frame[16];
void put_pixel(uint8_t x, uint8_t y){
	frame[y] = 1 <<x ;	
}
void clear_frame(){
	for(int i = 0; i < 16; i++){
		frame[i] = 0; 
	}
}
void timer1(){
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

   // 10000 Hz (16000000/((24+1)*64))
  OCR1A = 24;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 64
  TCCR1B |= (1 << CS11) | (1 << CS10);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
}
void timer2(){
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  // 20000 Hz (16000000/((24+1)*32))
  OCR2A = 24;
  // CTC
  TCCR2A |= (1 << WGM21);
  // Prescaler 32
  TCCR2B |= (1 << CS21) | (1 << CS20);
  // Output Compare Match A Interrupt Enable
  TIMSK2 |= (1 << OCIE2A);
}
void timer2_stop(){
  TIMSK2 &= ~(1 << OCIE2A);
}
void timer2_start(){
  TIMSK2 |= (1 << OCIE2A);
}
volatile uint16_t render_timer; 
volatile uint32_t general_timer; 
volatile scomplex sample_vec[32];
volatile int sample_counter;
volatile uint16_t adc;
volatile uint8_t sample_done; 
ISR(TIMER1_COMPA_vect) {
	render_timer++;
	general_timer++;
}
ISR(TIMER2_COMPA_vect){
	if(sample_done == 0){
		if(sample_counter == -1){
			ADCSRA |= (1<<ADSC);
			sample_counter++;
		}else if(sample_counter == 31){
			sample_vec[reversed[sample_counter]].real = ADCH-VOLTAGE_OFFSET;
			sample_done = 1;
			timer2_stop(); 
			sample_counter = -1; 
		}else{
			sample_vec[reversed[sample_counter]].real = ADCH - VOLTAGE_OFFSET ;
			ADCSRA |= (1<<ADSC);
			sample_counter++;
		}	
	}
	
}
void render(uint16_t pwm){
	static uint8_t counter = 0; 
	if(render_timer >= pwm){
			render_timer = 0; 
			if(counter == 16){
				send(0,0xFF);
				counter = 0;
			}else{
				send(frame[counter],1 << counter );
				counter++;
			}
		}
	}

void demo(){
	frame[15] = 0xFF; 
	frame[14] = 0b10010000;
	frame[13] = 0b10010000;
	frame[12] = 0b10010000;
	frame[10] = 0xFF; 
	frame[9] = 0b10010000;
	frame[8] = 0b10010000;
	frame[7] = 0b10010000;

	frame[5] = 0b10000000;
	frame[4] = 0b10000000;
	frame[3] = 0b11111111;
	frame[2] = 0b10000000;
	frame[1] = 0b10000000;
	

}

void init_adc(){
	ADCSRA |= ( 1 << ADPS2 ) | (1 << ADPS0)| (1 << ADEN);  //125 kHz
    ADMUX |= (0xf0 & ADMUX) | PC0 ; 
	ADMUX |= (1 << ADLAR);
	ADCSRA |= (1 << ADSC ); // start ADC conversion ./
            //wait til conversion is finnished 
    while(ADCSRA & (1 <<ADSC));
}
void columns( uint8_t * column_vector){
	for(int i = 0; i < 16; i++){
		frame[i] = (1 << column_vector[i]) -1; 
	}
}
void decay_columns(uint16_t decay_speed){
	if((general_timer % decay_speed) == 0){
		for(int i = 0; i < 16; i++){
			frame[i] = frame[i] >> 1; 
		}
	}
}
int main(){
	/*Variable initialization */
	sample_counter = -1; 	
	
	/* Pin initialization */ 
    DDRB |= (1 << PB3) | (1 << PB5);
    DDRB |= ( 1 << PB2 );
    PORTB &= ~(1 << PB2);
    DDRD |=  (1 << DRCLK ) ;
    spi();
	
		/* Render Cycle */
	demo();	
	uint8_t column_vec[16] ;
	columns(column_vec);
	init_adc();
	initSerial();
	sei();
	char buffer[20];
	_delay_ms(1);
	timer1();
	timer2();
    while(1){
			if(sample_done == 1){
		writeString("\nSamples = [");
			for (int i = 0; i < 32; i++){
				writeInt((int)sample_vec[i].real);
				writeChar(' ');
			}	
			for(int i = 0; i < 32; i++){
				int k = reversed[i];
				sample_vec[i].real = hanning[k] * sample_vec[i].real;
				sample_vec[i].imag = 0; 
			}
		
			writeString(" ]");
		    fft(sample_vec);
			for(uint16_t i = 0; i < 16; i++){
				 column_vec[i] = (magnitude(sample_vec[i]))  ;
			 }
			columns(column_vec);
			sample_done = 2; 
			timer2_start();	
			writeString("\navr_fft =[");
			for (int i = 0; i < 16; i++){
				writeInt((int)column_vec[i]);
				writeChar(' ');
			}
			writeString("0 ]");
		/*	writeString("\n  cos lut \n");
			for (int i = 0; i < 5; i++){
				for(int j = 0; j < 16; j++){
					writeInt(cos_lut[i][j]*100);
					writeChar(' ');
				}
				writeChar('\n');
			}	*/
		}
		decay_columns(200);
//		render(1);
		
	//	clear_frame(); 
	}
}
