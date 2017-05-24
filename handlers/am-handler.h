
#
#ifndef	__AM_PROCESSOR__
#define	__AM_PROCESSOR__
#include	<stdio.h>
#include	"radio-constants.h"
#include	"radio-handler.h"
#include	"fir-filters.h"

class amHandler: public radioHandler {
public:
		amHandler	(int32_t audioRate, int16_t Mode);
		~amHandler	(void);
std::complex<float> handle	(std::complex<float>);
private:
	int32_t		audioRate;
	int16_t		Mode;
	int32_t		fmRate;
	float		dc_alpha;
	float		m_w1;
	HilbertFilter	SSB_Filter;
	lowpassFIR	usb_filter;
	bandpassFIR	lsb_filter;
};
#endif
