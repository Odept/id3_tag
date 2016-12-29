#include "frame.h"

#include "common.h"
#include "utf8.h"

#include <cstring> // memcpy
#include <sstream>


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
FrameType CFrame3::getFrameType(const Frame3::Header_t& f_header)
{
	if( !f_header.isValid() )
	{
		std::ostringstream oss;
		oss << f_header.Id[0] << f_header.Id[1] << f_header.Id[2] << f_header.Id[3];
		ASSERT_MSG(!"Invalid ID3v2 frame", oss.str());
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
	ASSERT(~f_header.Flags & TFTagAlter);
	//ASSERT(~f_header.Flags & TFFileAlter);
	ASSERT(~f_header.Flags & TFCompression);
	ASSERT(~f_header.Flags & TFEncryption);
	ASSERT(~f_header.Flags & TFGroupingId);
	ASSERT( !(f_header.Flags & TFReserved) );

	if(f_header.Flags & TFReadOnly)
	{
		std::ostringstream oss;
		oss << f_header.Id[0] << f_header.Id[1] << f_header.Id[2] << f_header.Id[3];
		WARNING("The \"" << oss.str() << "\" frame of the ID3v2 tag is read-only - the flag will be cleared if the frame is modified");
	}

	switch(f_header.IdFourCC)
	{
		case FCC_TRACK:		return FrameTrack;
		case FCC_DISC:		return FrameDisc;
		case FCC_BPM:		return FrameBPM;
		case FCC_TITLE:		return FrameTitle;
		case FCC_ARTIST:	return FrameArtist;
		case FCC_ALBUM:		return FrameAlbum;
		case FCC_AARTIST:	return FrameAlbumArtist;
		case FCC_YEAR:		return FrameYear;
		case FCC_COMPOSER:	return FrameComposer;
		case FCC_PUBLISHER:	return FramePublisher;
		case FCC_OARTIST:	return FrameOrigArtist;
		case FCC_COPYRIGHT:	return FrameCopyright;
		case FCC_ENCODED:	return FrameEncoded;
		case FCC_GENRE:		return FrameGenre;
		case FCC_COMMENT:	return FrameComment;
		case FCC_URL:		return FrameURL;
		case FCC_PICTURE:	return FramePicture;

		default:			return FrameUnknown;
	}
}

// ============================================================================
CRawFrame3::CRawFrame3(const Frame3& f_frame):
	m_frame(sizeof(f_frame.Header) + f_frame.Header.size())
{
	memcpy(&m_frame[0], &f_frame, m_frame.size());
	m_id = std::string(f_frame.Header.Id, sizeof(f_frame.Header.Id));
}

// ============================================================================
static std::string toString(const char* f_data, size_t f_size, Encoding f_encoding)
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


CTextFrame3::CTextFrame3(const Frame3& f_frame)
{
	auto& frame = *reinterpret_cast<const TextFrame3*>(f_frame.Data);
	auto size = f_frame.Header.size();

	ASSERT(size >= sizeof(frame.Encoding));
	m_encodingRaw = static_cast<Encoding>(frame.Encoding);

	auto uRawStringSize = size - sizeof(frame.Encoding);
	m_text = toString(frame.RawString, uRawStringSize, m_encodingRaw);
}

// ============================================================================
static int parseIndex(std::string::const_iterator& f_it, const std::string::const_iterator& f_end)
{
	ASSERT(f_it != f_end);
	ASSERT(*f_it == '(');

	auto it = f_it + 1;
	for(int i = 0; it != f_end; it++)
	{
		char c = *it;

		if(c == ')')
		{
			f_it = ++it;
			return i;
		}

		if(c < '0' || c > '9')
			break;
		i = i * 10 + (c - '0');
	}

	return -1;
}

void CGenreFrame3::parse()
{
	if(m_text.empty())
		return;

	if(m_text[0] == '(')
	{
		std::string unparsed = m_text;
		auto it  = unparsed.cbegin();
		auto end = unparsed.cend();

		m_indexV1 = parseIndex(it, end);
		ASSERT(!Tag::genre(m_indexV1).empty());

		m_text = std::string(it, end);
		if(m_text.empty())
			m_text = Tag::genre(m_indexV1);
		else
			updateExtended();
	}
	else
		m_indexV1 = Tag::genre(m_text);
}

// ============================================================================
template<typename T>
static std::string parseTextField(const T* f_data, size_t& f_ioSize, Encoding f_encoding)
{
	// Search for . . . <0> . . .
	for(size_t i = 0, n = f_ioSize; i < n; i += sizeof(T))
	{
		ASSERT(reinterpret_cast<const char*>(reinterpret_cast<const T*>(reinterpret_cast<const char*>(f_data) + i) + 1) <=
			   reinterpret_cast<const char*>(f_data) + n);
		if(*(const T*)((const char*)f_data + i) == 0)
		{
			f_ioSize = i + sizeof(T);
			return toString(reinterpret_cast<const char*>(f_data), i, f_encoding);
		}
	}
	// No need to update the f_ioSize if it was 0 or the end of the first string was not found
	return std::string("");
}

static std::string parseTextField(const char* f_data, size_t& f_ioSize, Encoding f_encoding)
{
	switch(f_encoding)
	{
		case EncRaw:
		case EncUTF8:
			return parseTextField<char>(f_data, f_ioSize, f_encoding);
		default:
			return parseTextField<short>(reinterpret_cast<const short*>(f_data), f_ioSize, f_encoding);
	}
}


CCommentFrame3::CCommentFrame3(const Frame3& f_frame, bool f_bMMJB):
	CTextFrame3("")
{
	auto& frame = *reinterpret_cast<const CommentFrame3*>(f_frame.Data);
	auto size = f_frame.Header.size();

	ASSERT(size > sizeof(frame.Encoding) + sizeof(frame.Language));
	m_encodingRaw = (Encoding)frame.Encoding;

	for(uint i = 0; i < sizeof(m_lang) / sizeof(*m_lang); i++)
		m_lang[i] = frame.Language[i];

	auto uRawSize = size - sizeof(frame.Encoding) - sizeof(frame.Language);
	auto shortNameSize = uRawSize;

	m_short = parseShortString(frame.RawShortString, /*io*/shortNameSize, m_encodingRaw, f_bMMJB);

	ASSERT(shortNameSize <= uRawSize);
	m_text = toString(frame.RawShortString + shortNameSize, uRawSize - shortNameSize, m_encodingRaw);
}

std::string CCommentFrame3::parseShortString(const char* f_data, size_t& f_ioSize, Encoding f_encoding, bool f_bMMJB) const
{
	auto str = parseTextField(f_data, /*io*/f_ioSize, f_encoding);
	if(f_bMMJB)
		ASSERT(hasMMJBPrefix(str));
	else if(!str.empty())
	{
		if( hasMMJBPrefix(str) )
			throw ExceptionMMJB();
		WARNING("short comment \"" << str << "\" is present");
	}
	return str;
}

// ============================================================================
CURLFrame3::CURLFrame3(const Frame3& f_frame):
	CTextFrame3("")
{
	auto& frame = *reinterpret_cast<const URLFrame3*>(f_frame.Data);
	auto size = f_frame.Header.size();

	ASSERT(size > sizeof(frame.Encoding));
	m_encodingRaw = (Encoding)frame.Encoding;

	auto uRawSize = size - sizeof(frame.Encoding);
	auto descSize = uRawSize;

	m_description = parseTextField(frame.Description, /*io*/descSize, m_encodingRaw);
	ASSERT(m_description == std::string(""));

	ASSERT(descSize <= uRawSize);
	m_text = toString(frame.Description + descSize, uRawSize - descSize, EncRaw);
}

// ============================================================================
CPictureFrame3::CPictureFrame3(const Frame3& f_frame)
{
	auto& frame = *reinterpret_cast<const PictureFrame3*>(f_frame.Data);
	auto size = f_frame.Header.size();

	// Encoding
	ASSERT(size > sizeof(frame.Encoding));
	m_encodingRaw = static_cast<Encoding>(frame.Encoding);
	size -= sizeof(frame.Encoding);

	// MIME
	const char* pData = frame.MIME;
	for(; size && *pData; ++pData, --size) {}
	ASSERT(static_cast<long>(size) > 0);
	if(auto s = pData - frame.MIME)
		m_mime = std::string(frame.MIME, s);
	--size;
	++pData;

	// Picture Type
	ASSERT(size >= sizeof(char));
	m_type = static_cast<PictureType>(*pData);
	size--;
	pData++;

	// Description
	auto sz = size;
	m_description = parseTextField(pData, /*io*/sz, m_encodingRaw);
	ASSERT(sz <= size);
	size -= sz;
	pData += sz;

	// Image Data
	m_data.resize(size);
	memcpy(&m_data[0], pData, size);
}

