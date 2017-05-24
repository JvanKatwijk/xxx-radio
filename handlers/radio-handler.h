#
#ifndef	__RADIO_HANDLER__
#define	__RADIO_HANDLER__

#include	"radio-constants.h"

class radioHandler {
public:
		radioHandler	(void);
	virtual	~radioHandler	(void);
virtual std::complex<float> handle (std::complex<float>);
};
#endif
