#include "id3v1.h"

// Basic Routines
bool		CID3v1::isValid()		const { return m_valid;   }
bool		CID3v1::isV11()			const { return m_v11;     }

const char*	CID3v1::getTitle()		const { return m_title;   }
const char*	CID3v1::getArtist()		const { return m_artist;  }
const char*	CID3v1::getAlbum()		const { return m_album;   }
uint		CID3v1::getYear()		const { return m_year;    }
const char*	CID3v1::getComment()	const { return m_comment; }
uint		CID3v1::getTrack()		const { return m_track;   }
uint		CID3v1::getGenreIndex()	const { return m_genre;   }
const char*	CID3v1::getGenre()		const
{
	return (m_genre < (sizeof(m_genres) / sizeof(*m_genres))) ? m_genres[m_genre] : NULL;
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
	if(pData[0] != 'T' || pData[1] != 'A' || pData[2] != 'G')
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

// Genres
const char* CID3v1::m_genres[] =
{
	// 0x0
	"Classic Rock",
	"Country",
	"Dance",
	"Disco",
	"Funk",
	"Grunge",
	"Hip-Hop",
	"Jazz",
	"Metal",
	"New Age",
	"Oldies",
	"Other",
	"Pop",
	"Rhythm & Blues",
	"Rap",
	// 0x10
	"Reggae",
	"Rock",
	"Techno",
	"Industrial",
	"Alternative",
	"Ska",
	"Death Metal",
	"Pranks",
	"Soundtrack",
	"Euro-Techno",
	"Ambient",
	"Trip-Hop",
	"Vocal",
	"Jazz & Funk",
	"Fusion",
	"Trance",
	// 0x20
	"Classical",
	"Instrumental",
	"Acid",
	"House",
	"Game",
	"Sound Clip",
	"Gospel",
	"Noise",
	"Alternative Rock",
	"Bass",
	"Soul",
	"Punk Rock",
	"Space",
	"Meditative",
	"Instrumental Pop",
	"Instrumental Rock",
	// 0x30
	"Ethnic",
	"Gothic",
	"Darkwave",
	"Techno-Industrial",
	"Electronic",
	"Pop-Folk",
	"Eurodance",
	"Dream",
	"Southern Rock",
	"Comedy",
	"Cult",
	"Gangsta",
	"Top 40",
	"Christian Rap",
	"Pop/Funk",
	"Jungle",
	// 0x40
	"Native American",
	"Cabaret",
	"New Wave",
	"Psychedelic",
	"Rave",
	"Showtunes",
	"Trailer",
	"Lo-Fi",
	"Tribal",
	"Acid Punk",
	"Acid Jazz",
	"Polka",
	"Retro",
	"Musical",
	"Rock & Roll",
	"Hard Rock",
	// 0x50 (Winamp Extensions)
	"Folk",
	"Folk-Rock",
	"National Folk",
	"Swing",
	"Fast Fusion",
	"Bebop",
	"Latin",
	"Revival",
	"Celtic",
	"Bluegrass",
	"Avantgarde",
	"Gothic Rock",
	"Progressive Rock",
	"Psychedelic Rock",
	"Symphonic Rock",
	"Slow Rock",
	// 0x60
	"Big Band",
	"Chorus",
	"Easy Listening",
	"Acoustic",
	"Humour",
	"Speech",
	"Chanson",
	"Opera",
	"Chamber Music",
	"Sonata",
	"Symphony",
	"Booty Bass",
	"Primus",
	"Porn groove",
	"Satire",
	"Slow Jam",
	// 0x70
	"Club",
	"Tango",
	"Samba",
	"Folklore",
	"Ballad",
	"Power Ballad",
	"Rhythmic Soul",
	"Freestyle",
	"Duet",
	"Punk rock",
	"Drum Solo",
	"A Capella",
	"Euro-House",
	"Dance Hall",
	"Goa Trance",
	"Drum & Bass",
	// 0x80
	"Club-House",
	"Hardcore Techno",
	"Terror",
	"Indie",
	"BritPop",
	"Afro-Punk",
	"Polsk Punk",
	"Beat",
	"Christian Gangsta Rap",
	"Heavy Metal",
	"Black Metal",
	"Crossover",
	"Contemporary Christian",
	"Christian Rock",
	// Winamp >= v1.91
	"Merengue",
	"Salsa",
	// 0xA0
	"Thrash Metal",
	"Anime",
	"Jpop",
	"Synthpop",
	// Winamp > v5
	"Abstract",
	"Art Rock",
	"Baroque",
	"Bhangra",
	"Big Beat",
	"Breakbeat",
	"Chillout",
	"Downtempo",
	"Dub",
	"EBM",
	"Eclectic",
	"Electro",
	// 0xB0
	"Electroclash",
	"Emo",
	"Experimental",
	"Garage",
	"Global",
	"IDM",
	"Illbient",
	"Industro-Goth",
	"Jam Band",
	"Krautrock",
	"Leftfield",
	"Lounge",
	"Math Rock",
	"New Romantic",
	"Nu-Breakz",
	"Post-Punk",
	// 0xC0
	"Post-Rock",
	"Psytrance",
	"Shoegaze",
	"Space Rock",
	"Trop Rock",
	"World Music",
	"Neoclassical",
	"Audiobook",
	"Audio Theatre",
	"Neue Deutsche Welle",
	"Podcast",
	"Indie Rock",
	"G-Funk",
	"Dubstep",
	"Garage Rock",
	"Psybient"
	// 0xD0
};

