#include "id3v1.h"

#include "common.h"
#include "genre.h"


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
		for(uint i = 0; i < sizeof(Year) / sizeof(*Year); i++)
		{
			ASSERT(Year[i] >= 0 && Year[i] <= '9');
			year *= 10;
			year += Year[i] - '0';
		}
		return year;
	}
	bool isV11() const { return (_v10 == 0); }
};

// Basic Routines
bool		CID3v1::isV11()			const { return m_v11;					}

const char*	CID3v1::getTitle()		const { return m_title;					}
const char*	CID3v1::getArtist()		const { return m_artist;				}
const char*	CID3v1::getAlbum()		const { return m_album;					}
uint		CID3v1::getYear()		const { return m_year;					}
const char*	CID3v1::getComment()	const { return m_comment;				}
uint		CID3v1::getTrack()		const { return m_track;					}
//uint		CID3v1::getGenreIndex()	const { return m_genre;					}
const char*	CID3v1::getGenre()		const { return CGenre::get(m_genre);	}

// Complex Routines
CID3v1* CID3v1::gen(const uchar* f_pData, unsigned long long f_size)
{
	ASSERT(f_size < ((1ull << (sizeof(uint) * 8)) - 1));
	if(f_size < TagSize)
		return NULL;

	const Tag& tag = *(const Tag*)(f_pData + f_size - TagSize);
	if(!tag.isValid())
		return NULL;

	return new CID3v1(tag);
}


CID3v1::CID3v1(const Tag& f_tag)
{

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

