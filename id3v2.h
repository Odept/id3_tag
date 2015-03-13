#ifndef __ID3_V2_H__
#define __ID3_V2_H__

#pragma once

#include <vector>
#include <string>


class CID3v2
{
private:
	typedef unsigned int	uint;
	typedef unsigned short	ushort;
	typedef unsigned char	uchar;

	struct __attribute__ ((__packed__)) Tag3
	{
		struct
		{
			uint Id;
			uint Size;
			ushort Flags;
		} Header;
		const uchar Data[1];
	};
	
public:
	CID3v2(const std::vector<uchar>& f_data);

	std::string parseTextFrame(const Tag3& f_tag);

	bool parse3(const char* f_data, uint f_size);

	bool		isValid()		const;

	bool		getVersion()	const;
	//const char*	getTitle()		const;
	//const char*	getArtist()		const;
	//const char*	getAlbum()		const;
	//uint		getYear()		const;
	//const char*	getComment()	const;
	//uint		getTrack()		const;
	//uint		getGenreIndex()	const;
	//const char*	getGenre()		const;
		
private:
	CID3v2();

	//void copyField(char* f_dst, const char* f_src, uint f_size);

private:
	bool m_valid;
	bool m_version;

	std::string m_track;
	//char m_title[31];
	//char m_artist[31];
	//char m_album[31];
	//uint m_year;
	//char m_comment[31];
	//uint m_track;
	//uint m_genre;

public:
	//static const uint TagSize = 128;

private:
	//static const char* m_genres[0xD0];
};

#endif // __ID3_V1_H__

