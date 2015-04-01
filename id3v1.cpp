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

	uint getYear() const
	{
		uint year = 0;
		for(uint i = 0; (i < sizeof(Year) / sizeof(*Year)) && Year[i]; i++)
		{
			ASSERT(Year[i] >= 0 && Year[i] <= '9');
			year *= 10;
			year += Year[i] - '0';
		}
		return year;
	}
	bool isV11() const { return (_v10 == 0); }
};

// Getters & Setters
bool CID3v1::isV11() const { return m_v11; }
void CID3v1::setV11(bool f_val) { m_v11 = f_val; }

#define DEF_GETTER(Type, Name, Field) \
	const Type CID3v1::get##Name() const { return Field; }

#define DEF_GETTER_SETTER_UINT(Name, Field) \
	DEF_GETTER(uint, Name, Field); \
	void CID3v1::set##Name(uint f_val) { Field = f_val; }

#define DEF_GETTER_SETTER_CHAR(Name, Field) \
	DEF_GETTER(char*, Name, Field); \
	void CID3v1::set##Name(const char* f_ptr) { copyField(Field, f_ptr, sizeof(Field) - 1); }

DEF_GETTER_SETTER_CHAR(Title     , m_title  );
DEF_GETTER_SETTER_CHAR(Artist    , m_artist );
DEF_GETTER_SETTER_CHAR(Album     , m_album  );
DEF_GETTER_SETTER_UINT(Year      , m_year   );
DEF_GETTER_SETTER_CHAR(Comment   , m_comment);
DEF_GETTER_SETTER_UINT(Track     , m_track  );
DEF_GETTER_SETTER_UINT(GenreIndex, m_genre  );

DEF_GETTER(char*, Genre, CGenre::get(m_genre));

// ============================================================================
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

	const Tag& tag = *(const Tag*)(f_pData + f_size - sizeof(Tag));
	if(!tag.isValid())
		return NULL;

	return new CID3v1(tag);
}


CID3v1::CID3v1(const Tag& f_tag)
{
	ASSERT(sizeof(Tag) == Size);

	copyField(m_title  , f_tag.Title  , sizeof(m_title)  - 1);
	copyField(m_artist , f_tag.Artist , sizeof(m_artist) - 1);
	copyField(m_album  , f_tag.Album  , sizeof(m_album)  - 1);
	copyField(m_comment, f_tag.Comment, sizeof(m_comment)  - 1);

	m_year = f_tag.getYear();
	m_genre = f_tag.Genre;

	// ID3v1.1
	m_v11 = f_tag.isV11();
	m_track = m_v11 ? f_tag.Track : 0;
}


void CID3v1::copyField(char* f_dst, const char* f_src, uint f_size)
{
	uint i;
	for(i = 0; (i < f_size) && f_src[i]; i++)
		f_dst[i] = f_src[i];
	f_dst[i] = 0;
}

void CID3v1::serializeField(char* f_dst, const char* f_src, uint f_sizeDst) const
{
	uint i;
	for(i = 0; (i < f_sizeDst) && f_src[i]; i++)
		f_dst[i] = f_src[i];
	for(; i < f_sizeDst; i++)
		f_dst[i] = 0;
}


bool CID3v1::serialize(const uchar* f_pData, uint f_size) const
{
	Tag& tag = *(Tag*)f_pData;
	if(f_size < sizeof(tag))
		return false;

	tag.Id[0] = 'T';
	tag.Id[1] = 'A';
	tag.Id[2] = 'G';

	serializeField(tag.Title , m_title , sizeof(tag.Title) );
	serializeField(tag.Artist, m_artist, sizeof(tag.Artist));
	serializeField(tag.Album , m_album , sizeof(tag.Album) );

	uint year = m_year % 10000;
	tag.Year[0] = year / 1000;
	year %= 1000;
	tag.Year[1] = year / 100;
	year %= 100;
	tag.Year[2] = year / 10;
	year %= 10;
	tag.Year[3] = year;

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

	return true;
}

