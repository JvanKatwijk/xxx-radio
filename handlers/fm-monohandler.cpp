#
/*
 *    Copyright (C) 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the sdrplay-radio software
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
#include	"fm-monohandler.h"
#include	"fm-demodulator.h"

#define	AUDIO_FREQ_DEV_PROPORTION 0.85f

	fm_monoHandler::fm_monoHandler (int32_t	fmRate,
	                                bool	deemphasis) {
	this	-> fmRate		= fmRate;
	this	-> deemphasis		= deemphasis;
        float	F_G	= 0.65 * fmRate / 2; // highest freq in message
        float	Delta_F = 0.90 * fmRate / 2;
        float	B_FM    = 2 * (Delta_F + F_G);
        K_FM		 = 0.1 * M_PI / fmRate * B_FM;
	my_demodulator	= new demodulator (fmRate, K_FM);

	audiogainAverage	= 0;
	peakLevelcnt		= 0;
	Tau			= 1000000.0 / 50;
	alpha			= 1.0 / (float (fmRate) / Tau + 1.0);
	xkm1			= 0;
	max_freq_deviation	= 0.95 * (0.5 * fmRate);
	norm_freq_deviation	= 0.55 * max_freq_deviation;
}

	fm_monoHandler::~fm_monoHandler (void) {
}

//	   Rate here is fmRate, 192k
std::complex<float> fm_monoHandler:: handle (std::complex<float> v) {
	float res = my_demodulator ->  demodulate (v);

//	for the audiogain correction
	if (abs (v) * 50 > peakLevel)
	   peakLevel = abs (v) * 50;
	if (++ peakLevelcnt >= fmRate / 2) {
	   float ratio	= max_freq_deviation / norm_freq_deviation;
	   if (peakLevel > 0)
	      this -> audioGain	= 
	                     (ratio / peakLevel) / AUDIO_FREQ_DEV_PROPORTION;
	   if (audioGain <= 0.05)
	      audioGain = 0.05;
	   audioGain	= 0.95 * audiogainAverage + 0.05 * audioGain;
	   audiogainAverage = audioGain;
	   peakLevelcnt	= 0;
	   peakLevel	= -100;
	}

//	if we have deemphasis
	if (deemphasis)
	   res	= xkm1	= (res - xkm1) * alpha + xkm1;
	res	*= audioGain;
	return std::complex<float>(res, res);
}

