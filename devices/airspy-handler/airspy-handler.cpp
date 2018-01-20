#
/**
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 *	recoding, taking parts and extending for the airspyHandler interface
 *	for the SDR-J-DAB receiver.
 *	jan van Katwijk
 *	Lazy Chair Computing
 */

#include "airspy-handler.h"

static
const	int	EXTIO_NS	=  8192;
static
const	int	EXTIO_BASE_TYPE_SIZE = sizeof (float);

static
std::complex<float> cmul (std::complex<float> x, float y) {
	return std::complex<float> (real (x) * y, imag (x) * y);
}

	airspyHandler::airspyHandler (int32_t	rate,
	                              int32_t	frequency,
	                              int16_t	ppmCorrection,
	                              int16_t	theGain) {
int	result, i;
int	distance	= 10000000;
std::vector <uint32_t> sampleRates;
uint32_t samplerateCount;

	this	-> outputRate	= rate;
	this	-> frequency	= frequency;
	this	-> ppmCorrection = ppmCorrection;
	this	-> gain		= theGain * 21 / 100;
//
	device			= 0;
	serialNumber		= 0;
	theBuffer		= NULL;
	strcpy (serial,"");
	result = airspy_init ();
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_init () failed: %s (%d)\n",
	             airspy_error_name((airspy_error)result), result);
	   throw (42);
	}

	fprintf (stderr, "airspy init is geslaagd\n");
	result = airspy_open (&device);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airpsy_open () failed: %s (%d)\n",
	             airspy_error_name ((airspy_error)result), result);
	   throw (43);
	}

	(void) airspy_set_sample_type (device, AIRSPY_SAMPLE_INT16_IQ);
	(void) airspy_get_samplerates (device, &samplerateCount, 0);
	sampleRates. resize (samplerateCount);
	airspy_get_samplerates (device, sampleRates. data (), samplerateCount);

	selectedRate	= 0;
	for (i = 0; i < (int)samplerateCount; i ++) {
	   if (abs ((int)(sampleRates [i]) - outputRate) < distance) {
	      distance	= abs ((int)(sampleRates [i]) - outputRate);
	      selectedRate = sampleRates [i];
	   }
	}

	if (selectedRate == 0) {
	   fprintf (stderr, "Sorry. cannot help you\n");
	   throw (44);
	}
	else
	   fprintf (stderr, "samplerate = %d, will be converted to %d\n",
	                        selectedRate, outputRate);

	result = airspy_set_samplerate (device, selectedRate);
	if (result != AIRSPY_SUCCESS) {
           printf ("airspy_set_samplerate() failed: %s (%d)\n",
	             airspy_error_name ((enum airspy_error)result), result);
	   throw (45);
	}

//	The sizes of the mapTable and the convTable are
//	predefined and follow from the input and output rate
//	(selectedRate / 1000) vs (outputRate / 1000)
	convBufferSize		= selectedRate / 1000;
	mapTable_int. resize (outputRate / 1000);
	mapTable_float. resize (outputRate / 1000);
	for (i = 0; i < outputRate / 1000; i ++) {
	   float inVal	= float (selectedRate / 1000);
	   mapTable_int [i] =  int16_t (floor (i * (inVal / (outputRate / 1000.0))));
	   mapTable_float [i] = i * (inVal / (outputRate / 1000.0)) - mapTable_int [i];
	}

	convIndex	= 0;
	convBuffer. resize (convBufferSize + 1);

	theBuffer	= new RingBuffer<std::complex<float>> (512 * 1024);
	running. store (false);
//
//	Here we set the gain and frequency

	if (airspy_set_freq (device, frequency) != AIRSPY_SUCCESS)
	   fprintf (stderr, "failing setting frequency\n");
	int16_t tmp	= (theGain * 21) / 100;
	(void) airspy_set_sensitivity_gain (device, tmp);
}

	airspyHandler::~airspyHandler (void) {
	if (device) {
	   int result = airspy_stop_rx (device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("airspy_stop_rx () failed: %s (%d)\n",
	             airspy_error_name ((airspy_error)result), result);
	   }

//	   if (rf_bias)
//	      set_rf_bias ();
	   result = airspy_close (device);
	   if (result != AIRSPY_SUCCESS) {
	      printf ("airspy_close () failed: %s (%d)\n",
	             airspy_error_name ((airspy_error)result), result);
	   }
	}
	airspy_exit ();
	if (theBuffer != NULL)
	   delete theBuffer;
}

void	airspyHandler::setVFOFrequency (int32_t nf) {
int result = airspy_set_freq (device, frequency = nf);

	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_freq() failed: %s (%d)\n",
	            airspy_error_name ((airspy_error)result), result);
	}
}

int32_t	airspyHandler::getVFOFrequency (void) {
	return frequency;
}

