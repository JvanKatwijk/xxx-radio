
#include	<thread>
#include	<atomic>
#include	<stdio.h>
#include	"radio-constants.h"
#include	"fir-filters.h"
#include	"radio-handler.h"
#include	"newconverter.h"
class		deviceHandler;

class radioProcessor {
public:
		radioProcessor (deviceHandler	*vi,
                                int32_t		inputRate,
                                int32_t		fmRate,
                                int32_t		audioRate,
	                        int16_t		Mode,
	                        bool		filtering,
	                        bool		deemphasis,
                                FILE		*output);
		~radioProcessor (void);
void		start		(void);
void		stop		(void);
private:
	void	run		(void);
	std::atomic<bool> 	running;
	deviceHandler	*inputDevice;
	radioHandler	*theHandler;
	int32_t		inputRate;
	int32_t		fmRate;
	int32_t		audioRate;
	int16_t		Mode;
	bool		filtering;
	bool		deemphasis;
	FILE		*output;
	int32_t		decimatingScale;
	decimatingFIR	decimatingFilter;
	lowpassFIR	lowpassFilter;
	std::thread	threadHandle;
	newConverter	theConverter;
	void		writeBuffertoFile (std::complex<float> *buffer,
                                           int16_t	amount,
	                                   bool,
                                           FILE		*outfile);
};

