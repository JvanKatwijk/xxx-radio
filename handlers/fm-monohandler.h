#
#ifndef	__FM_MONO_HANDLER__
#define	__FM_MONO_HANDLER__
#include	<stdio.h>
#include	"radio-constants.h"
#include	"radio-handler.h"
#include	"fm-demodulator.h"

class fm_monoHandler : public radioHandler {
public:
		fm_monoHandler (int32_t	fmRate,
	                        bool	deemphasis);
		~fm_monoHandler (void);
std::complex<float> handle	(std::complex<float>);
private:
	int32_t		fmRate;
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
};
#endif