bool	airspyHandler::restartReader	(void) {
int	result;
int32_t	bufSize	= EXTIO_NS * EXTIO_BASE_TYPE_SIZE * 2;

	if (running . load ())
	   return true;

	theBuffer	-> FlushRingBuffer ();
	result = airspy_set_sample_type (device, AIRSPY_SAMPLE_INT16_IQ);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_set_sample_type () failed: %s (%d)\n",
	            airspy_error_name ((airspy_error)result), result);
	   return false;
	}

	result = airspy_set_sensitivity_gain (device, gain);
	result = airspy_start_rx (device,
	            (airspy_sample_block_cb_fn)callback, this);
	if (result != AIRSPY_SUCCESS) {
	   printf ("airspy_start_rx () failed: %s (%d)\n",
	         airspy_error_name ((airspy_error)result), result);
	   return false;
	}
//
//	finally:
	running. store (airspy_is_streaming (device) == AIRSPY_TRUE) ;
	return running. load ();
}

void	airspyHandler::stopReader (void) {
int result;

	if (!running. load ())
	   return;

	airspy_stop_rx (device);
	if (result != AIRSPY_SUCCESS ) 
	   printf ("airspy_stop_rx () failed: %s (%d)\n",
	          airspy_error_name ((airspy_error)result), result);
	running. store (false);
}
//
int airspyHandler::callback (airspy_transfer* transfer) {
airspyHandler *p;

	if (!transfer)
	   return 0;		// should not happen

	uint32_t bytes_to_write = transfer -> sample_count * sizeof (int16_t) * 2; 
	p = static_cast<airspyHandler *> (transfer -> ctx);

//	we use AIRSPY_SAMPLE_INT16:
	uint8_t *pt_rx_buffer   = (uint8_t *)transfer -> samples;
	p -> data_available (pt_rx_buffer, bytes_to_write);
	return 0;
}

//	called from AIRSPY data callback
//	The buffer received from hardware contains
//	16-bit short int IQ samples (4 bytes per sample)
//
//	We do the rate conversion here,
//	read in groups of outputRate / 1000 samples
//	and transform them into groups of 2 * 512 samples
int 	airspyHandler::data_available (void *buf, int buf_size) {	
int16_t	*sbuf	= (int16_t *)buf;
int nSamples	= buf_size / (sizeof (int16_t) * 2);
std::complex<float> temp [outputRate / 1000];
int32_t  i, j;

	for (i = 0; i < nSamples; i ++) {
	   convBuffer [convIndex ++] =
	               std::complex<float> (sbuf [2 * i] / (float)2048,
	                                    sbuf [2 * i + 1] / (float)2048);
	   if (convIndex > convBufferSize) {
	      for (j = 0; j < outputRate / 1000; j ++) {
	         int16_t  inpBase	= mapTable_int [j];
	         float    inpRatio	= mapTable_float [j];
	         temp [j]	= cmul (convBuffer [inpBase + 1], inpRatio) + 
	                          cmul (convBuffer [inpBase], 1 - inpRatio);
	      }

	      theBuffer	-> putDataIntoBuffer (temp, outputRate / 1000);
//
//	shift the sample at the end to the beginning, it is needed
//	as the starting sample for the next time
	      convBuffer [0] = convBuffer [convBufferSize];
	      convIndex = 1;
	   }
	}
	return 0;
}
//
const char *airspyHandler::getSerial (void) {
airspy_read_partid_serialno_t read_partid_serialno;
int result = airspy_board_partid_serialno_read (device,
	                                          &read_partid_serialno);
	if (result != AIRSPY_SUCCESS) {
	   printf ("failed: %s (%d)\n",
	         airspy_error_name ((airspy_error)result), result);
	   return "UNKNOWN";
	} else {
	   snprintf (serial, sizeof(serial), "%08X%08X", 
	             read_partid_serialno. serial_no [2],
	             read_partid_serialno. serial_no [3]);
	}

	return serial;
}
//
void	airspyHandler::resetBuffer (void) {
	theBuffer	-> FlushRingBuffer ();
}

int16_t	airspyHandler::bitDepth (void) {
	return 12;
}

int32_t	airspyHandler::getSamples (std::complex<float> *v, int32_t size) {
	return theBuffer	-> getDataFromBuffer (v, size);
}

int32_t	airspyHandler::Samples	(void) {
	return theBuffer	-> GetRingBufferReadAvailable ();
}
//

const char* airspyHandler::board_id_name (void) {
uint8_t bid;

	if (airspy_board_id_read (device, &bid) == AIRSPY_SUCCESS)
	   return airspy_board_id_name ((airspy_board_id)bid);
	else
	   return "UNKNOWN";
}
//
void	airspyHandler::setGain		(int value) {
	gain	= value * 21 / 100;
	(void) airspy_set_sensitivity_gain (device, gain);
}

