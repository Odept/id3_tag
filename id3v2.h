#ifndef __ID3_V2_H__
#define __ID3_V2_H__

#pragma once

#include <vector>
#include <string>
#include <memory> // shared_ptr; "-std=c++0x" is required

#include "genre.h"


class CID3v2
{
private:
	typedef unsigned int	uint;
	typedef unsigned short	ushort;
	typedef unsigned char	uchar;

	struct __attribute__ ((__packed__)) Frame3
	{
		struct __attribute__ ((__packed__))
		{
			union
			{
				char Id[4];
				uint IdFourCC;
			};
			char SizeRaw[4];
			ushort Flags;
			uint getSize() const
			{
				return (SizeRaw[0] << 24) | (SizeRaw[1] << 16) | (SizeRaw[2] << 8) | SizeRaw[3];
			}
		} Header;
		const uchar Data[1];
	};

	struct __attribute__ ((__packed__)) TextFrame
	{
		uchar Encoding;
		const char RawString[1];
	};
	
public:
	CID3v2(const std::vector<uchar>& f_data);

	bool parse3(const char* f_data, uint f_size);

	bool		isValid()		const;

	uint		getVersion()	const;

	const std::string&	getTrack()		const;
	const std::string&	getDisk()		const;
	const std::string&	getTitle()		const;
	const std::string&	getArtist()		const;
	const std::string&	getAlbum()		const;
	const std::string&	getYear()		const;
	const std::string&	getGenre()		const;
	int					getGenreIndex()	const;
	//const char*	getComment()	const;

private:
	std::string parseTextFrame(const TextFrame& f_frame, uint f_uFrameSize) const;

	bool isValidFrame(const Frame3& f_frame) const;
		
private:
	typedef std::shared_ptr<CGenre>	GenrePtr;

	CID3v2();

	//void copyField(char* f_dst, const char* f_src, uint f_size);

private:
	bool m_valid;

	uint m_version;

	std::string	m_track;
	std::string	m_disk;
	std::string	m_title;
	std::string	m_artist;
	std::string	m_album;
	std::string	m_year;
	// Use shared_ptr to get rid of declaration of internal class
	GenrePtr	m_genre;
	//std::string	m_comment;

public:
	//static const uint TagSize = 128;
};

#endif // __ID3_V1_H__

