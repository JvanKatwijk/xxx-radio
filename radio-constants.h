#
/*
 *    Copyright (C)  2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of sdrplay-radio
 *
 *    sdrplay-radio is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    sdrplay-radio is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdrplay-radio; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#
#ifndef __RADIO_CONSTANTS__
#define	__RADIO_CONSTANTS__

#include	<math.h>
#include	<complex>
#include	<stdint.h>
#include	<limits>
#include	<stdlib.h>
#include	<unistd.h>

#ifdef __MINGW32__
#include	"windows.h"
#else
#include	"dlfcn.h"
typedef	void	*HINSTANCE;
#endif
//
using namespace std;

#define	Mode_fm_mono	0100
#define	Mode_fm_stereo	0101
#define	Mode_am		0102
#define	Mode_lsb	0103
#define	Mode_usb	0104
#define	Mode_raw	0105

#define	Hz(x)		(x)
#define	Khz(x)		(x * 1000)
#define	KHz(x)		(x * 1000)
#define	Mhz(x)		(Khz (x) * 1000)
#define	MHz(x)		(KHz (x) * 1000)
/*
 */
#define	MINIMUM(x, y)	((x) < (y) ? x : y)
#define	MAXIMUM(x, y)	((x) > (y) ? x : y)

//	common functions
static inline
float	jan_abs (complex<float> z) {
float	re	= real (z);
float	im	= imag (z);
	if (re < 0) re = - re;
	if (im < 0) im = - im;
	return re + im;
}
static inline
bool	isIndeterminate (float x) {
	return x != x;
}

static inline
bool	isInfinite (double x) {
	return x == numeric_limits<float>::infinity ();
}
//
static inline
std::complex<float> cmul (std::complex<float> x, float y) {
	return std::complex<float> (real (x) * y, imag (x) * y);
}

static inline
std::complex<float> cdiv (std::complex<float> x, float y) {
	return std::complex<float> (real (x) / y, imag (x) / y);
}

static inline
float	get_db (float x, int32_t y) {
	return 20 * log10 ((x + 1) / (float)(y));
}
//
static	inline
float	PI_Constrain (float val) {
	if (0 <= val && val < 2 * M_PI)
	   return val;
	if (val >= 2 * M_PI)
	   return fmod (val, 2 * M_PI);
//	apparently val < 0
	if (val > - 2 * M_PI)
	   return val + 2 * M_PI;
	return 2 * M_PI - fmod (- val, 2 * M_PI);
}
#endif

