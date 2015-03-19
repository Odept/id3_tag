#ifndef __ID3_V1_H__
#define __ID3_V1_H__

#pragma once

#include <vector>


class CID3v1
{
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	CID3v1(const std::vector<uchar>& f_data);

	bool		isValid()		const;
	bool		isV11()			const;

	const char*	getTitle()		const;
	const char*	getArtist()		const;
	const char*	getAlbum()		const;
	uint		getYear()		const;
	const char*	getComment()	const;
	uint		getTrack()		const;
	//uint		getGenreIndex()	const;
	const char*	getGenre()		const;
		
private:
	CID3v1();

	void copyField(char* f_dst, const char* f_src, uint f_size);

private:
	bool m_valid;
	bool m_v11;

	char m_title[31];
	char m_artist[31];
	char m_album[31];
	uint m_year;
	char m_comment[31];
	uint m_track;
	uint m_genre;

public:
	static const uint TagSize = 128;

private:
	static const char* m_genres[0xD0];
};

#endif // __ID3_V1_H__

