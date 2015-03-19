#include "id3v2.h"

#include "common.h"
#include "utf8.h"
#include "genre.h"

// Basic Routines
bool		CID3v2::isValid()		const { return m_valid;   }

uint		CID3v2::getVersion()	const { return m_version; }

const std::string&	CID3v2::getTrack()			const { return m_track;		}
const std::string&	CID3v2::getDisk()			const { return m_disk;		}
const std::string&	CID3v2::getTitle()			const { return m_title;		}
const std::string&	CID3v2::getArtist()			const { return m_artist;	}
const std::string&	CID3v2::getAlbum()			const { return m_album;		}
const std::string&	CID3v2::getYear()			const { return m_year;		}

bool CID3v2::isExtendedGenre() const
{
	return m_genre.get() ? m_genre->isExtended() : false;
}
const std::string& CID3v2::getGenre() const
{
	if(!m_genre.get())
	{
		static std::string empty("");
		return empty;
	}
	if(m_genre->isExtended())
	{
		static std::string genre( CGenre::get(m_genre->getIndex()) );
		return genre;
	}
	else
		return m_genre->str();
}
const std::string& CID3v2::getGenreEx() const
{
	static std::string empty("");
	return (m_genre.get() && m_genre->isExtended()) ? m_genre->str() : empty;
}
int CID3v2::getGenreIndex() const
{
	return m_genre.get() ? m_genre->getIndex() : -1;
}

// Complex Routines
CID3v2::CID3v2(const std::vector<uchar>& f_data):
	m_valid(false),
	m_version(0)
{
	const char* pData = (const char*)&f_data[0];

	// Check for 'ID3'
	static const uint HeaderSize = 10;
	if((f_data.size() < HeaderSize) ||
		pData[0] != 'I' || pData[1] != 'D' || pData[2] != '3')
	{
		return;
	}
	pData += 3;

	// Version
	uint ver = pData[0];
	uint rev = pData[1];

	if(ver == 0xFF || rev == 0xFF)
		return;
	ASSERT(ver == 3);
	m_version = (ver << 8) | rev;
	pData += 2;

	// Flags
	uint flags = *pData++;

	// ID3v2
	if(flags & 0x80)
	{
		// Unsynchronisation
		ASSERT(!"Unsynchronisation");
		flags &= ~0x80;
	}
	if(flags && ver == 0)
		return;

	// ID3v2.3
	if(flags & 0x40)
	{
		// Extended header
		ASSERT(!"Extended header");
		flags &= ~0x40;
	}
	if(flags & 0x20)
	{
		// Experimental indicator
		ASSERT(!"Experimental indicator");
		flags &= ~0x20;
	}
	if(flags && ver <= 3)
		return;

	// ID3v2.4
	if(flags & 0x10)
	{
		// Footer present
		flags &= ~0x10;
	}
	if(flags && ver <= 4)
		return;

	// Size (after unsychronisation and including padding, without header)
	if((pData[0] | pData[1] | pData[2] | pData[3]) & 0x80)
		return;
uint m_size = (pData[0] << (24 - 3)) | (pData[1] << (16 - 2)) | (pData[2] << (8 - 1)) | pData[3];
	pData += 4;

	if(int(f_data.size() - ((const uchar*)pData - &f_data[0])) < int(m_size))
		return;
	if(ver == 3)
	{
		if( !parse3(pData, m_size) )
			return;
	}

	m_valid = true;
}


bool CID3v2::isValidFrame(const Frame3& f_frame) const
{
	for(uint i = 0; i < sizeof(f_frame.Header.Id) / sizeof(f_frame.Header.Id[0]); i++)
	{
		char c = f_frame.Header.Id[i];
		if(c < '0' || (c > '9' && c < 'A') || c > 'Z')
			return false;
	}
	return true;
}


