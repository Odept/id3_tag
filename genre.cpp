#include "genre.h"

#include "common.h"


static const char* s_genres[] =
{
	// 0x0
	"Blues",
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

// ============================================================================
const std::string&	CGenre::str()			const { return m_genre;    }
int					CGenre::getIndex()		const { return m_indexV1;  }
bool				CGenre::isExtended()	const { return m_extended; }


CGenre::CGenre(uint f_index):
	m_genre( get(f_index) ),
	m_indexV1(f_index),
	m_extended(false)
{}

CGenre::CGenre(const std::string& f_genre):
	m_indexV1(-1),
	m_extended(false)
{
	std::string::const_iterator  it = f_genre.begin();
	std::string::const_iterator end = f_genre.end();
	if(it == end)
		return;

	// Name
	if(*it == '(')
	{
		// ID3v1 reference
		for(m_indexV1 = 0, it++; it != end; it++)
		{
			if(*it == ')')
			{
				it++;
				break;
			}
			m_indexV1 = m_indexV1 * 10 + (*it - '0');
		}
		ASSERT((uint)m_indexV1 < sizeof(s_genres) / sizeof(*s_genres));
		ASSERT(*it != '(');

		m_genre.append(it, end);
		m_extended = true;
	}
	else
		m_genre = f_genre;

	// Index
	if(m_indexV1 != -1)
		return;
	for(uint i = 0; i < sizeof(s_genres) / sizeof(*s_genres); i++)
	{
		if(m_genre.compare(s_genres[i]) != 0)
			continue;
		m_indexV1 = i;
		break;
	}
}


const char* CGenre::get(uint f_index)
{
	return (f_index < sizeof(s_genres) / sizeof(*s_genres)) ? s_genres[f_index] : "";
}

