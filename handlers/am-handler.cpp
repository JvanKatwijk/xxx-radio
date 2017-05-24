#include	"am-handler.h"

	amHandler::amHandler	(int32_t audioRate,
	                         int16_t Mode):
	                              SSB_Filter (31, 0.25, audioRate),
	                              usb_filter (21, 2500, audioRate),
	                              lsb_filter (21, -2500, 0, audioRate) {
	this	-> audioRate	= audioRate;
	this	-> Mode		= Mode;
	this	-> dc_alpha	= 0.98;
	this	-> m_w1		= 0;
}

	amHandler::~amHandler	(void) {
}

std::complex<float> amHandler::handle	(std::complex<float> s) {
float	res;
float	mag;
float	w0;

	switch (Mode) {
	   default:
	   case Mode_am:
	      mag	= abs (s);
	      break;

	   case Mode_lsb:
	      s		= lsb_filter. Pass (SSB_Filter. Pass (s));
	      mag	= real (s) + imag (s);
	      break;

	   case Mode_usb:
	      s		= usb_filter. Pass (SSB_Filter. Pass (s));
	      mag	= real (s) - imag (s);
	      break;
	}
//      HF filtering according to CuteSDR
	w0	= mag + dc_alpha * m_w1;
	res	= w0 - m_w1;
	m_w1	= w0;
	return std::complex<float> (res, res);
}