bool CID3v2::parse3(const char* f_data, uint f_size)
{
#define FOUR_CC(A, B, C, D) \
(( (A) & 0xFF)			|\
 (((B) & 0xFF) <<  8)	|\
 (((C) & 0xFF) << 16)	|\
 (((D) & 0xFF) << 24))

	// ID3v2 tags are limited to 256 MB maximum
	const char* pData;
	int size;

	for(pData = f_data, size = f_size; size >= (int)sizeof(Frame3);)
	{
		const Frame3& f = *(const Frame3*)pData;
		if(!isValidFrame(f))
			break;

		uint uDataSize = f.Header.getSize();
//std::cout << f.Header.Id[0] << f.Header.Id[1] << f.Header.Id[2] << f.Header.Id[3] << std::endl;
		ASSERT(uDataSize);
		ASSERT(size >= sizeof(f.Header) + uDataSize);
		
		enum TagFlags
		{
			TFTagAlter		= 0x0080,
			TFFileAlter		= 0x0040,
			TFReadOnly		= 0x0020,
			TFCompression	= 0x8000,
			TFEncryption	= 0x4000,
			TFGroupingId	= 0x2000,

			TFReserved		= 0x1F1F
		};
		ASSERT(~f.Header.Flags & TFTagAlter);
		//ASSERT(~f.Header.Flags & TFFileAlter);
		ASSERT(~f.Header.Flags & TFReadOnly);
		ASSERT(~f.Header.Flags & TFCompression);
		ASSERT(~f.Header.Flags & TFEncryption);
		ASSERT(~f.Header.Flags & TFGroupingId);
		ASSERT( !(f.Header.Flags & TFReserved) );

		switch(f.Header.IdFourCC)
		{
			// Track
			case FOUR_CC('T','R','C','K'):
				m_track = parseTextFrame(*(const TextFrame*)f.Data, uDataSize);
				break;
			// Disk
			case FOUR_CC('T','P','O','S'):
				m_disk = parseTextFrame(*(const TextFrame*)f.Data, uDataSize);
				break;
			// Title
			case FOUR_CC('T','I','T','2'):
				m_title = parseTextFrame(*(const TextFrame*)f.Data, uDataSize);
				break;
			// Artist
			case FOUR_CC('T','P','E','1'):
				m_artist = parseTextFrame(*(const TextFrame*)f.Data, uDataSize);
				break;
			// Album
			case FOUR_CC('T','A','L','B'):
				m_album = parseTextFrame(*(const TextFrame*)f.Data, uDataSize);
				break;
			// Year
			case FOUR_CC('T','Y','E','R'):
				m_year = parseTextFrame(*(const TextFrame*)f.Data, uDataSize);
				break;
			// Genre
			case FOUR_CC('T','C','O','N'):
				m_genre = GenrePtr(new CGenre(parseTextFrame(*(const TextFrame*)f.Data, uDataSize)));
				break;
			// Comment
			case FOUR_CC('C','O','M','M'):
				break;
			// Album Artist
			case FOUR_CC('T','P','E','2'):
				break;
			// Composer
			case FOUR_CC('T','C','O','M'):
				break;
			// Publisher
			case FOUR_CC('T','P','U','B'):
				break;
			// Original Artist
			case FOUR_CC('T','O','P','E'):
				break;
			// Copyright
			case FOUR_CC('T','C','O','P'):
				break;
			// URL
			case FOUR_CC('W','X','X','X'):
				break;
			// Encoded
			case FOUR_CC('T','E','N','C'):
				break;
			// BPM
			case FOUR_CC('T','B','P','M'):
				break;
		}

		pData += sizeof(f.Header) + uDataSize;
		size  -= sizeof(f.Header) + uDataSize;
	}

	// Validate tail
	for(; size; size--, pData++)
		ASSERT(*pData == 0x00);

	return true;
}


std::string CID3v2::parseTextFrame(const TextFrame& f_frame, uint f_uFrameSize) const
{
	uint uRawSize = f_uFrameSize - sizeof(f_frame.Encoding);
	ASSERT((int)uRawSize > 0);

	switch(f_frame.Encoding)
	{
		case 0x00 /*ISO-8859-1 (LATIN-1)*/:
			ASSERT(!"ISO-8859-1");
			//return std::string((const char*)f_frame.RawString, uRawSize);
		case 0x01 /*UCS-2 (UTF-16, with BOM)*/:
			return UTF8::fromUCS2(f_frame.RawString, uRawSize);
		case 0x02 /*UTF-16BE (without BOM, since v2.4)*/:
			ASSERT(!"UTF-16BE");
		case 0x03 /*UTF-8 (since v2.4)*/:
			ASSERT(!"UTF-8");
		default:
			ASSERT(!"Unsupported encoding");
	}
}
