#include "utf8.h"

#include "common.h"

#include <vector>
#include <cstring> // memcpy

#include <iconv.h>


std::string UTF8::fromU16(const char* f_data, size_t f_size, const char* f_format)
{
	iconv_t cd = iconv_open("UTF-8", f_format);
	ASSERT(cd != iconv_t(-1));

	std::vector<char> bufIn(f_size);
	char* pIn = &bufIn[0];
	memcpy(pIn, f_data, f_size);
	size_t sizeIn = bufIn.size();;

	std::vector<char> bufOut((f_size + 2/*UTF-16(NULL)*/) * 2 /*chars * 4 (max for UTF-8)*/);
	char* pOut = &bufOut[0];
	size_t sizeOut = bufOut.size();

	size_t nNonReversible = iconv(cd, &pIn, &sizeIn, &pOut, &sizeOut);
	if(nNonReversible == size_t(-1))
	{
		switch(errno)
		{
			case EILSEQ:    ASSERT(!"Invalid multi-byte sequence");
			case EINVAL:    ASSERT(!"Incomplete mutli-byte sequence");
			case E2BIG:     ASSERT(!"Output buffer is too small");
			default:        ASSERT(!"Convesrion error");
		}
	}
	ASSERT(nNonReversible == 0);
	*pOut = 0;

	iconv_close(cd);

	return std::string(&bufOut[0]);
}
