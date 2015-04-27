#include "frame.h"

#include "common.h"
#include "utf8.h"

#include "id3v2.h" // FrameID

#include <cstring> // memcpy


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
#define FCC_PICTURE		FOUR_CC('A','P','I','C')

// ============================================================================
CFrame3* CFrame3::gen(const Frame3& f_frame, uint f_uDataSize, uint* pFrameID)
{
	if( !f_frame.isValid() )
	{
		ERROR("Invalid frame " <<
			  f_frame.Header.Id[0] <<
			  f_frame.Header.Id[1] <<
			  f_frame.Header.Id[2] <<
			  f_frame.Header.Id[3]);
		return NULL;
	}

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
		case FCC_ENCODED:	*pFrameID = FrameEncoded;		break;

		case FCC_GENRE:
			*pFrameID = FrameGenre;
			return new CGenreFrame3(*(const TextFrame3*)f_frame.Data, f_uDataSize);

		case FCC_COMMENT:
			*pFrameID = FrameComment;
			return new CCommentFrame3(*(const CommentFrame3*)f_frame.Data, f_uDataSize);

		case FCC_URL:
			*pFrameID = FrameURL;
			return new CURLFrame3(*(const URLFrame3*)f_frame.Data, f_uDataSize);

		case FCC_PICTURE:
			*pFrameID = FramePicture;
			return new CPictureFrame3(*(const PictureFrame3*)f_frame.Data, f_uDataSize);

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
	m_id = std::string(f_frame.Header.Id, sizeof(f_frame.Header.Id));
}

// ============================================================================
static std::string toString(const char* f_data, uint f_size, Encoding f_encoding)
{
	if(!f_size)
		return std::string("");

	switch(f_encoding)
	{
		case EncRaw:
			return std::string(f_data, f_size);
		case EncUCS2:
			return UTF8::fromUCS2(f_data, f_size);
		case EncUTF16BE:
			ASSERT(!"UTF-16BE");
		case EncUTF8:
			return std::string(f_data, f_size);
		default:
			ASSERT(!"Unsupported encoding");
	}
}


CTextFrame3::CTextFrame3(const TextFrame3& f_frame, uint f_uFrameSize)
{
	ASSERT(f_uFrameSize >= sizeof(f_frame.Encoding));
	m_encodingRaw = (Encoding)f_frame.Encoding;

	uint uRawStringSize = f_uFrameSize - sizeof(f_frame.Encoding);
	m_text = toString(f_frame.RawString, uRawStringSize, m_encodingRaw);
}

// ============================================================================
static int parseIndex(std::string::const_iterator& f_it, const std::string::const_iterator& f_end)
{
	ASSERT(f_it != f_end);
	ASSERT(*f_it == '(');

	std::string::const_iterator it = f_it + 1;
	for(int i = 0; it != f_end; it++)
	{
		char c = *it;

		if(c == ')')
		{
			f_it = ++it;
			return i;
		}

		if(c <= '0' || c >= '9')
			break;
		i = i * 10 + (c - '0');
	}

	return -1;
}

void CGenreFrame3::init(const std::string& f_text)
{
	m_indexV1 = -1;
	m_extended = false;

	std::string::const_iterator  it = f_text.begin();
	std::string::const_iterator end = f_text.end();
	if(it == end)
		return;

	if(*it == '(')
		m_indexV1 = parseIndex(it, end);
	m_text = std::string(it, end);

	// Try convert to index
	if(m_text.empty())
		return;

	int i = genre(m_text.c_str());
	if((i != -1) &&
	   (m_indexV1 == -1 || i == m_indexV1))
	{
		m_text.clear();
		m_indexV1 = i;
		return;
	}

	if(m_indexV1 != -1)
		m_extended = true;
}

// ============================================================================
template<typename T>
static std::string parseTextField(const T* f_pData, uint* f_pSize, Encoding f_encoding)
{
	// Search for . . . <0> . . .
	for(uint i = 0, n = *f_pSize; i < n; i += sizeof(T))
	{
		ASSERT((const char*)((const T*)((const char*)f_pData + i) + 1) <=
			   (const char*)f_pData + n);
		if(*(const T*)((const char*)f_pData + i) == 0)
		{
			*f_pSize = i + sizeof(T);
			return toString((const char*)f_pData, i, f_encoding);
		}
	}
	// No need to update *f_pSize if it was 0 or the end of the first string was not found
	return std::string("");
}

static std::string parseTextField(const char* f_pData, uint* f_pSize, Encoding f_encoding)
{
	switch(f_encoding)
	{
		case EncRaw:
		case EncUTF8:
			return parseTextField<char>(f_pData, f_pSize, f_encoding);
		default:
			return parseTextField<short>((const short*)f_pData, f_pSize, f_encoding);
	}
}


CCommentFrame3::CCommentFrame3(const CommentFrame3& f_frame, uint f_uFrameSize)
{
	ASSERT(f_uFrameSize > sizeof(f_frame.Encoding) + sizeof(f_frame.Language));
	m_encodingRaw = (Encoding)f_frame.Encoding;

	for(uint i = 0; i < sizeof(m_lang) / sizeof(*m_lang); i++)
		m_lang[i] = f_frame.Language[i];

	uint uRawSize = f_uFrameSize - sizeof(f_frame.Encoding) - sizeof(f_frame.Language);
	uint size = uRawSize;

	m_short = parseTextField(f_frame.RawShortString, &size, m_encodingRaw);
	ASSERT(m_short == std::string(""));

	ASSERT(size <= uRawSize);
	m_text = toString(f_frame.RawShortString + size, uRawSize - size, m_encodingRaw);
}

// ============================================================================
CURLFrame3::CURLFrame3(const URLFrame3& f_frame, uint f_uFrameSize)
{
	ASSERT(f_uFrameSize > sizeof(f_frame.Encoding));
	m_encodingRaw = (Encoding)f_frame.Encoding;

	uint uRawSize = f_uFrameSize - sizeof(f_frame.Encoding);
	uint size = uRawSize;

	m_description = parseTextField(f_frame.Description, &size, m_encodingRaw);
	ASSERT(m_description == std::string(""));

	ASSERT(size <= uRawSize);
	m_text = toString(f_frame.Description + size, uRawSize - size, EncRaw);
}

// ============================================================================
CPictureFrame3::CPictureFrame3(const PictureFrame3& f_frame, uint f_uFrameSize)
{
	uint size = f_uFrameSize;

	// Encoding
	ASSERT(size > sizeof(f_frame.Encoding));
	m_encodingRaw = (Encoding)f_frame.Encoding;
	size -= sizeof(f_frame.Encoding);

	// MIME
	const char* pData = f_frame.MIME;
	for(; size && *pData; pData++, size--) {}
	ASSERT((int)size > 0);
	if(uint s = (uint)(pData - f_frame.MIME))
		m_mime = std::string(f_frame.MIME, s);
	size--;
	pData++;

	// Picture Type
	ASSERT(size >= sizeof(char));
	m_type = (PictureType)*pData;
	size--;
	pData++;

	// Description
	uint sz = size;
	m_description = parseTextField(pData, &sz, m_encodingRaw);
	ASSERT(sz <= size);
	size -= sz;
	pData += sz;

	// Image Data
	m_data.resize(size);
	memcpy(&m_data[0], pData, size);
}

