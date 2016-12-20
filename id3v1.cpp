#include "id3v1.h"


CID3v1::CID3v1(const Tag_t& f_tag):
	m_v11		(f_tag.isV11()),
#define INIT_FIELD(FieldName, TagName)	FieldName(f_tag.TagName, strnlen(f_tag.TagName, sizeof(f_tag.TagName)))
	INIT_FIELD	(m_title	, Title),
	INIT_FIELD	(m_artist	, Artist),
	INIT_FIELD	(m_album	, Album),
	INIT_FIELD	(m_year		, Year),
#undef INIT_FIELD
	m_comment	(f_tag.Comment, strnlen(f_tag.Comment, m_v11 ? sizeof(f_tag.Comment11) : sizeof(f_tag.Comment))),
	m_track		(m_v11 ? f_tag.Track : 0),
	m_genre		(f_tag.Genre),
	m_maskModified(0),
	m_tag(f_tag)
{}


void CID3v1::serialize(std::vector<unsigned char>& f_outStream)
{
	flushChanges();
	ASSERT(!m_maskModified);

	ASSERT(m_tag.isValid());
	auto offset = f_outStream.size();
	f_outStream.resize(offset + sizeof(m_tag.Raw));
	memcpy(&f_outStream[offset], m_tag.Raw, sizeof(m_tag.Raw));
}

void CID3v1::flushChanges()
{
	ASSERT(!"Not tested");
#define IS_MODIFIED(Name)		(m_maskModified & static_cast<uint>(ModMask::Name))
#define CLEAR_MODIFIED(Name)	m_maskModified &= ~static_cast<uint>(ModMask::Name)
#define SYNC_FIELD(Name, Field) \
	if(IS_MODIFIED(Name)) \
	{ \
		len = Field.length(); \
		if(len > sizeof(Tag_t::Name)) \
		{ \
			WARNING("The length of "#Name" (" << Field.length() << ") exceeds " << sizeof(Tag_t::Name) << " characters - the string will be truncated"); \
			len = sizeof(Tag_t::Name); \
		} \
		memcpy(m_tag.Name, Field.c_str(), len); \
		CLEAR_MODIFIED(Name); \
	}

	size_t len;

	SYNC_FIELD(Title	, m_title);
	SYNC_FIELD(Artist	, m_artist);
	SYNC_FIELD(Album	, m_album);
	SYNC_FIELD(Year		, m_year);

	m_tag.v10 = isV11() ? 0 : 1;
	if(isV11())
	{
		SYNC_FIELD(Comment11, m_comment);

		if(IS_MODIFIED(Track))
		{
			ASSERT(isUint8(m_track));
			m_tag.Track = m_track;
			CLEAR_MODIFIED(Track);
		}
	}
	else
		SYNC_FIELD(Comment, m_comment);

	if(IS_MODIFIED(Genre))
	{
		ASSERT(isUint8(m_genre));
		m_tag.Genre = m_genre;
		CLEAR_MODIFIED(Genre);
	}
#undef SYNC_FIELD
#undef CLEAR_MODIFIED
#undef IS_MODIFIED
}

// ====================================
namespace Tag
{
	size_t IID3v1::size() { return sizeof(CID3v1::Tag_t); }

	size_t IID3v1::getSize(const unsigned char* f_data, size_t f_size)
	{
		if(f_size < sizeof(CID3v1::Tag_t))
			return 0;

		auto& tag = *reinterpret_cast<const CID3v1::Tag_t*>(f_data);
		return tag.isValid() ? sizeof(CID3v1::Tag_t) : 0;
	}


	std::shared_ptr<IID3v1> IID3v1::create(const unsigned char* f_data, size_t f_size)
	{
		ASSERT(f_size == sizeof(CID3v1::Tag_t));

		auto& tag = *reinterpret_cast<const CID3v1::Tag_t*>(f_data);
		ASSERT(tag.isValid());
		return std::make_shared<CID3v1>(tag);
	}

	// Creates an empty tag
	std::shared_ptr<IID3v1> IID3v1::create()
	{
		CID3v1::Tag_t tag;
		memset(&tag, 0, sizeof(tag));
		tag.Id[0] = 'T';
		tag.Id[1] = 'A';
		tag.Id[2] = 'G';
		ASSERT(tag.isValid());

		return std::make_shared<CID3v1>(tag);
	}
}

