#include "id3v2.h"

#include "common.h"
#include "utf8.h"
#include "genre.h"


#define FOUR_CC(A, B, C, D)	 \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))

#define FCC_TRACK		FOUR_CC('T','R','C','K')
#define FCC_DISC		FOUR_CC('T','P','O','S')
#define FCC_TITLE		FOUR_CC('T','I','T','2')
#define FCC_ARTIST		FOUR_CC('T','P','E','1')
#define FCC_ALBUM		FOUR_CC('T','A','L','B')
#define FCC_YEAR		FOUR_CC('T','Y','E','R')
#define FCC_GENRE		FOUR_CC('T','C','O','N')
#define FCC_COMMENT		FOUR_CC('C','O','M','M')
#define FCC_AARTIST		FOUR_CC('T','P','E','2')
#define FCC_COMPOSER	FOUR_CC('T','C','O','M')
#define FCC_PUBLISHER	FOUR_CC('T','P','U','B')
#define FCC_OARTIST		FOUR_CC('T','O','P','E')
#define FCC_COPYRIGHT	FOUR_CC('T','C','O','P')
#define FCC_URL			FOUR_CC('W','X','X','X')
#define FCC_ENCODED		FOUR_CC('T','E','N','C')
#define FCC_BPM			FOUR_CC('T','B','P','M')


struct __attribute__ ((__packed__)) Frame3
{
	struct __attribute__ ((__packed__))
	{
		union
		{
			char Id[4];
			uint IdFourCC;
		};
		uchar SizeRaw[4];
		ushort Flags;
		uint getSize() const
		{
			return (SizeRaw[0] << 24) | (SizeRaw[1] << 16) | (SizeRaw[2] << 8) | SizeRaw[3];
		}
	} Header;
	const uchar Data[1];

	bool isValid() const
	{
		for(uint i = 0; i < sizeof(Header.Id) / sizeof(Header.Id[0]); i++)
		{
			char c = Header.Id[i];
			if(c < '0' || (c > '9' && c < 'A') || c > 'Z')
				return false;
		}
		return true;
	}
};


struct __attribute__ ((__packed__)) TextFrame3
{
	uchar Encoding;
	const char RawString[1];
};

// ============================================================================
// Basic Routines
bool		CID3v2::isValid()		const { return m_valid;   }

uint		CID3v2::getVersion()	const { return m_version; }

const std::string&	CID3v2::getTrack()			const { return strTextFrame(m_iTrack);		}
const std::string&	CID3v2::getDisc()			const { return strTextFrame(m_iDisc);		}
const std::string&	CID3v2::getBPM()			const { return strTextFrame(m_iBPM);		}
const std::string&	CID3v2::getTitle()			const { return strTextFrame(m_iTitle);		}
const std::string&	CID3v2::getArtist()			const { return strTextFrame(m_iArtist);		}
const std::string&	CID3v2::getAlbum()			const { return strTextFrame(m_iAlbum);		}
const std::string&	CID3v2::getAlbumArtist()	const { return strTextFrame(m_iAArtist);	}
const std::string&	CID3v2::getYear()			const { return strTextFrame(m_iYear);		}

const std::string&	CID3v2::getComposer()		const { return strTextFrame(m_iComposer);	}
const std::string&	CID3v2::getPublisher()		const { return strTextFrame(m_iPublisher);	}
const std::string&	CID3v2::getOArtist()		const { return strTextFrame(m_iOArtist);	}
const std::string&	CID3v2::getCopyright()		const { return strTextFrame(m_iCopyright);	}
//const std::string&	CID3v2::getURL()			const { return strTextFrame(m_iCopyright);	}
const std::string&	CID3v2::getEncoded()		const { return strTextFrame(m_iEncoded);	}

bool CID3v2::isExtendedGenre() const
{
	return isValidIndex(m_iGenre) ? static_cast<const CGenreFrame3*>(m_frames[m_iGenre])->get().isExtended()
								  : false;
}
const std::string CID3v2::getGenre() const
{
	if( !isValidIndex(m_iGenre) )
		return m_strEmpty;
	const CGenre& genre = static_cast<const CGenreFrame3*>(m_frames[m_iGenre])->get();
	if(genre.isExtended())
		return std::string( CGenre::get(genre.getIndex()) );
	else
		return genre.str();
}
const std::string& CID3v2::getGenreEx() const
{
	if( !isValidIndex(m_iGenre) )
		return m_strEmpty;
	const CGenre& genre = static_cast<const CGenreFrame3*>(m_frames[m_iGenre])->get();
	return genre.isExtended() ? genre.str() : m_strEmpty;
}
int CID3v2::getGenreIndex() const
{
	return isValidIndex(m_iGenre) ? static_cast<const CGenreFrame3*>(m_frames[m_iGenre])->get().getIndex()
								  : -1;
}

