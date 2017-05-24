#
#ifndef	__FM_STEREO_HANDLER__
#define	__FM_STEREO_HANDLER__
#include	<stdio.h>
#include	"radio-constants.h"
#include	"radio-handler.h"
#include	"fm-demodulator.h"
#include	"fir-filters.h"
#include	"fft-filters.h"

class	HilbertFilter;
class	pllC;

class fm_stereoHandler : public radioHandler {
public:
		fm_stereoHandler (int32_t	fmRate,
	                          bool		deemphasis);
		~fm_stereoHandler (void);
std::complex<float> handle	(std::complex<float>);
private:
	int32_t		fmRate;
	std::complex<float> *Table;
	HilbertFilter	theHilbertFilter;
	fftFilter	pilotBandFilter;
	lowpassFIR	lrdiffFilter;
	lowpassFIR	lrplusFilter;
	pllC		*pilotRecover;
	bool		filtering;
	bool		deemphasis;
	float		K_FM;
	float		max_freq_deviation;
	float		norm_freq_deviation;
	demodulator	*my_demodulator;
	float		audioGain;
	float		audiogainAverage;
	float           peakLevel;
	int32_t         peakLevelcnt;
	float		Tau;
	float		alpha;
	float		xkm1;
	float		ykm1;
};
#endif

