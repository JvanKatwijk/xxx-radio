#
#ifndef	__RAW_PROCESSOR__
#define	__RAW_PROCESSOR__
#include	<stdio.h>
#include	"radio-constants.h"
#include	"radio-handler.h"

class rawHandler: public radioHandler {
public:
		rawHandler	(int32_t	fmRate);
		~rawHandler	(void);
std::complex<float> handle	(std::complex<float>);
private:
	int32_t		fmRate;
};
#endif
