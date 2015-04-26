#include "id3v1.h"

#include "common.h"
#include "genre.h"

#include <cstring>


struct __attribute__ ((__packed__)) Tag
{
	char	Id[3];
	char	Title[30];
	char	Artist[30];
	char	Album[30];
	char	Year[4];
	union
	{
		char		Comment[30];
		struct
		{
			char	_Comment[28];
			uchar	_v10;
			uchar	Track;
		};
	};
	uchar	Genre;

	bool isValid() const { return (Id[0] == 'T' && Id[1] == 'A' && Id[2] == 'G'); }

	bool isV11() const { return (_v10 == 0); }
};

// Getters & Setters
enum
{
	MTitle		= 1 << 0,
	MArtist		= 1 << 1,
	MAlbum		= 1 << 2,
	MYear		= 1 << 3,
	MComment	= 1 << 4,
	MTrack		= 1 << 5,
	MGenreIndex	= 1 << 6,
};
#define MARK_MODIFIED(Name) m_maskModified |= M##Name

bool CID3v1::isV11() const { return m_v11; }
//void CID3v1::setV11(bool f_val) { m_v11 = f_val; }

#define DEF_GETTER(Type, Name, Field) \
	Type CID3v1::get##Name() const { return Field; }
#define DEF_MODIFIED(Name) \
	bool CID3v1::isModified##Name() const { return (m_maskModified & M##Name); }

#define DEF_GETTER_MODIFIED(Type, Name, Field) \
	DEF_GETTER(Type, Name, Field); \
	DEF_MODIFIED(Name)

static bool copyField(char*, const char*, uint);
#define DEF_GETTER_SETTER_MODIFIED_CHAR(Name, Field) \
	DEF_GETTER_MODIFIED(const char*, Name, Field); \
	bool CID3v1::set##Name(const char* f_ptr) \
	{ \
		MARK_MODIFIED(Name); \
		return copyField(Field, f_ptr, sizeof(Field) - 1); \
	}

// Standard
DEF_GETTER_SETTER_MODIFIED_CHAR(Title     , m_title		);
DEF_GETTER_SETTER_MODIFIED_CHAR(Artist    , m_artist	);
DEF_GETTER_SETTER_MODIFIED_CHAR(Album     , m_album		);
DEF_GETTER_SETTER_MODIFIED_CHAR(Year      , m_year		);
DEF_GETTER_SETTER_MODIFIED_CHAR(Comment   , m_comment	);

static bool set_uint8(uint* f_pField, uint f_val)
{
	if(f_val <= 0xFF)
	{
		*f_pField = f_val;
		return true;
	}
	else
	{
		*f_pField = 0;
		return false;
	}
}

// Track
uint CID3v1::getTrack() const
{
	if(isV11())
		return m_track;
	else
	{
		ERROR("not an ID3v1.1, return 0 (track)");
		return 0;
	}
}
bool CID3v1::setTrack(uint f_val)
{
	if(isV11())
	{
		MARK_MODIFIED(Track);
		return set_uint8(&m_track, f_val);
	}
	else
	{
		ERROR("not an ID3v1.1 (track)");
		return false;
	}
}
DEF_MODIFIED(Track);

// Genre
#define DEF_GETTER_SETTER_MODIFIED_UINT(Name, Field) \
	DEF_GETTER_MODIFIED(uint, GenreIndex, m_genre); \
	bool CID3v1::set##Name(uint f_val) \
	{ \
		MARK_MODIFIED(Name); \
		return set_uint8(&Field, f_val); \
	}
DEF_GETTER_SETTER_MODIFIED_UINT(GenreIndex, m_genre);
DEF_GETTER(const char*, Genre, CGenre::get(m_genre));

// ============================================================================
uint CID3v1::getSize() { return sizeof(Tag); }
CID3v1* CID3v1::create()
{
	Tag tag;
	memset(&tag, 0, sizeof(tag));
	tag.Id[0] = 'T';
	tag.Id[1] = 'A';
	tag.Id[2] = 'G';
	ASSERT(tag.isValid());

	return new CID3v1(tag);
}


CID3v1* CID3v1::gen(const uchar* f_pData, unsigned long long f_size)
{
	ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
	if(f_size < sizeof(Tag))
		return NULL;

	const Tag& tag = *(const Tag*)f_pData;
	if(!tag.isValid())
		return NULL;

	return new CID3v1(tag);
}


CID3v1::CID3v1(const Tag& f_tag):
	m_maskModified(0)
{
	ASSERT(sizeof(Tag) == 128);

	copyField(m_title  , f_tag.Title  , sizeof(m_title)    - 1);
	copyField(m_artist , f_tag.Artist , sizeof(m_artist)   - 1);
	copyField(m_album  , f_tag.Album  , sizeof(m_album)    - 1);
	copyField(m_year   , f_tag.Year   , sizeof(m_year)     - 1);
	copyField(m_comment, f_tag.Comment, sizeof(m_comment)  - 1);

	m_genre = f_tag.Genre;

	// ID3v1.1
	m_v11 = f_tag.isV11();
	m_track = m_v11 ? f_tag.Track : 0;
}


static bool copyField(char* f_dst, const char* f_src, uint f_size)
{
	uint i;
	for(i = 0; (i < f_size) && f_src[i]; i++)
		f_dst[i] = f_src[i];
	f_dst[i] = 0;
	return !f_src[i];
}

static void serializeField(char* f_dst, const char* f_src, uint f_sizeDst)
{
	uint i;
	for(i = 0; (i < f_sizeDst) && f_src[i]; i++)
		f_dst[i] = f_src[i];
	for(; i < f_sizeDst; i++)
		f_dst[i] = 0;
}


bool CID3v1::serialize(const uchar* f_pData, uint f_size, bool f_bResetModified)
{
	Tag& tag = *(Tag*)f_pData;
	if(f_size < sizeof(tag))
	{
		ERROR("Too small buffer for ID3v1 tag serialization");
		return false;
	}

	tag.Id[0] = 'T';
	tag.Id[1] = 'A';
	tag.Id[2] = 'G';

	serializeField(tag.Title , m_title , sizeof(tag.Title) );
	serializeField(tag.Artist, m_artist, sizeof(tag.Artist));
	serializeField(tag.Album , m_album , sizeof(tag.Album) );
	serializeField(tag.Year  , m_year  , sizeof(tag.Year)  );

	if(isV11())
	{
		serializeField(tag._Comment, m_comment, sizeof(tag._Comment));
		tag._v10 = 0x00;
		tag.Track = m_track & 0xFF;
	}
	else
	{
		serializeField(tag.Comment, m_comment, sizeof(tag.Comment));
		if(!tag._v10)
			tag._v10 = 0x20;
	}

	tag.Genre = m_genre & 0xFF;

	if(f_bResetModified)
		m_maskModified = 0;
	return true;
}

