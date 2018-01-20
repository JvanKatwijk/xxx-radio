#
/*
 *    Copyright (C) 2016, 2017
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the XXX-radio program
 *
 *    XXX-radio is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    XXX-radio is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with sdrplay-radio; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<unistd.h>
#include	<signal.h>
#include	<getopt.h>
#include	<atomic>
#include        <cstdio>
#include        <iostream>
#include	<cstring>
#include	<csignal>
#include	"samplerate.h"
#include	"radio-constants.h"
#include	"radio-processor.h"
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#define	name	"sdrplay-radio"
#elif	HAVE_AIRSPY
#include	"airspy-handler.h"
#define	name	"airspy-radio"
#endif

static
std::atomic<bool>       running;

void usage (void) {
	fprintf (stderr,
		"%s, a command line FM demodulator\n\n", name);
	fprintf (stderr,
		"Use:\t%s -f freq [-options] [filename]\n", name);
	fprintf (stderr,
		"\t-f frequency_to_tune_to [Hz]\n"
		"\t-F frequency_to_tune_to [kHz]\n"
		"\t[-s audio (default: 22050Hz)]\n"
	        "\t[-A antenna selector (only for rsp2)]\n"
		"\t[-g tuner_gain (default: 50)]\n"
	        "\t[-G autogain]\n"
		"\t[-p ppm_correction (default: 0)]\n"
		"\t[-O filename (a '-' dumps samples to stdout)\n"
		"\t (omitting the filename also uses stdout)\n\n"
		"\t[-d device_index (default: 0)]\n"
	        "\t[-Z enables lowpass filtering of the audiosignal]\n"
		"\t[-D enables de-emphasis (default: off)]\n"
	        "\t[-M Mode select, default \"fm-mono\"]\n"
	        "\t[ other modes \"fm-stero\", \"am\" \"raw\" \"usb\" \"lsb\"]\n"
		"\n"
		"\tProduces signed 16 bit ints, use Sox or aplay to hear them.\n"
	        "\tNote that fm-stereo produces pairs of signed 16 bit ints.\n"
		"\tsdrplay_radio ... - | play -t raw -r 22050 -es -b 16 -c 1 -V1 -\n"
		"\t                 | aplay -r 22050 -f S16_LE -t raw -c 1\n"
		"\t  -s 22.5k -     | multimon -t raw /dev/stdin\n"
	        "\t  -s 22050       | multimon-ng -a FLEX -t raw /dev/stdin\n");
	   
	exit(1);
}

#ifdef _WIN32
BOOL WINAPI
sighandler (int signum) {
	if (CTRL_C_EVENT == signum) {
	   fprintf(stderr, "Signal caught, exiting!\n");
	   running. store (false);
	   return TRUE;
	}
	return FALSE;
}
#else
static void signalHandler(int signum) {
	fprintf(stderr, "Signal caught, exiting!\n");
	running. store (false);
}
#endif

int16_t	getMode (char	*ss) {
std::string s = std::string (ss);

	if ((s == std::string ("FM-MONO")) || (s == std::string ("fm-mono")))
	   return Mode_fm_mono;
	if ((s == std::string ("FM-STEREO")) || (s == std::string ("fm-stereo")))
	   return Mode_fm_stereo;
	if ((s == std::string ("AM")) || (s == std::string ("am")))
	   return Mode_am;
	if ((s == std::string ("USB")) || (s == std::string ("usb")))
	   return Mode_usb;
	if ((s == std::string ("LSB")) || (s == std::string ("lsb")))
	   return Mode_lsb;
	if ((s == std::string ("RAW")) || (s == std::string ("RAW")))
	   return Mode_raw;
	return Mode_raw;
}

int	main (int argc, char **argv) {
deviceHandler *inputDevice;
int	opt;
// Default values
int16_t	deviceGain	= 50;
bool	autoGain	= false;
int32_t	inputRate;
int32_t	fmRate;
int32_t	audioRate	= 22050;
int16_t	ppmCorrection	= 0;
int16_t	Mode		= Mode_fm_mono;
bool	deemphasis	= false;
bool	filtering	= false;
FILE	*outFile	= stdout;
radioProcessor	*theWorker;
int32_t		tunedFrequency	= -1;
struct sigaction sigact;

	fprintf (stderr, "%s, Copyright Jan van Katwijk\n Lazy Chair Computing\n",
	                                                 name);
//	default
	std::setlocale (LC_ALL, "");
//
	while ((opt = getopt (argc, argv, "Df:p:F:O:g:Gs:M:Z")) != -1) {
	   switch (opt) {
	      case 'f':
	         tunedFrequency	= atoi (optarg);
	         break;

	      case 'F':
	         tunedFrequency	= atoi (optarg) * 1000;
	         break;

	      case 'g':
	         deviceGain	= atoi (optarg);
	         break;

	      case 'p':
	         ppmCorrection	= atoi (optarg);
	         break;

	      case 'O':
	         outFile = fopen (optarg, "w+b");
	         if (outFile == NULL) {
	            fprintf (stderr, "could not open file %s (fatal)\n",
	                                                optarg);
	            exit (21);
	         }
	         break;

	      case 's':
	         audioRate	= atoi (optarg);
	         break;

	      case 'M':
	         Mode		= getMode (optarg);
	         fprintf (stderr, "The Mode = %o\n", Mode);
	         break;

	      case 'Z':
	         filtering	= true;
	         break;

	      case 'D':
	         deemphasis	= true;
	         break;

	      case 'G':
	         autoGain	= true;
	         break;

	      default:
	         break;
	   }
	}

	if (tunedFrequency == -1)
	   usage ();		// will terminate
	fmRate		= 192000;
	inputRate	= 11 * fmRate;
//	The input device handler will generate an exception when
//	installation does not work

	try {
#ifdef	HAVE_SDRPLAY
	   inputDevice	= new sdrplayHandler (inputRate,
	                                      tunedFrequency,
	                                      ppmCorrection,
	                                      deviceGain,
	                                      0,
	                                      0,
	                                      autoGain);
#elif	HAVE_AIRSPY
	   inputDevice	= new airspyHandler (inputRate,
	                                     tunedFrequency,
	                                     ppmCorrection,
	                                     deviceGain);
#else
	   inputDevice	= new deviceHandler ();
#endif
	}
	catch (int e) {
	   cerr << "Installing device failed, fatal" << e << endl;
	   exit (3);
	}

	inputDevice	-> setVFOFrequency (tunedFrequency);

	fprintf (stderr, "parameters: frequency = %d, working rate = %d, sampling rate = %d\n", inputDevice -> getVFOFrequency (), fmRate, audioRate);
	signal(SIGINT, signalHandler);  

//	OK, it seems we have a device
//
	theWorker	= new radioProcessor (inputDevice,
	                                      inputRate,
	                                      fmRate,
	                                      audioRate,
	                                      Mode,
	                                      filtering,
	                                      deemphasis,
	                                      outFile);

	if (!inputDevice -> restartReader ()) {
	   fprintf (stderr, "device does not start, fatal\n");
	   exit (31);
	}

	theWorker	-> start ();
	
	running. store (true);
	while (running. load ()) 
	   sleep (1);
	fprintf (stderr, "got the message, stopping\n");
	inputDevice	-> stopReader ();
	theWorker	-> stop ();
	delete	theWorker;
	delete	inputDevice;
}
