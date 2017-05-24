#
/*
 *    Copyright (C) 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the sdrplay-fm software
 *
 *    sdrplay-fm is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    sdrplay-fm is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdrplay-fm; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<complex>
#include	"radio-processor.h"
#include	"fm-monohandler.h"
#include	"fm-stereohandler.h"
#include	"am-handler.h"
#include	"raw-handler.h"
#include	"device-handler.h"
//
//	The "radioProcessor" is the one that controls the job, samples
//	are read in at pretty high speed (inputRate), and (integer)
//	decimated to fmRate, the rate the work is being done
//	A second converter, from fmrate to audiorate,
//	is required to reach audiorate.
	radioProcessor::radioProcessor (deviceHandler	*inputDevice,
	                                int32_t		inputRate,
	                                int32_t		fmRate,
	                                int32_t		audioRate,
	                                int16_t		Mode,
	                                bool		filtering,
	                                bool		deemphasis,
	                                FILE		*output):
				          decimatingFilter (4 * inputRate / fmRate + 1,
	                                                    fmRate / 2,
	                                                    inputRate,
	                                                    inputRate / fmRate),
	                                  lowpassFilter (21,
	                                                 audioRate / 2,
	                                                 fmRate),
	                                  theConverter (fmRate,
	                                                audioRate,
	                                                4096) {
	running. store (false);
	this	-> inputDevice		= inputDevice;
	this	-> inputRate		= inputRate;
	this	-> fmRate		= fmRate;
	this	-> decimatingScale	= inputRate / fmRate;
	this	-> audioRate		= audioRate;
	this	-> Mode			= Mode;
	this	-> deemphasis		= deemphasis;
	this	-> output		= output;
//
//	we select the "handler", 
	switch (Mode) {
	   case Mode_fm_mono:
	      theHandler	= new fm_monoHandler (fmRate, deemphasis);
	      break;
	   case Mode_fm_stereo:
	      theHandler	= new fm_stereoHandler (fmRate, deemphasis);
	      break;
	   case Mode_am:
	   case Mode_usb:
	   case Mode_lsb:
	      theHandler	= new amHandler (audioRate, Mode);
	      break;
	   case Mode_raw:
	      theHandler	= new rawHandler (fmRate);
	      break;
	   default:
	      theHandler	= new fm_monoHandler (fmRate, deemphasis);
	      break;
	}
}

	radioProcessor::~radioProcessor (void) {
	if (running. load ()) {
	   running. store (false);
	   threadHandle. join ();
        }
	delete theHandler;
	fprintf (stderr, "fmprocessor is gestopt\n");
}

void	radioProcessor::stop	(void) {
	if (running. load ()) {
	   running. store (false);
	   threadHandle. join ();
        }
}
	

void    radioProcessor::start    (void) {
        running. store (true);
        threadHandle    = std::thread (&radioProcessor::run, this);
}

//	We have a separate thread for the processing, processing
//	is in itself a simple loop, reading samples, decimating,
//	processing/decimating filtering and sending it to output

#define	bufferSize	4096
void	radioProcessor::run (void) {
std::complex<float>	dataBuffer	[bufferSize];
std::complex<float>	passBuffer	[theConverter. getOutputsize ()];
int32_t		no_of_output;
int16_t		amount;
std::complex<float> v;
std::complex<float> res;

	try {
	   int16_t i;
	   std::complex<float> v;
	   while (running. load ()) {
	      while (running. load () &&
	                  (inputDevice -> Samples () < bufferSize)) 
	         usleep (100);	// should be enough

	      if (!running. load ())
	         throw (22);
//
//	This is what we get in, first thing: decimating
//	which - by the way - is a pretty resource consuming operation
	      amount = inputDevice -> getSamples (dataBuffer, bufferSize);
	      switch (Mode) {
	         default:
	         case Mode_fm_mono:
	            for (i = 0; i < amount; i ++) {
	               v	= dataBuffer [i];
	               if (!decimatingFilter. Pass (v, &v))
	                  continue;

	               res	= theHandler -> handle (v);
	               if (filtering)
	                  res = lowpassFilter. Pass (res);

	               if (theConverter.
	                        convert (res, passBuffer, &no_of_output)) {
	                  writeBuffertoFile (passBuffer,
	                                     no_of_output, false, output);
	               }
	            }	// end main loop 
	            break;

	         case Mode_fm_stereo:
	            for (i = 0; i < amount; i ++) {
	               v	= dataBuffer [i];
	               if (!decimatingFilter. Pass (v, &v))
	                  continue;

	               res	= theHandler -> handle (v);
	               if (filtering)
	                  res = lowpassFilter. Pass (res);

	               if (theConverter.
	                        convert (res, passBuffer, &no_of_output)) {
	                  writeBuffertoFile (passBuffer,
	                                     no_of_output,  true, output);
	               }
	            } // end main loop
	            break;

	         case Mode_raw:
	            for (i = 0; i < amount; i ++) {
	               v	= dataBuffer [i];
	               if (!decimatingFilter. Pass (v, &v))
	                  continue;

	               if (filtering)
	                  res = lowpassFilter. Pass (v);
	               if (theConverter.
	                     convert (res, passBuffer, &no_of_output)) {
	               writeBuffertoFile (passBuffer,
	                                  no_of_output,  false, output);
	               }
	            }
	            break;

	         case Mode_am:
	         case Mode_usb:
	         case Mode_lsb:
//
//	For these modes, we first decimate
	            for (i = 0; i < amount; i ++) {
	               v	= dataBuffer [i];
	               if (!decimatingFilter. Pass (v, &v))
	                  continue;

	               if (filtering)
	                  v = lowpassFilter. Pass (v);
	               if (theConverter.
	                         convert (v, passBuffer, &no_of_output)) {
	                  for (i = 0; i < no_of_output; i ++)
	                     passBuffer [i] = theHandler ->
	                                           handle (passBuffer [i]);
	                  writeBuffertoFile (passBuffer,
	                                      no_of_output, false, output);
	                }
	            }
	      }

	   } // end "running" loop
	   fprintf (stderr, "normal termination\n");
	}
	catch (int e) {;}
}
//
//	While we work with floats, output is in short int's
void	radioProcessor::writeBuffertoFile (std::complex<float>	*buffer,
	                                   int16_t		amount,
	                                   bool			isStereo,
	                                   FILE		*outfile) {
int16_t	temp [2 * amount];
int16_t	i;
	if (isStereo) {
	   for (i = 0; i < amount; i ++) {
	      temp [2 * i]	= real (buffer [i]) * 32768;
	      temp [2 * i + 1]	= imag (buffer [i]) * 32768;
	   }

	   fwrite (temp, 4, amount, outfile);
	}
	else {
	   for (i = 0; i < amount; i ++) {
	      temp [i]	= real (buffer [i]) * 32768;
	   }
	   fwrite (temp, 2, amount, outfile);
	}
}

