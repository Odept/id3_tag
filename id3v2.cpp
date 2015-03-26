#include "id3v2.h"

#include "common.h"
#include "utf8.h"
#include "genre.h"

#include <cstring> // memcpy


#define FOUR_CC(A, B, C, D)	 \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))

#define FCC_TRACK		FOUR_CC('T','R','C','K')
#define FCC_DISC		FOUR_CC('T','P','O','S')
#define FCC_BPM			FOUR_CC('T','B','P','M')
#define FCC_TITLE		FOUR_CC('T','I','T','2')
#define FCC_ARTIST		FOUR_CC('T','P','E','1')
#define FCC_ALBUM		FOUR_CC('T','A','L','B')
#define FCC_AARTIST		FOUR_CC('T','P','E','2')
#define FCC_YEAR		FOUR_CC('T','Y','E','R')
#define FCC_GENRE		FOUR_CC('T','C','O','N')
#define FCC_COMMENT		FOUR_CC('C','O','M','M')
#define FCC_COMPOSER	FOUR_CC('T','C','O','M')
#define FCC_PUBLISHER	FOUR_CC('T','P','U','B')
#define FCC_OARTIST		FOUR_CC('T','O','P','E')
#define FCC_COPYRIGHT	FOUR_CC('T','C','O','P')
#define FCC_URL			FOUR_CC('W','X','X','X')
#define FCC_ENCODED		FOUR_CC('T','E','N','C')


struct __attribute__ ((__packed__)) Tag
{
	struct __attribute__ ((__packed__)) Header_t
	{
		char	Id[3];
		uchar	Version;
		uchar	Revision;
		uchar	Flags;
		uint	Size;

		bool isValid() const
		{
			// Check for 'ID3'
			if(Id[0] != 'I' || Id[1] != 'D' || Id[2] != '3')
				return false;

			// Version
			if(Version == 0xFF || Revision == 0xFF)
				return false;

			// Flags::ID3v2
			if((Flags & ~0x80) && Version == 0)
				return false;

			// Flags::ID3v2.3
			if((Flags & ~0xE0) && Version <= 3)
				return false;

			// Flags::ID3v2.4
			if((Flags & ~0xF0) && Version <= 4)
				return false;

			// Size
			if(Size & 0x80808080)
				return false;

			return true;
		}

		// Size (after unsychronisation and including padding, without header)
		uint getSize() const
		{
			const uchar* pSize = (const uchar*)&Size;
			return (pSize[0] << (24 - 3)) |
				   (pSize[1] << (16 - 2)) |
				   (pSize[2] << ( 8 - 1)) |
				    pSize[3];
		}
	} Header;
	uchar Frames[1];
};


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
	uchar Data[1];

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
	char RawString[1];
};

// ============================================================================
// Basic Routines
uint		CID3v2::getVersion()	const { return m_version; }

const std::string&	CID3v2::getTrack()			const { return strTextFrame(FrameTrack);		}
const std::string&	CID3v2::getDisc()			const { return strTextFrame(FrameDisc);			}
const std::string&	CID3v2::getBPM()			const { return strTextFrame(FrameBPM);			}
const std::string&	CID3v2::getTitle()			const { return strTextFrame(FrameTitle);		}
const std::string&	CID3v2::getArtist()			const { return strTextFrame(FrameArtist);		}
const std::string&	CID3v2::getAlbum()			const { return strTextFrame(FrameAlbum);		}
const std::string&	CID3v2::getAlbumArtist()	const { return strTextFrame(FrameAlbumArtist);	}
const std::string&	CID3v2::getYear()			const { return strTextFrame(FrameYear);			}

const std::string&	CID3v2::getComposer()		const { return strTextFrame(FrameComposer);		}
const std::string&	CID3v2::getPublisher()		const { return strTextFrame(FramePublisher);	}
const std::string&	CID3v2::getOArtist()		const { return strTextFrame(FrameOrigArtist);	}
const std::string&	CID3v2::getCopyright()		const { return strTextFrame(FrameCopyright);	}
//const std::string&	CID3v2::getURL()			const { return strTextFrame(FrameURL);	}
const std::string&	CID3v2::getEncoded()		const { return strTextFrame(FrameEncoded);		}

bool CID3v2::isExtendedGenre() const
{
	const CGenreFrame3* pGenre = getGenreFrame();
	return pGenre ? pGenre->get().isExtended() : false;
}
const std::string CID3v2::getGenre() const
{
	const CGenreFrame3* pGenre = getGenreFrame();
	if(!pGenre)
		return m_strEmpty;

	const CGenre& genre = pGenre->get();
	if(genre.isExtended())
		return std::string( CGenre::get(genre.getIndex()) );
	else
		return genre.str();
}
const std::string& CID3v2::getGenreEx() const
{
	const CGenreFrame3* pGenre = getGenreFrame();
	if(!pGenre)
		return m_strEmpty;

	const CGenre& genre = pGenre->get();
	return genre.isExtended() ? genre.str() : m_strEmpty;
}
int CID3v2::getGenreIndex() const
{
	const CGenreFrame3* pGenre = getGenreFrame();
	return pGenre ? pGenre->get().getIndex() : -1;
}

const std::vector<CFrame3*> CID3v2::getUnknownFrames() const { return m_framesUnknown; }


CID3v2::~CID3v2() { cleanup(); }

// ====================================
// Complex Routines
CID3v2* CID3v2::gen(const uchar* f_pData, unsigned long long f_size)
{
	const Tag* pTag = findTag(f_pData, f_size);
	if(!pTag)
		return NULL;
	if(f_size < sizeof(pTag->Header) + pTag->Header.getSize())
		return NULL;

	CID3v2* p = new CID3v2(*pTag);
	if(p->parse(*pTag))
		return p;
	else
	{
		delete p;
		return NULL;
	}
}


