#include "frame.h"

#include "common.h"
#include "utf8.h"

#include "id3v2.h" // FrameID

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

// ============================================================================
CFrame3* CFrame3::gen(const Frame3& f_frame, uint f_uDataSize, uint* pFrameID)
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
		case FCC_COMPOSER:	*pFrameID = FrameComposer;		break;
		case FCC_PUBLISHER:	*pFrameID = FramePublisher;		break;
		case FCC_OARTIST:	*pFrameID = FrameOrigArtist;	break;
		case FCC_COPYRIGHT:	*pFrameID = FrameCopyright;		break;
		//case FCC_URL:		*pFrameID = FrameURL;			break;
		case FCC_ENCODED:	*pFrameID = FrameEncoded;		break;

		case FCC_GENRE:
			*pFrameID = FrameGenre;
			return new CGenreFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);

		case FCC_COMMENT:
			*pFrameID = FrameComment;
			return new CCommentFrame3(*(const CommentFrame3*)f_frame.Data, f_uDataSize);

		default:
			*pFrameID = FrameUnknown;
			return new CRawFrame3(f_frame);
	}
	return new CTextFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);
}

// ============================================================================
CRawFrame3::CRawFrame3(const Frame3& f_frame):
	m_frame(sizeof(f_frame.Header) + f_frame.Header.getSize())
{
	memcpy(&m_frame[0], &f_frame, m_frame.size());
}

// ============================================================================
static std::string toString(const char* f_data, uint f_size, Encoding f_encoding)
{
	switch(f_encoding)
	{
		case EncRaw:
			return std::string(f_data, f_size);
		case EncUCS2:
			return UTF8::fromUCS2(f_data, f_size);
		case EncUTF16BE:
			ASSERT(!"UTF-16BE");
		case EncUTF8:
			ASSERT(!"UTF-8");
		default:
			ASSERT(!"Unsupported encoding");
	}
}


CTextFrame3::CTextFrame3(const TextFrame3& f_frame, uint f_uFrameSize)
{
	ASSERT(f_uFrameSize >= sizeof(f_frame.Encoding));
	m_encodingRaw = (Encoding)f_frame.Encoding;

	uint uRawStringSize = f_uFrameSize - sizeof(f_frame.Encoding);
	if(!uRawStringSize)
		return;
	m_text = toString(f_frame.RawString, uRawStringSize, m_encodingRaw);
}


CTextFrame3* CTextFrame3::create()
{
	TextFrame3 frame;
	frame.Encoding = EncUCS2;
	return new CTextFrame3(frame, sizeof(frame) - sizeof(frame.RawString));
}


CTextFrame3& CTextFrame3::operator=(const std::string& f_val)
{
	m_text = f_val;
	return *this;
}

// ============================================================================
CCommentFrame3::CCommentFrame3(const CommentFrame3& f_frame, uint f_uFrameSize)
{
	ASSERT(f_uFrameSize > sizeof(f_frame.Encoding) + sizeof(f_frame.Language));
	m_encodingRaw = (Encoding)f_frame.Encoding;

	for(uint i = 0; i < sizeof(m_lang) / sizeof(*m_lang); i++)
		m_lang[i] = f_frame.Language[i];

	uint uRawStringsSize = f_uFrameSize - sizeof(f_frame.Encoding) - sizeof(f_frame.Language);
	if(m_encodingRaw == EncRaw)
		fill<char> (f_frame.RawShortString, uRawStringsSize);
	else
		fill<short>((const short*)f_frame.RawShortString, uRawStringsSize,
					(m_encodingRaw == EncUTF8) ? sizeof(char) : sizeof(short));
}


template<typename T>
void CCommentFrame3::fill(const T* f_data, uint f_size, uint f_step)
{
	const T* pStr = f_data;
	uint n;

	// Search for . . . <0> . . .
	for(n = f_size; n; n -= f_step, pStr = (const T*)((const char*)pStr + f_step))
	{
		if(pStr != 0)
			continue;

		if(f_size - n)
			m_short = toString((const char*)f_data, f_size - n, m_encodingRaw);

		pStr++;
		n -= sizeof(T);
		break;
	}
	ASSERT(n);

	if(n)
		m_full = toString((const char*)pStr, n, m_encodingRaw);
}

