#include <math.h>
#include "fft.h"
#include <avr/io.h>
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define FIXED_POINT 100000

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


void fft(scomplex * samples){
	int counter = 0;
	//determines the offset between odd and even 
	int line = 1;
	//number of stages 
	//        N
	// X[k] = Σ x_k W_n
	//       n=0 
	scomplex cmplxPart;	
	
	int odd,even;
	for (int i = 0; i < 5; i++){
		counter = 0;
		int offset = 0; 
		for(int j = 0; j < 16; j++){
			if(counter == line){
				counter = 0;
				offset += line;
			}	
			cmplxPart.real = cos_lut[i][j];
			cmplxPart.imag = sin_lut[i][j];

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

float cos_lut[5][16] = {
        {1,         1,         1,         1,         1,         1,         1,         1,         1,         1,         1,         1,         1,         1,         1,         1},
        {1,         0,         1,         0,         1,         0,         1,         0,         1,         0,         1,         0,         1,         0,         1,         0},
        {1,   0.70711,         0,  -0.70711,         1,   0.70711,         0,  -0.70711,         1,   0.70711,         0,  -0.70711,         1,   0.70711,         0,  -0.70711},
        {1,   0.92388,   0.70711,   0.38268,         0,  -0.38268,  -0.70711,  -0.92388,         1,   0.92388,   0.70711,   0.38268,         0,  -0.38268,  -0.70711,  -0.92388},
        {1,   0.98079,   0.92388,   0.83147,   0.70711,   0.55557,   0.38268,   0.19509,         0,  -0.19509,  -0.38268,  -0.55557,  -0.70711,  -0.83147,  -0.92388,  -0.98079}
};

float sin_lut[5][16] ={
        {0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0,         0},
        {0,        -1,         0,        -1,         0,        -1,         0,        -1,         0,        -1,         0,        -1,         0,        -1,         0,        -1},
        {0,  -0.70711,        -1,  -0.70711,         0,  -0.70711,        -1,  -0.70711,         0,  -0.70711,        -1,  -0.70711,         0,  -0.70711,        -1,  -0.70711},
        {0,  -0.38268,  -0.70711,  -0.92388,        -1,  -0.92388,  -0.70711,  -0.38268,         0,  -0.38268,  -0.70711,  -0.92388,        -1,  -0.92388,  -0.70711,  -0.38268},
        { 0,  -0.19509,  -0.38268,  -0.55557,  -0.70711,  -0.83147,  -0.92388,  -0.98079,        -1,  -0.98079,  -0.92388,  -0.83147,  -0.70711,  -0.55557,  -0.38268,  -0.19509},
};

float hanning[32] ={0,0.0096074,0.03806,0.084265,0.14645,0.22221,0.30866,0.40245,0.5,0.59755,0.69134,0.77779,0.85355,0.91573,0.96194,0.99039,1,0.99039,0.96194,0.91573,0.85355,0.77779,0.69134,0.59755,0.5,0.40245,0.30866,0.22221,0.14645,0.084265,0.03806,0.0096074};
uint8_t reversed[32] = {0,16,8,24,4,20,12,28,2,18,10,26,6,22,14,30,1,17,9,25,5,21,13,29,3,19,11,27,7,23,15,31};