const Tag* CID3v2::findTag(const uchar* f_pData, unsigned long long f_size)
{
	ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
	if(f_size < sizeof(Tag::Header))
		return NULL;

	const uchar* pData = f_pData;
	for(uint n = (uint)f_size - sizeof(Tag::Header); n; n--, pData++)
	{
		if( ((const Tag::Header_t*)pData)->isValid() )
			return (const Tag*)pData;
	}
	return NULL;
}


CID3v2::CID3v2(const Tag& f_tag):
	m_version(0),
	m_strEmpty("")
{
	// Constructor is internal and the header is already validated here
	const Tag::Header_t& h = f_tag.Header;

	// Version
	ASSERT(h.Version == 3);
	m_version = (h.Version << 8) | h.Revision;

	// Flags: unsynchronisation (ID3v2)
	if(h.Flags & 0x80)
	{
		ASSERT(!"Unsynchronisation");
	}

	// Flags: extended header (ID3v2.3)
	if(h.Flags & 0x40)
	{
		ASSERT(!"Extended header");
	}

	// Flags: experimental indicator (ID3v2.3)
	if(h.Flags & 0x20)
	{
		ASSERT(!"Experimental indicator");
	}

	// Flags: footer present (ID3v2.4)
	if(h.Flags & 0x10)
	{
		ASSERT(!"Footer present");
	}
}


bool CID3v2::parse(const Tag& f_tag)
{
	switch (m_version >> 8)
	{
		case 3:
			return parse3(f_tag);
		default:
			return false;
	}
}


bool CID3v2::parse3(const Tag& f_tag)
{
	// ID3v2 tags are limited to 256 MB maximum
	const uchar* pData;
	int size;

	for(pData = (const uchar*)f_tag.Frames, size = f_tag.Header.getSize();
		size >= (int)sizeof(Frame3);)
	{
		const Frame3& f = *(const Frame3*)pData;
		uint uDataSize = f.Header.getSize();
//std::cout << ">>> " << size << " | " << sizeof(f.Header) << " + " << uDataSize << std::endl;
//std::cout << f.Header.Id[0] << f.Header.Id[1] << f.Header.Id[2] << f.Header.Id[3] << std::endl;
		if(!uDataSize)
			break;

		ASSERT(size >= (int)(sizeof(f.Header) + uDataSize));
		FrameID frameID;
		CFrame3* pFrame = CFrame3::gen(f, uDataSize, &frameID);
		if(!pFrame)
		{
			cleanup();
			return false;
		}

		if(frameID == FrameUnknown)
			m_framesUnknown.push_back(pFrame);
		else
		{
			ASSERT(m_frames.find(frameID) == m_frames.end())
			m_frames[frameID] = pFrame;
		}

		pData += sizeof(f.Header) + uDataSize;
		size  -= sizeof(f.Header) + uDataSize;
	}

	// Validate tail
	for(; size; size--, pData++)
		ASSERT(*pData == 0x00);

	return true;
}


const CTextFrame3* CID3v2::getTextFrame(FrameID f_id) const
{
	frames_t::const_iterator it = m_frames.find(f_id);
	return (it == m_frames.end()) ? NULL : static_cast<const CTextFrame3*>(it->second);
}

const CGenreFrame3* CID3v2::getGenreFrame() const
{
	return static_cast<const CGenreFrame3*>( getTextFrame(FrameGenre) );
}

const std::string& CID3v2::strTextFrame(FrameID f_id) const
{
	const CTextFrame3* pFrame = getTextFrame(f_id);
	return pFrame ? pFrame->get() : m_strEmpty;
}


void CID3v2::cleanup()
{
	for(frames_t::iterator it = m_frames.begin(), end = m_frames.end(); it != end; it++)
		delete it->second;
	m_frames.clear();

	for(uint i = 0, n = (uint)m_framesUnknown.size(); i < n; i++)
		delete m_framesUnknown[i];
	m_framesUnknown.resize(0);
}

// ============================================================================
CFrame3* CFrame3::gen(const Frame3& f_frame, uint f_uDataSize, FrameID* pFrameID)
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
		case FCC_TRACK:		*pFrameID = FrameTrack;			break;
		case FCC_DISC:		*pFrameID = FrameDisc;			break;
		case FCC_BPM:		*pFrameID = FrameBPM;			break;
		case FCC_TITLE:		*pFrameID = FrameTitle;			break;
		case FCC_ARTIST:	*pFrameID = FrameArtist;		break;
		case FCC_ALBUM:		*pFrameID = FrameAlbum;			break;
		case FCC_AARTIST:	*pFrameID = FrameAlbumArtist;	break;
		case FCC_YEAR:		*pFrameID = FrameYear;			break;
		case FCC_GENRE:
			*pFrameID = FrameGenre;
			return new CGenreFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);
		//case FCC_COMMENT:	*pFrameID = FrameComment;		break;
		case FCC_COMPOSER:	*pFrameID = FrameComposer;		break;
		case FCC_PUBLISHER:	*pFrameID = FramePublisher;		break;
		case FCC_OARTIST:	*pFrameID = FrameOrigArtist;	break;
		case FCC_COPYRIGHT:	*pFrameID = FrameCopyright;		break;
		//case FCC_URL:		*pFrameID = FrameURL;			break;
		case FCC_ENCODED:	*pFrameID = FrameEncoded;		break;

		default:
			*pFrameID = FrameUnknown;
			return new CRawFrame3(f_frame);
	}
	return new CTextFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);
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

