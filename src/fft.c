#include <math.h>
#include "../include/fft.h"
#define PI 3.14159265359
#define TWO_PI 6.28318530718

unsigned int Log2n(unsigned int n ){
    return (n>1) ? 1 + Log2n(n/2) : 0;
}
unsigned int reversedNumber(unsigned int numberToBeReversed, unsigned int bitSpan){
    unsigned int num = 0;
     {
    for (int i = 0; i < bitSpan; i++)
        if (numberToBeReversed & (1 << (bitSpan-i-1)))
        {
            num |= (1 << i);
        }else
        {
            num &= ~(1 << i );
        }
    }
    return num;
}

scomplex multiply(scomplex x, scomplex y){
	scomplex mult;
	mult.real = x.real * y.real -(x.imag*y.imag);
	mult.imag = x.real * y.imag + x.imag *y.real;
	return mult;
}
scomplex add(scomplex x, scomplex y){
	scomplex ad; 
	ad.real = x.real + y.real; 
	ad.imag = x.imag + y.imag;
	return ad;
}
scomplex subtract(scomplex x, scomplex y){
	scomplex sub; 
	sub.real = x.real - y.real; 
	sub.imag = x.imag - y.imag;
	return sub;
}
float magnitude(scomplex x){
	return sqrt( x.real*x.real + x.imag*x.imag);
}
int imagnitude(scomplex x){
	return (int) sqrt(x.real*x.real + x.imag*x.imag);
}


void fft(scomplex * samples, scomplex * freqbins, int N,int logN){
	int counter = 0;
	int numButterflies = N/2;
	//determines the offset between odd and even 
	int line = 1;
	//number of stages 
	int numStages = logN;
	//        N
	// X[k] = Σ x_k W_n
	//       n=0 
	scomplex cmplxPart;	
	
	int odd,even;
	for (int i = 0; i < numStages; i++){
		counter = 0;
		int offset = 0; 
		for(int j = 0; j < numButterflies; j++){
			if(counter == line){
				counter = 0;
				offset += line;
			}	
			cmplxPart.real = cos( -TWO_PI * (j%line) / (2<<i) );
			cmplxPart.imag = sin( -TWO_PI * (j%line) / (2<<i ));

			even = j+offset;
			odd = even + line;

			cmplxPart = multiply(samples[odd],cmplxPart);
			samples[odd] = subtract(samples[even],cmplxPart);
			samples[even] = add(samples[even], cmplxPart);
			counter ++;

		}
		line = line * 2;
	}

}