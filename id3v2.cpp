#include "id3v2.h"

#include "common.h"
#include "frame.h"


// Getters/Setters
bool CID3v2::isExtendedGenre(unsigned f_index) const
{
	auto it = m_frames.find(FrameGenre);
	if(it == m_frames.end())
		throw std::out_of_range(__FUNCTION__);

	auto& vec = it->second;
	return frame_cast<CGenreFrame3>(vec.at(f_index))->isExtended();
}


std::vector<std::string> CID3v2::getUnknownFrames() const
{
	std::vector<std::string> names;
	for(auto it = m_framesUnknown.cbegin(), end = m_framesUnknown.cend(); it != end; ++it)
		names.push_back( (*it)->getId() );
	return names;
}

// ====================================
CID3v2::CID3v2(const uchar* f_data, size_t f_size):
	m_tag(f_size),
	m_modified(false)
{
	auto& header = reinterpret_cast<const CID3v2::Tag_t*>(f_data)->Header;

	// Version
	ASSERT(header.Version == 3 || header.Version == 4);
	m_ver_minor = header.Version;
	m_ver_revision = header.Revision;

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

	parse();
}


void CID3v2::parse()
{
	switch(m_ver_minor)
	{
		case 3:
		case 4:
			parse3();
			break;
		default:
			ASSERT_MSG(!"Unsupported ID3v2 tag version", std::to_string(m_ver_minor));
	}
}


static std::shared_ptr<CFrame3> createFrame(FrameType f_type, const Frame3& f_frame)
{
	switch(f_type)
	{
		case FrameGenre:	return std::make_shared<CGenreFrame3>	(f_frame);
		case FrameComment:	return std::make_shared<CCommentFrame3>	(f_frame);
		case FrameMMJB:		return std::make_shared<CMMJBFrame3>	(f_frame);
		case FrameURL:		return std::make_shared<CURLFrame3>		(f_frame);
		case FramePicture:	return std::make_shared<CPictureFrame3>	(f_frame);
		case FrameUnknown:	return std::make_shared<CRawFrame3>		(f_frame);

		default:			return std::make_shared<CTextFrame3>	(f_frame);
	}
}

void CID3v2::parse3()
{
	auto& tag = *reinterpret_cast<const Tag_t*>(&m_tag[0]);
	auto tagSize = m_tag.size();

	// The size of a ID3v2 tag is limited to 256 MB
	const uchar* pData;
	size_t size = tag.Header.size();
	ASSERT(tagSize == sizeof(tag.Header) + tag.Header.size());

	for(pData = static_cast<const uchar*>(tag.Frames); size >= sizeof(Frame3::Header);)
	{
		auto& f = *reinterpret_cast<const Frame3*>(pData);
		if(!f.Header.isValid())
			break;

		auto frameSize = f.Header.size();
		ASSERT(sizeof(f.Header) + frameSize <= size);

		// Get frame type
		FrameType frameType = CFrame3::getFrameType(f.Header);
		std::shared_ptr<CFrame3> frame;

		// Create frame
		for(auto bRetry = true; bRetry;)
		{
			try
			{
				frame = createFrame(frameType, f);
				bRetry = false;
			}
			catch(const CCommentFrame3::ExceptionMMJB&)
			{
				frameType = FrameMMJB;
			}
		}

		// Store frame
		switch(frameType)
		{
			case FrameMMJB:
				m_framesMMJB.push_back( frame_cast<CMMJBFrame3>(frame) );
				break;

			case FrameUnknown:
				m_framesUnknown.push_back( frame_cast<CRawFrame3>(frame) );
				break;

			default:
				m_frames[frameType].push_back(frame);
		}

		// Next
		pData += sizeof(f.Header) + frameSize;
		size  -= sizeof(f.Header) + frameSize;
	}

	// Validate tail
	for(; size; --size, ++pData)
		ASSERT(*pData == 0x00);
}


void CID3v2::serialize(std::vector<uchar>& f_outStream)
{
	ASSERT(!m_modified);
	f_outStream.insert(f_outStream.end(), m_tag.begin(), m_tag.end());
}

// ====================================
namespace Tag
{
	size_t IID3v2::getSize(const unsigned char* f_data, size_t f_size)
	{
		if(f_size < sizeof(CID3v2::Tag_t::Header_t))
			return 0;

		auto& tag = *reinterpret_cast<const CID3v2::Tag_t*>(f_data);
		if( !tag.Header.isValid() )
			return 0;

		auto tagSize = tag.getSize();
		if(f_size < tagSize)
			return 0;

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


	std::shared_ptr<IID3v2> IID3v2::create(const unsigned char* f_data, size_t f_size)
	{
		return std::make_shared<CID3v2>(f_data, f_size);
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

