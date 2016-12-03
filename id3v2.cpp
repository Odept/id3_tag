#include "id3v2.h"

#include "common.h"
#include "genre.h"
#include "frame.h"


// Getters/Setters

//static bool isFrameModified(const CFrame3* f_pFrame) { return f_pFrame ? f_pFrame->isModified() : false; }
//#define DEF_MODIFIED(Type, Name) \
//	bool CID3v2::isModified##Name() const { return isFrameModified( getFrame<Type>(Frame##Name) ); }

// Genre
#define GENRE_FRAME() getFrame<CGenreFrame3>(FrameGenre)

const std::string CID3v2::getGenre() const
{
	const CGenreFrame3* pGenre = GENRE_FRAME();
	if(!pGenre)
		return m_strEmpty;
	int i = pGenre->getIndex();
	return (i == -1) ? pGenre->get() : ::genre(i);
}
const std::string& CID3v2::getGenreEx() const
{
	const CGenreFrame3* pGenre = GENRE_FRAME();
	if(!pGenre)
		return m_strEmpty;
	return pGenre->isExtended() ? pGenre->get() : m_strEmpty;
}
bool CID3v2::isExtendedGenre() const
{
	const CGenreFrame3* pGenre = GENRE_FRAME();
	return pGenre ? pGenre->isExtended() : false;
}

void CID3v2::setGenre(const std::string& f_text)
{
	int index = ::genreIndex(f_text);
	if(index != -1)
		setGenre(index);

	CGenreFrame3* pFrame = GENRE_FRAME();
	if(pFrame)
		*pFrame = f_text;
	else
	{
		pFrame = new CGenreFrame3(f_text);
		m_frames[FrameGenre] = pFrame;
	}
}
void CID3v2::setGenre(uint f_index)
{
	CGenreFrame3* pFrame = GENRE_FRAME();
	if(pFrame)
		*pFrame = f_index;
	else
	{
		pFrame = new CGenreFrame3(f_index);
		m_frames[FrameGenre] = pFrame;
	}
}
//DEF_MODIFIED(CTextFrame3, Genre);

// Image
#define PICTURE_FRAME() getFrame<CPictureFrame3>(FramePicture)

const std::vector<uchar>& CID3v2::getPictureData() const
{
	static const std::vector<uchar> m_dataEmpty;
	const CPictureFrame3* pPic = PICTURE_FRAME();
	return pPic ? pPic->getData() : m_dataEmpty;
}

const std::string& CID3v2::getPictureDescription() const
{
	const CPictureFrame3* pPic = PICTURE_FRAME();
	return pPic ? pPic->getDescription() : m_strEmpty;
}


std::vector<std::string> CID3v2::getUnknownFrames() const
{
	std::vector<std::string> names;
	for(auto it = m_framesUnknown.begin(), end = m_framesUnknown.end(); it != end; ++it)
		names.push_back( (*it)->getId() );
	return names;
}

// ====================================
CID3v2::CID3v2(const uchar* f_data, size_t f_size):
	m_strEmpty(""),
	m_tag(f_size),
	m_modified(false)
{
	auto& header = reinterpret_cast<const CID3v2::Tag_t*>(f_data)->Header;

	// Version
	ASSERT(header.Version == 3 || header.Version == 4);
	m_version = (header.Version << 8) | header.Revision;

	// Flags: unsynchronisation (ID3v2)
	if(header.Flags & Tag_t::Header_t::FUnsynchronisation)
	{
		ASSERT(!"Unsynchronisation");
	}

	// Flags: extended header (ID3v2.3)
	if(header.Flags & Tag_t::Header_t::FExtendedHeader)
	{
		ASSERT(!"Extended header");
	}

	// Flags: experimental indicator (ID3v2.3)
	if(header.Flags & Tag_t::Header_t::FExperimental)
	{
		ASSERT(!"Experimental indicator");
	}

	// Flags: footer present (ID3v2.4)
	if(header.Flags & Tag_t::Header_t::FFooter)
	{
		ASSERT(!"Untested/unknown");
	}

	memcpy(&m_tag[0], f_data, f_size);
}


bool CID3v2::parse()
{
	switch(m_version >> 8)
	{
		case 3:
		case 4:
			return parse3();
		default:
			ERROR("Unsupported ID3v2 tag version " << (m_version >> 8));
			return false;
	}
}


