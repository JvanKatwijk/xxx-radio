#
#include	"fm-demodulator.h"
#define DCAlpha 0.0001

	demodulator::demodulator 	(int32_t rateIn, float K_FM) {
	this	-> inputRate	= rateIn;
	this	-> K_FM		= K_FM;
	fm_afc			= 0;
	fm_cvt			= 0.90 * (rateIn / (M_PI * 150000));
}

	demodulator::~demodulator	(void) {
}

float	demodulator::demodulate		(complex<float> z) {
float	res;
float	I, Q;

	if (abs (z) <= 0.001)
	   I = Q = 0.001;	// do not make these 0 too often
	else { 
	   I = real (z) / abs (z);
	   Q = imag (z) / abs (z);
	}

	z	= complex<float> (I, Q);
	res	= arg (z * conj (old_z));
	old_z	= z;

	fm_afc  = (1 - DCAlpha) * fm_afc + DCAlpha * res;
	res     = (res - fm_afc) * fm_cvt;
	res     /= K_FM;
        return res;
}