// Complex Routines
CID3v2::CID3v2(const std::vector<uchar>& f_data):
	m_valid(false),
	m_version(0),
	m_iFrame(0),
	m_strEmpty("")
{
	m_iTrack = m_iDisc = m_iBPM =
	m_iTitle = m_iArtist = m_iAlbum = m_iYear = m_iGenre = /*m_iComment =*/ m_iAArtist =
	m_iComposer = m_iPublisher = m_iOArtist = m_iCopyright = /*m_iURL =*/ m_iEncoded = -1;

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

CID3v2::~CID3v2() { cleanup(); }


void CID3v2::cleanup()
{
	for(uint i = 0, n = (uint)m_frames.size(); i < n; i++)
		delete m_frames[i];
}


bool CID3v2::parse3(const char* f_data, uint f_size)
{
	// ID3v2 tags are limited to 256 MB maximum
	const char* pData;
	int size;

	for(pData = f_data, size = f_size, m_iFrame = 0; size >= (int)sizeof(Frame3); m_iFrame++)
	{
		const Frame3& f = *(const Frame3*)pData;
		uint uDataSize = f.Header.getSize();
//std::cout << ">>> " << size << " | " << sizeof(f.Header) << " + " << uDataSize << std::endl;
//std::cout << f.Header.Id[0] << f.Header.Id[1] << f.Header.Id[2] << f.Header.Id[3] << std::endl;
		if(!uDataSize)
			break;

		ASSERT(size >= (int)(sizeof(f.Header) + uDataSize));
		CFrame3* pFrame = CFrame3::gen(f, uDataSize);
		if(!pFrame)
		{
			cleanup();
			return false;
		}

		switch(f.Header.IdFourCC)
		{
			case FCC_TRACK:		m_iTrack		= m_iFrame;	break;
			case FCC_DISC:		m_iDisc			= m_iFrame;	break;
			case FCC_TITLE:		m_iTitle		= m_iFrame;	break;
			case FCC_ARTIST:	m_iArtist		= m_iFrame;	break;
			case FCC_ALBUM:		m_iAlbum		= m_iFrame;	break;
			case FCC_YEAR:		m_iYear			= m_iFrame;	break;
			case FCC_GENRE:		m_iGenre		= m_iFrame;	break;
			//case FCC_COMMENT:	m_iComment		= m_iFrame;	break;
			case FCC_AARTIST:	m_iAArtist		= m_iFrame;	break;
			case FCC_COMPOSER:	m_iComposer		= m_iFrame;	break;
			case FCC_PUBLISHER:	m_iPublisher	= m_iFrame;	break;
			case FCC_OARTIST:	m_iOArtist		= m_iFrame;	break;
			case FCC_COPYRIGHT:	m_iCopyright	= m_iFrame;	break;
			//case FCC_URL:		m_iURL			= m_iFrame;	break;
			case FCC_ENCODED:	m_iEncoded		= m_iFrame;	break;
			case FCC_BPM:		m_iBPM			= m_iFrame;	break;

			default:;
		}
		m_frames.push_back(pFrame);

		pData += sizeof(f.Header) + uDataSize;
		size  -= sizeof(f.Header) + uDataSize;
	}

	// Validate tail
	for(; size; size--, pData++)
		ASSERT(*pData == 0x00);

	return true;
}


bool CID3v2::isValidIndex(int f_index) const
{
	return ((uint)f_index < (uint)m_frames.size());
}

const std::string& CID3v2::strTextFrame(int f_index) const
{
	return isValidIndex(f_index) ? static_cast<const CTextFrame3*>(m_frames[f_index])->get()
								 : m_strEmpty;
}

// ============================================================================
CFrame3* CFrame3::gen(const Frame3& f_frame, uint f_uDataSize)
{
	if( !f_frame.isValid() )
		return NULL;

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
	ASSERT(~f_frame.Header.Flags & TFTagAlter);
	//ASSERT(~f_frame.Header.Flags & TFFileAlter);
	ASSERT(~f_frame.Header.Flags & TFReadOnly);
	ASSERT(~f_frame.Header.Flags & TFCompression);
	ASSERT(~f_frame.Header.Flags & TFEncryption);
	ASSERT(~f_frame.Header.Flags & TFGroupingId);
	ASSERT( !(f_frame.Header.Flags & TFReserved) );

	switch(f_frame.Header.IdFourCC)
	{
		case FCC_TRACK:
		case FCC_DISC:
		case FCC_TITLE:
		case FCC_ARTIST:
		case FCC_ALBUM:
		case FCC_YEAR:
		case FCC_AARTIST:
		case FCC_COMPOSER:
		case FCC_PUBLISHER:
		case FCC_OARTIST:
		case FCC_COPYRIGHT:
		case FCC_ENCODED:
		case FCC_BPM:
			return new CTextFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);
		case FCC_GENRE:
			return new CGenreFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);

		//case FCC_COMMENT:
		//	break;

		//case FCC_URL:
		//	break;

		default:
			return new CRawFrame3(f_frame);
	}
}

// ====================================
CRawFrame3::CRawFrame3(const Frame3& f_frame):
	m_frame(sizeof(f_frame.Header) + f_frame.Header.getSize())
{
	memcpy(&m_frame[0], &f_frame, m_frame.size());
}

// ====================================
const std::string& CTextFrame3::get() const { return m_text; }

CTextFrame3::CTextFrame3(const TextFrame3& f_frame, uint f_uFrameSize)
{
	uint uRawStringSize = f_uFrameSize - sizeof(f_frame.Encoding);
	ASSERT((int)uRawStringSize > 0);

	switch(f_frame.Encoding)
	{
		case 0x00 /*ISO-8859-1 (LATIN-1)*/:
			m_text = std::string((const char*)f_frame.RawString, uRawStringSize);
			break;
		case 0x01 /*UCS-2 (UTF-16, with BOM)*/:
			m_text = UTF8::fromUCS2(f_frame.RawString, uRawStringSize);
			break;
		case 0x02 /*UTF-16BE (without BOM, since v2.4)*/:
			ASSERT(!"UTF-16BE");
		case 0x03 /*UTF-8 (since v2.4)*/:
			ASSERT(!"UTF-8");
		default:
			ASSERT(!"Unsupported encoding");
	}
}

// ====================================
const CGenre& CGenreFrame3::get() const { return m_genre; }

CGenreFrame3::CGenreFrame3(const TextFrame3& f_frame, uint f_uFrameSize):
	CTextFrame3(f_frame, f_uFrameSize),
	m_genre(m_text)
{}

