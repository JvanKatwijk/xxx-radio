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
 *	recoding and taking parts for the airspyRadio interface
 *	for the DAB library
 *	jan van Katwijk
 *	Lazy Chair Computing
 */
#ifndef __AIRSPY_HANDLER__
#define	__AIRSPY_HANDLER__

#include	"ringbuffer.h"
#include	"device-handler.h"
#include	<complex>
#include	<vector>
#include	<atomic>

#ifdef  __MINGW32__
#include        "windows.h"
#endif

#ifndef	__MINGW32__
#include	"libairspy/airspy.h"
#else
#include	"airspy.h"
#endif


class airspyHandler: public deviceHandler {
public:
			airspyHandler		(int32_t,
	                                         int32_t, int16_t, int16_t);
			~airspyHandler		(void);
	void		setVFOFrequency		(int32_t);
	int32_t		getVFOFrequency		(void);
	bool		restartReader		(void);
	void		stopReader		(void);
	int32_t		getSamples		(std::complex<float> *v,
	                                                int32_t size);
	int32_t		Samples			(void);
	void		resetBuffer		(void);
	int16_t		bitDepth		(void);
	void		setGain			(int32_t);
private:
//
	int32_t		outputRate;
	int32_t		frequency;
	int16_t		ppmCorrection;
	int16_t		gain;
	std::atomic<bool>	running;
const	char*		board_id_name (void);
	struct airspy_device* device;
	uint64_t 	serialNumber;
	char		serial[128];
	int32_t		selectedRate;
	int16_t		convBufferSize;
	int16_t		convIndex;
	std::vector<complex<float> > convBuffer;
	std::vector<int16_t>	mapTable_int;
	std::vector<float>	mapTable_float;
	RingBuffer<std::complex<float>> *theBuffer;
	int32_t		inputRate;
static
	int		callback (airspy_transfer_t *);
	int		data_available (void *buf, int buf_size);
const	char *		getSerial (void);
};

#endif
