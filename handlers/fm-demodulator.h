
#ifndef	__FM_DEMODULATOR__
#define	__FM_DEMODULATOR__

#include	"radio-constants.h"

class	demodulator {
private:
	float		Imin1;
	float		Qmin1;
	complex<float>	old_z;
	float		fm_afc;
	float		fm_cvt;
	float		K_FM;
	int32_t		inputRate;
public:
	demodulator 	(int32_t, float);
	~demodulator	(void);
float	demodulate	(complex<float> z);
};

#endif

