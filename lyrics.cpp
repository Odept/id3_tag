#include "tag.h"

#include "common.h"

#include <cstring> // memcpy
#include <vector>


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

// ====================================
class CLyrics : public Tag::ILyrics
{
public:
	CLyrics(const uchar* f_data, size_t f_size): m_tag(f_size)
	{
		memcpy(&m_tag[0], f_data, f_size);
	}
	CLyrics() = delete;

	void serialize(std::vector<unsigned char>& f_outStream) final override
	{
		f_outStream.insert(f_outStream.end(), m_tag.begin(), m_tag.end());
	}

	size_t getSize() const final override { return m_tag.size(); }

private:
	std::vector<uchar> m_tag;
};

// ====================================
namespace Tag
{
	size_t ILyrics::getSize(const unsigned char* f_data, size_t f_size)
	{
		ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
		auto size = f_size;

		// Check header
		auto& h = *reinterpret_cast<const Header_t*>(f_data);
		if(size < sizeof(h) || !h.isValid())
			return 0;
		size -= sizeof(h);

		// Search for the footer
		if(size < sizeof(Footer_t))
			return 0;

		auto p = f_data + sizeof(Header_t);
		for(auto n = size - sizeof(Footer_t) + 1; n; --n, ++p)
		{
			auto& f = *reinterpret_cast<const Footer_t*>(p);
			if( f.isValid() )
				return (reinterpret_cast<const uchar*>(&f + 1) - f_data);
		}

		// Lyrics tag footer not found
		return 0;
	}

	std::shared_ptr<ILyrics> ILyrics::create(const unsigned char* f_data, size_t f_size)
	{
		return std::make_shared<CLyrics>(f_data, f_size);
	}
}

