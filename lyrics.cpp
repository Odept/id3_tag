#include "lyrics.h"

#include "common.h"

#include <cstring> // memcpy


struct __attribute__ ((__packed__)) Header_t
{
	union __attribute__ ((__packed__))
	{
		char	cId[11];
		uint	uId[2];
	};

	bool isValid() const
	{
		return (uId[0] == FOUR_CC('L','Y','R','I') &&
				uId[1] == FOUR_CC('C','S','B','E') &&
				cId[8] == 'G' && cId[9] == 'I' && cId[10] == 'N');
	}
};

struct __attribute__ ((__packed__)) Footer_t
{
	union __attribute__ ((__packed__))
	{
		char	cId[9];
		uint	uId[2];
	};

	bool isValid() const
	{
		return (uId[0] == FOUR_CC('L','Y','R','I') &&
				((uId[1] == FOUR_CC('C','S','E','N') && cId[8] == 'D') ||
				 (uId[1] == FOUR_CC('C','S','2','0') && cId[8] == '0')));
	}
};

// ============================================================================
CLyrics* CLyrics::gen(const uchar* f_pData, unsigned long long f_size, uint* f_puTagSize)
{
	ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
	uint size(f_size);

	// Check header
	const Header_t& h = *(const Header_t*)f_pData;
	if(size < sizeof(h) || !h.isValid())
		return NULL;
	size -= sizeof(h);

	// Search for the footer
	if(size < sizeof(Footer_t))
	{
		ERROR("Too small buffer for a lyrics tag footer");
		return NULL;
	}

	const uchar* p = f_pData + sizeof(Header_t);
	for(uint n = size - sizeof(Footer_t) + 1; n; n--, p++)
	{
		const Footer_t& f = *(const Footer_t*)p;
		if( !f.isValid() )
			continue;

		size = uint((const uchar*)(&f + 1) - f_pData);
		if(f_puTagSize)
			*f_puTagSize = size;
		return new CLyrics(f_pData, size);
	}

	ERROR("Lyrics footer not found");
	return NULL;
}


CLyrics::CLyrics(const uchar* f_data, uint f_size):
	m_data(f_size)
{
	memcpy(&m_data[0], f_data, f_size);
}

