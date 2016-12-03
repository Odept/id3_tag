#include "tag.h"

#include "common.h"

#include <cstring> // memcpy
#include <vector>


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
	char	Key[];
	//char	Null;
	//char	Value[];
};

// ====================================
class CAPE : public Tag::IAPE
{
public:
	CAPE(const uchar* f_data, size_t f_size): m_tag(f_size)
	{
		memcpy(&m_tag[0], f_data, f_size);
	}
	CAPE() = delete;

	void serialize(std::vector<unsigned char>& f_outStream) final override
	{
		f_outStream.insert(f_outStream.end(), m_tag.begin(), m_tag.end());
	}

private:
	std::vector<uchar> m_tag;
};

// ====================================
namespace Tag
{
	IAPE::TagSize IAPE::getSize(const unsigned char* f_data, size_t f_size)
	{
		ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
		auto size = f_size;

		// Check header
		auto& h = *reinterpret_cast<const Header_t*>(f_data);
		if(size < sizeof(h) || !h.isValidHeader())
			return 0;
		size -= sizeof(h);
		ASSERT(h.Size == sizeof(h));

		// Parse items
		auto pData = f_data + sizeof(Header_t);
		for(uint i = 0; i < h.Items; i++)
		{
			auto& ii = *reinterpret_cast<const Item*>(pData);
			if(size < sizeof(ii))
				return 0;
			size -= sizeof(ii);

			for(pData = reinterpret_cast<const uchar*>(ii.Key); *pData && size; pData++, size--) {}
			if(!size)
				return 0;

			pData += 1/*NULL*/ + ii.Size;
			if(size < ii.Size)
				return 0;
			size -= ii.Size;
		}

		// Check footer
		auto& f = *reinterpret_cast<const Header_t*>(pData);
		if(size < sizeof(f) || !f.isValidFooter())
			return 0;
		size -= sizeof(f);
		ASSERT(f.Size == (const uchar*)(&f + 1) - (const uchar*)(&h + 1));

		return (reinterpret_cast<const uchar*>(&f + 1) - reinterpret_cast<const uchar*>(&h));
	}

	std::shared_ptr<IAPE> IAPE::create(const unsigned char* f_data, TagSize f_size)
	{
		return std::make_shared<CAPE>(f_data, f_size);
	}
}

