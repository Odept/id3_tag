#include "ape.h"

#include "common.h"

#include <cstring> // memcpy


#define FOUR_CC(A, B, C, D)	 \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))


struct __attribute__ ((__packed__)) Flags_t
{
	union
	{
		struct
		{
			uint ReadOnly	:  1;
			uint Type		:  2;
			uint Undefined	: 26;
			uint IsHeader	:  1;
			uint NoFooter	:  1;
			uint HasHeader	:  1;
		};
		uint uCell;
	};
};

struct __attribute__ ((__packed__)) Header_t
{
	union
	{
		char	cId[8];
		uint	uId[2];
	};
	uint	Version;
	uint	Size;
	uint	Items;
	Flags_t	Flags;
	uint	Reserved[2];

	bool isValidHeader() const { return (isValidId() &&  Flags.HasHeader &&  Flags.IsHeader && Version >= 2000); }
	bool isValidFooter() const { return (isValidId() && !Flags.NoFooter  && !Flags.IsHeader); }

private:
	bool isValidId() const
	{
		return (uId[0] == FOUR_CC('A','P','E','T') &&
				uId[1] == FOUR_CC('A','G','E','X') &&
				Flags.Undefined == 0);
	}
};

struct __attribute__ ((__packed__)) Item
{
	uint	Size;
	Flags_t	Flags;
	char	Key[1];
	//char	Null;
	//char	Value[1];
};

// ============================================================================
CAPE* CAPE::gen(const uchar* f_pData, unsigned long long f_size, uint* f_puTagSize)
{
	ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
	uint size(f_size);

	// Check header
	const Header_t& h = *(const Header_t*)f_pData;
	if(size < sizeof(h) || !h.isValidHeader())
		return NULL;
	size -= sizeof(h);
	ASSERT(h.Size == sizeof(h));

	// Parse items
	const uchar* pData = f_pData + sizeof(Header_t);
	for(uint i = 0; i < h.Items; i++)
	{
		const Item& ii = *(const Item*)pData;
		if(size < sizeof(ii))
			return NULL;
		size -= sizeof(ii);

		for(pData = (const uchar*)ii.Key; *pData && size; pData++, size--) {}
		if(!size)
			return NULL;

		pData += 1/*NULL*/ + ii.Size;
		if(size < ii.Size)
			return NULL;
		size -= ii.Size;
	}

	// Check footer
	const Header_t& f = *(const Header_t*)pData;
	if(size < sizeof(f) || !f.isValidFooter())
		return NULL;
	size -= sizeof(f);
	ASSERT(f.Size == (const uchar*)(&f + 1) - (const uchar*)(&h + 1));

	size = uint((const uchar*)(&f + 1) - (const uchar*)&h);
	if(f_puTagSize)
		*f_puTagSize = size;
	return new CAPE(f_pData, size);
}

bool CAPE::isValidHeader(const uchar* f_pData, unsigned long long f_size)
{
	const Header_t& h = *(const Header_t*)f_pData;
	if(f_size < sizeof(h))
		return false;
	return (h.isValidHeader() || h.isValidFooter());
}


CAPE::CAPE(const uchar* f_data, uint f_size):
	m_data(f_size)
{
	memcpy(&m_data[0], f_data, f_size);
}

