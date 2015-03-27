#ifndef __ID3_V1_H__
#define __ID3_V1_H__

#pragma once


class Tag;


class CID3v1
{
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	static CID3v1* gen(const uchar* f_pData, unsigned long long f_size);

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
	CID3v1(const Tag& f_tag);
	CID3v1();

	void copyField(char* f_dst, const char* f_src, uint f_size);

private:
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
};

#endif // __ID3_V1_H__

