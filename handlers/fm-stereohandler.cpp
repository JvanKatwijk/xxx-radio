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
#include	"fm-stereohandler.h"
#include	"fm-demodulator.h"
#include	"pllC.h"

#define	RDS_DECIMATOR		8
#define	PILOT_FILTERSIZE	35
#define	RDSLOWPASS_SIZE		11
#define	HILBERT_SIZE		7
#define	FFT_SIZE		512
#define	PILOT_WIDTH		100
#define	LEVEL_SIZE		512
#define	LEVEL_FREQ		3
#define PILOT_FREQUENCY         19000
#define LRDIFF_FREQUENCY        (2 * PILOT_FREQUENCY)
#define RDS_FREQUENCY           (3 * PILOT_FREQUENCY)
#define OMEGA_DEMOD             (2 * M_PI / fmRate)
#define OMEGA_PILOT             PILOT_FREQUENCY * OMEGA_DEMOD
#define OMEGA_RDS               RDS_FREQUENCY * OMEGA_DEMOD
#define PILOT_DELAY             (FFT_SIZE - PILOT_FILTERSIZE) * OMEGA_PILOT


//
#define	AUDIO_FREQ_DEV_PROPORTION 0.85f

	fm_stereoHandler::fm_stereoHandler (int32_t	fmRate,
	                                    bool	deemphasis):
	                                    pilotBandFilter (FFT_SIZE,
	                                                     PILOT_FILTERSIZE),
	                                    lrdiffFilter (5, 15000, fmRate),
	                                    lrplusFilter (5, 15000, fmRate),
	                                    theHilbertFilter (HILBERT_SIZE,
	                                                      fmRate / 2 - 1,
	                                                      fmRate) {
int	i;
	this	-> fmRate		= fmRate;
	this	-> deemphasis		= deemphasis;
        float	F_G	= 0.65 * fmRate / 2; // highest freq in message
        float	Delta_F = 0.90 * fmRate / 2;
        float	B_FM    = 2 * (Delta_F + F_G);
        K_FM		 = 0.1 * M_PI / fmRate * B_FM;

	Table		= new std::complex<float> [fmRate];
	for (i = 0; i < fmRate; i ++)
           Table [i] = std::complex<float> (cos (2 * M_PI * i / fmRate),
                                            sin (2 * M_PI * i / fmRate));
        pilotBandFilter. setBand (PILOT_FREQUENCY - PILOT_WIDTH / 2,
                                  PILOT_FREQUENCY + PILOT_WIDTH / 2,
                                  fmRate);

        pilotRecover            = new pllC (fmRate,
                                            PILOT_FREQUENCY,
                                            PILOT_FREQUENCY - 100,
                                            PILOT_FREQUENCY + 100,
                                            10,
                                            Table);

	my_demodulator	= new demodulator (fmRate, K_FM);

	audiogainAverage	= 0;
	peakLevelcnt		= 0;
	Tau			= 1000000.0 / 50;
	alpha			= 1.0 / (float (fmRate) / Tau + 1.0);
	xkm1			= 0;
	max_freq_deviation	= 0.95 * (0.5 * fmRate);
	norm_freq_deviation	= 0.45 * max_freq_deviation;
}

	fm_stereoHandler::~fm_stereoHandler (void) {
}

//	   Rate here is fmRate, 192k
std::complex<float> fm_stereoHandler:: handle (std::complex<float> v) {
	float LRPlus = my_demodulator ->  demodulate (v);

//	for the audiogain correction
	if (abs (LRPlus) * 50 > peakLevel)
	   peakLevel = abs (LRPlus) * 50;
	if (++ peakLevelcnt >= fmRate / 4) {
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

	std::complex<float>pilot = theHilbertFilter. Pass (LRPlus, LRPlus);
	std::complex<float> LRDiff = pilot;
	pilot                   = pilotBandFilter. Pass (pilot);
        pilotRecover            -> do_pll (pilot);
        float currentPilotPhase       = pilotRecover -> getNco ();

//      for the actual phase we should take into account
//      the delay caused by the FIR bandfilter
        currentPilotPhase       += PILOT_DELAY;
//
//      shift the LRDiff signal down to baseband        38Khz
        int32_t thePhase        = currentPilotPhase / (2 * M_PI) * fmRate;
        LRDiff  *= conj (Table [(2 * thePhase) % fmRate]);

//      get rid of junk
        LRDiff                  = lrdiffFilter. Pass (LRDiff);
//      .... and for the LplusR as well
        LRPlus                  = lrplusFilter. Pass (LRPlus);

	float	left		= real (LRPlus + LRDiff);
	float	right		= real (- LRDiff - LRPlus);
//	if we have deemphasis
	if (deemphasis) {
	   left	= xkm1	= (left - xkm1) * alpha + xkm1;
	   right	= ykm1	= (right - ykm1) * alpha + ykm1;
	}
	left	*= audioGain;
	right	*= audioGain;
	return std::complex<float>(left, right);
}

