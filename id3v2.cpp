#include "id3v2.h"

#include "common.h"
#include "genre.h"
#include "frame.h"

// Getters & Setters
uint CID3v2::getVersion() const { return m_version; }

// Helpers
template<typename T>
T* CID3v2::getFrame(FrameID f_id) const
{
	frames_t::const_iterator it = m_frames.find(f_id);
	return (it == m_frames.end()) ? NULL : static_cast<T*>(it->second);
}

template<typename T>
static T* setFrame(T* f_pFrame, const std::string& f_val)
{
	if(f_pFrame)
	{
		*f_pFrame = f_val;
		return NULL;
	}
	else
		return new T(f_val);
}

static const std::string& strTextFrame(const CTextFrame3* f_pFrame, const std::string& f_default)
{
	return f_pFrame ? f_pFrame->get() : f_default;
}

static bool isFrameModified(const CFrame3* f_pFrame) { return f_pFrame ? f_pFrame->isModified() : false; }

#define DEF_GETTER_SETTER(Type, Name) \
	const std::string& CID3v2::get##Name() const \
	{ \
		return strTextFrame(getFrame<Type>(Frame##Name), m_strEmpty); \
	} \
	bool CID3v2::set##Name(const std::string& f_val) \
	{ \
		if(Type* pFrame = setFrame<Type>(getFrame<Type>(Frame##Name), f_val)) \
			m_frames[Frame##Name] = pFrame; \
		return true; \
	} \
	bool CID3v2::isModified##Name() const { return isFrameModified( getFrame<Type>(Frame##Name) ); }

// Text Frames
#define DEF_GETTER_SETTER_TEXT(Name) DEF_GETTER_SETTER(CTextFrame3, Name)

DEF_GETTER_SETTER_TEXT(      Track);
DEF_GETTER_SETTER_TEXT(       Disc);
DEF_GETTER_SETTER_TEXT(        BPM);
DEF_GETTER_SETTER_TEXT(      Title);
DEF_GETTER_SETTER_TEXT(     Artist);
DEF_GETTER_SETTER_TEXT(      Album);
DEF_GETTER_SETTER_TEXT(AlbumArtist);
DEF_GETTER_SETTER_TEXT(       Year);

DEF_GETTER_SETTER(CCommentFrame3, Comment);

DEF_GETTER_SETTER_TEXT(   Composer);
DEF_GETTER_SETTER_TEXT(  Publisher);
DEF_GETTER_SETTER_TEXT( OrigArtist);
DEF_GETTER_SETTER_TEXT(  Copyright);

DEF_GETTER_SETTER(CURLFrame3, URL);

DEF_GETTER_SETTER_TEXT(    Encoded);

// Genre
#define GENRE_FRAME() getFrame<CGenreFrame3>(FrameGenre)

bool CID3v2::isExtendedGenre() const
{
	const CGenreFrame3* pGenre = GENRE_FRAME();
	return pGenre ? pGenre->get().isExtended() : false;
}
const std::string CID3v2::getGenre() const
{
	const CGenreFrame3* pGenre = GENRE_FRAME();
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
	const CGenreFrame3* pGenre = GENRE_FRAME();
	if(!pGenre)
		return m_strEmpty;

	const CGenre& genre = pGenre->get();
	return genre.isExtended() ? genre.str() : m_strEmpty;
}
int CID3v2::getGenreIndex() const
{
	const CGenreFrame3* pGenre = GENRE_FRAME();
	return pGenre ? pGenre->get().getIndex() : -1;
}

// Image
#define PICTURE_FRAME() getFrame<CPictureFrame3>(FramePicture)

const std::vector<uchar>& CID3v2::getPictureData() const
{
	const CPictureFrame3* pPic = PICTURE_FRAME();
	return pPic ? pPic->getData() : m_dataEmpty;
}

const std::string& CID3v2::getPictureDescription() const
{
	const CPictureFrame3* pPic = PICTURE_FRAME();
	return pPic ? pPic->getDescription() : m_strEmpty;
}

// ====================================
std::vector<std::string> CID3v2::getUnknownFrames() const
{
	std::vector<std::string> names;
	for(unknownFrames_t::const_iterator it = m_framesUnknown.begin(), end = m_framesUnknown.end();
		it != end;
		it++)
	{
		names.push_back( (*it)->getId() );
	}
	return names;
}

CID3v2::~CID3v2() { cleanup(); }

// ====================================
// Complex Routines
CID3v2* CID3v2::create()
{
	uint size = sizeof(Tag);
	uchar s[] =
	{
		uchar((size >> (24 - 3)) & 0xFF),
		uchar((size >> (16 - 2)) & 0xFF),
		uchar((size >> ( 8 - 1)) & 0xFF),
		uchar((size            ) & 0xFF)
	};

	Tag tag =
	{{
		{'I', 'D', '3'},
		0x03, 0x00,
		0x00,
		*(uint*)s
	}};
	ASSERT(tag.Header.isValid());

	return new CID3v2(tag);
}


CID3v2* CID3v2::gen(const uchar* f_pData, unsigned long long f_size, uint* f_puTagSize)
{
	ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
	if(f_size < sizeof(Tag::Header_t))
	{
		//ERROR("Too small buffer for ID3v2 tag header");
		return NULL;
	}

	const Tag& tag = *(const Tag*)f_pData;
	if( !tag.Header.isValid() )
	{
		//ERROR("Invalid ID3v2 tag header");
		return NULL;
	}
	if(f_size < tag.getSize())
	{
		//ERROR("Too small buffer for ID3v2 tag");
		return NULL;
	}
	if(tag.Header.hasFooter())
	{
		const Tag::Header_t& footer = *(const Tag::Header_t*)(f_pData +
															  sizeof(tag.Header) +
															  tag.Header.getSize());
		if( !footer.isValidFooter(tag.Header) )
		{
			ERROR("Invalid ID3v2 tag footer");
			return NULL;
		}
	}

	CID3v2* p = new CID3v2(tag);
	if(p->parse(tag))
	{
		if(f_puTagSize)
			*f_puTagSize = tag.getSize();
		return p;
	}
	else
	{
		delete p;
		return NULL;
	}
}


CID3v2::CID3v2(const Tag& f_tag):
	m_strEmpty("")
{
	// Constructor is internal and the header is already validated here
	const Tag::Header_t& h = f_tag.Header;

	// Version
	ASSERT(h.Version == 3 || h.Version == 4);
	m_version = (h.Version << 8) | h.Revision;

	// Flags: unsynchronisation (ID3v2)
	if(h.Flags & Tag::Header_t::FUnsynchronisation)
	{
		ASSERT(!"Unsynchronisation");
	}

	// Flags: extended header (ID3v2.3)
	if(h.Flags & Tag::Header_t::FExtendedHeader)
	{
		ASSERT(!"Extended header");
	}

	// Flags: experimental indicator (ID3v2.3)
	if(h.Flags & Tag::Header_t::FExperimental)
	{
		ASSERT(!"Experimental indicator");
	}

	// Flags: footer present (ID3v2.4)
	//if(h.Flags & Tag::Header_t::FFooter) {}
}


bool CID3v2::parse(const Tag& f_tag)
{
	switch(m_version >> 8)
	{
		case 3:
		case 4:
			return parse3(f_tag);
		default:
			ERROR("Unsupported ID3v2 tag version " << (m_version >> 8));
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
		const Frame3& f = *reinterpret_cast<const Frame3*>(pData);
		uint uDataSize = f.Header.getSize();
//std::cout << ">>> " << size << " | " << sizeof(f.Header) << " + " << uDataSize << std::endl;
//std::cout << f.Header.Id[0] << f.Header.Id[1] << f.Header.Id[2] << f.Header.Id[3] << std::endl;
		if(!uDataSize)
			break;

		ASSERT(size >= (int)(sizeof(f.Header) + uDataSize));
		FrameID frameID;
		CFrame3* pFrame = CFrame3::gen(f, uDataSize, (uint*)&frameID);
		if(!pFrame)
		{
			cleanup();
			return false;
		}

		if(frameID == FrameUnknown)
			m_framesUnknown.push_back(static_cast<CRawFrame3*>(pFrame));
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


void CID3v2::cleanup()
{
	for(frames_t::iterator it = m_frames.begin(), end = m_frames.end(); it != end; it++)
		delete it->second;
	m_frames.clear();

	for(uint i = 0, n = (uint)m_framesUnknown.size(); i < n; i++)
		delete m_framesUnknown[i];
	m_framesUnknown.resize(0);
}

