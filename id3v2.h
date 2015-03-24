#ifndef __ID3_V2_H__
#define __ID3_V2_H__

#pragma once

#include <vector>
#include <string>
#include <memory> // shared_ptr; "-std=c++0x" is required

#include "genre.h"


struct Frame3;
struct TextFrame3;


class CFrame3
{
	friend class CRawFrame3;
	friend class CTextFrame3;
	friend class CGenreFrame3;

private:
	typedef unsigned int uint;

public:
	static CFrame3* gen(const Frame3& f_frame, uint f_uDataSize);

public:
	~CFrame3() {}

private:
	CFrame3() {}
};


class CRawFrame3 : public CFrame3
{
	friend class CFrame3;

public:
	~CRawFrame3() {}

protected:
	CRawFrame3(const Frame3& f_frame);

protected:
	std::vector<char> m_frame;
};


class CTextFrame3 : public CFrame3
{
	friend class CFrame3;

private:
	typedef unsigned int uint;

public:
	const std::string& get() const;

	~CTextFrame3() {}

protected:
	CTextFrame3(const TextFrame3& f_frame, uint f_uFrameSize);

protected:
	std::string m_text;
};


class CGenreFrame3 : public CTextFrame3
{
	friend class CFrame3;

private:
	typedef unsigned int uint;

public:
	const CGenre& get() const;

	~CGenreFrame3() {}

protected:
	CGenreFrame3(const TextFrame3& f_frame, uint f_uFrameSize);

protected:
	CGenre m_genre;
};


class CID3v2
{
private:
	typedef unsigned int	uint;
	typedef unsigned short	ushort;
	typedef unsigned char	uchar;
	
public:
	CID3v2(const std::vector<uchar>& f_data);
	~CID3v2();

	bool parse3(const char* f_data, uint f_size);

	bool		isValid()		const;

	uint		getVersion()	const;

	const std::string&	getTrack()			const;
	const std::string&	getDisc()			const;
	const std::string&	getBPM()			const;

	const std::string&	getTitle()			const;
	const std::string&	getArtist()			const;
	const std::string&	getAlbum()			const;
	const std::string&	getYear()			const;
	const std::string&	getAlbumArtist()	const;

	bool				isExtendedGenre()	const;
	const std::string	getGenre()			const;
	const std::string&	getGenreEx()		const;
	int					getGenreIndex()		const;

	//const std::string&	getComment()		const;

	const std::string&	getComposer()		const;
	const std::string&	getPublisher()		const;
	const std::string&	getOArtist()		const;
	const std::string&	getCopyright()		const;
	//const std::string&	getURL()			const;
	const std::string&	getEncoded()		const;

private:
	//typedef std::shared_ptr<CGenre>	GenrePtr;

	CID3v2();

	//void copyField(char* f_dst, const char* f_src, uint f_size);
	void cleanup();

	bool isValidIndex(int f_index) const;
	const std::string& strTextFrame(int f_index) const;

private:
	bool m_valid;

	uint m_version;

	int m_iFrame;
	std::vector<CFrame3*> m_frames;

	int m_iTrack;
	int m_iDisc;
	int	m_iBPM;
	int m_iTitle;
	int m_iArtist;
	int m_iAlbum;
	int m_iAArtist;
	int m_iYear;
	int m_iGenre;
	//int m_iComment;
	int m_iComposer;
	int m_iPublisher;
	int m_iOArtist;
	int m_iCopyright;
	//int m_iURL;
	int m_iEncoded;

	std::string m_strEmpty;

public:
	//static const uint TagSize = 128;
};

#endif // __ID3_V1_H__