bool CID3v2::parse3()
{
	auto& tag = *reinterpret_cast<const Tag_t*>(&m_tag[0]);
	auto tagSize = m_tag.size();

	// The size of a ID3v2 tag is limited to 256 MB
	const uchar* pData;
	size_t size = tag.Header.size();
	ASSERT(tagSize == sizeof(tag.Header) + tag.Header.size());

	for(pData = static_cast<const uchar*>(tag.Frames); size >= sizeof(Frame3);)
	{
		auto& f = *reinterpret_cast<const Frame3*>(pData);
		auto frameSize = f.Header.getSize();
//std::cout << ">>> " << size << " | " << sizeof(f.Header) << " + " << frameSize << std::endl;
//std::cout << f.Header.Id[0] << f.Header.Id[1] << f.Header.Id[2] << f.Header.Id[3] << std::endl;
		if(!frameSize)
			break;

		ASSERT(size >= sizeof(f.Header) + frameSize);
		FrameID frameID;
		CFrame3* pFrame = CFrame3::gen(f, frameSize, (uint*)&frameID);
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

		pData += sizeof(f.Header) + frameSize;
		size  -= sizeof(f.Header) + frameSize;
	}

	// Validate tail
	for(; size; --size, ++pData)
		ASSERT(*pData == 0x00);

	return true;
}


void CID3v2::cleanup()
{
	for(auto it = m_frames.begin(), end = m_frames.end(); it != end; ++it)
		delete it->second;
	m_frames.clear();

	for(size_t i = 0, n = m_framesUnknown.size(); i < n; ++i)
		delete m_framesUnknown[i];
	m_framesUnknown.resize(0);
}


void CID3v2::serialize(std::vector<uchar>& f_outStream)
{
	ASSERT(!m_modified);
	f_outStream.insert(f_outStream.end(), m_tag.begin(), m_tag.end());
}

// ====================================
namespace Tag
{
	IID3v2::TagSize IID3v2::getSize(const unsigned char* f_data, size_t f_size)
	{
		if(f_size < sizeof(CID3v2::Tag_t::Header_t))
		{
			//ERROR("Too small buffer for ID3v2 tag header");
			return 0;
		}

		auto& tag = *reinterpret_cast<const CID3v2::Tag_t*>(f_data);
		if( !tag.Header.isValid() )
		{
			//ERROR("Invalid ID3v2 tag header");
			return 0;
		}

		TagSize tagSize = tag.getSize();
		if(f_size < tagSize)
		{
			//ERROR("Too small buffer for ID3v2 tag");
			return 0;
		}

		if(tag.Header.hasFooter())
		{
			auto& footer = *reinterpret_cast<const CID3v2::Tag_t::Header_t*>(f_data + sizeof(tag.Header) + tag.Header.size());
			ASSERT(!"Requires verification");
			// tag.Header ??? (should be footer)
			if( !footer.isValidFooter(tag.Header) )
			{
				ASSERT(!"Invalid ID3v2 tag footer");
				return 0;
			}
		}

		return tagSize;
	}


	std::shared_ptr<IID3v2> IID3v2::create(const unsigned char* f_data, TagSize f_size)
	{
		auto sp = std::make_shared<CID3v2>(f_data, f_size);
		if(!sp->parse())
			sp.reset();
		return sp;
	}

	// Creates an empty tag
	std::shared_ptr<IID3v2> IID3v2::create()
	{
		uint size = sizeof(CID3v2::Tag_t);
		uchar s[] =
		{
			uchar((size >> (24 - 3)) & 0xFF),
			uchar((size >> (16 - 2)) & 0xFF),
			uchar((size >> ( 8 - 1)) & 0xFF),
			uchar((size            ) & 0xFF)
		};

		CID3v2::Tag_t tag =
		{
			{
				{'I', 'D', '3'},
				0x03, 0x00,
				0x00,
				*reinterpret_cast<uint*>(s)
			}
		};
		ASSERT(tag.Header.isValid());

		ASSERT(!"Check if size correct");
		return std::make_shared<CID3v2>(reinterpret_cast<uchar*>(&tag), tag.getSize());
	}
}

