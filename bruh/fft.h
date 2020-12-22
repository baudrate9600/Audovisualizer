//Fast fourier transform for avr 
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define N_SAMPLES 64
#include <avr/io.h>
typedef struct scomplex{
	float real;
	float imag;
}scomplex;


unsigned int Log2n(unsigned int n );
unsigned int reversedNumber(unsigned int numberToBeReversed, unsigned int bitSpan);
scomplex multiply(scomplex x, scomplex y);
scomplex add(scomplex x, scomplex y);
scomplex subtract(scomplex x, scomplex y);
float magnitude(scomplex x);
int imagnitude(scomplex x);
void fft(scomplex * samples );
//float cos_lut[5][16];
//float sin_lut[5][16];
extern uint8_t reversed[N_SAMPLES];
extern float hanning[N_SAMPLES];
float lut_cos[63];
float lut_sin[63]; 
