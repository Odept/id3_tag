#include "id3v1.h"

#include "common.h"
#include "genre.h"

// Basic Routines
bool		CID3v1::isValid()		const { return m_valid;		}
bool		CID3v1::isV11()			const { return m_v11;		}

const char*	CID3v1::getTitle()		const { return m_title;		}
const char*	CID3v1::getArtist()		const { return m_artist;	}
const char*	CID3v1::getAlbum()		const { return m_album;		}
uint		CID3v1::getYear()		const { return m_year;		}
const char*	CID3v1::getComment()	const { return m_comment;	}
uint		CID3v1::getTrack()		const { return m_track;		}
uint		CID3v1::getGenreIndex()	const { return m_genre;		}
const char*	CID3v1::getGenre()		const
{
	const char* szGenre = CGenre::get(m_genre);
	return szGenre ? szGenre : "";
}

// Complex Routines
CID3v1::CID3v1(const std::vector<uchar>& f_data):
	m_valid(false),
	m_v11(false),
	m_year(0),
	m_track(0),
	m_genre(0)
{
	const char* pData = (const char*)&f_data[0];

	// Check for 'TAG'
	if(f_data.size() < TagSize ||
	   pData[0] != 'T' || pData[1] != 'A' || pData[2] != 'G')
	{
		m_title[0] = m_artist[0] = m_album[0] = m_comment[0] = 0;
		return;
	}
	pData += 3;

	// Title, artist and album
	copyField(m_title , pData, 30);
	pData += 30;
	copyField(m_artist, pData, 30);
	pData += 30;
	copyField(m_album , pData, 30);
	pData += 30;

	// Year
	for(uint i = 0; i < 4; i++, pData++)
	{
		m_year *= 10;
		m_year += *pData - '0';
	}

	// Comment
	copyField(m_comment, pData, 30);
	pData += 28;

	// v1.1 and track
	if(!*pData)
	{
		m_v11 = true;
		m_track = pData[1];
	}
	pData += 2;

	// Genre
	m_genre = *pData;

	m_valid = true;
}


void CID3v1::copyField(char* f_dst, const char* f_src, uint f_size)
{
	uint i;
	for(i = 0; (i < f_size) && f_src[i]; i++)
		f_dst[i] = f_src[i];
	f_dst[i] = 0;
}

