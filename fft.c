#include <math.h>
#define PI 3.14159265359
#define TWO_PI 6.28318530718
typedef struct scomplex{
	float real;
	float imag;
}scomplex;

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

void bitreversal(scomplex * reversed){
    
}
void fft(scomplex * samples, scomplex * freqbins, int N){
	int counter = 0;
	int numButterflies = N/2;
	//determines the offset between odd and even 
	int line = 1;
	//number of stages 
	int numStages = log2f(N);
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