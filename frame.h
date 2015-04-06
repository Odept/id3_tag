#ifndef __FRAME_H__
#define __FRAME_H__

#pragma once


#include "genre.h"

#include <vector>


enum Encoding
{
	EncRaw		= 0x00,	/*ISO-8859-1 (LATIN-1)*/
	EncUCS2		= 0x01,	/*UCS-2 (UTF-16, with BOM)*/
	EncUTF16BE	= 0x02,	/*UTF-16BE (without BOM, since v2.4)*/
	EncUTF8		= 0x03	/*UTF-8 (since v2.4)*/
};


struct __attribute__ ((__packed__)) Tag
{
	struct __attribute__ ((__packed__)) Header_t
	{
		char			Id[3];
		unsigned char	Version;
		unsigned char	Revision;
		unsigned char	Flags;
		unsigned int	Size;

		bool isValid() const
		{
			// Check for 'ID3'
			if(Id[0] != 'I' || Id[1] != 'D' || Id[2] != '3')
				return false;

			// Version
			if(Version == 0xFF || Revision == 0xFF)
				return false;

			// Flags::ID3v2
			if((Flags & ~0x80) && Version == 0)
				return false;

			// Flags::ID3v2.3
			if((Flags & ~0xE0) && Version <= 3)
				return false;

			// Flags::ID3v2.4
			if((Flags & ~0xF0) && Version <= 4)
				return false;

			// Size
			if(Size & 0x80808080)
				return false;

			return true;
		}

		// Size (after unsychronisation and including padding, without header)
		unsigned int getSize() const
		{
			const unsigned char* pSize = (const unsigned char*)&Size;
			return (pSize[0] << (24 - 3)) |
				   (pSize[1] << (16 - 2)) |
				   (pSize[2] << ( 8 - 1)) |
				    pSize[3];
		}
	} Header;
	unsigned char Frames[1];
};


struct __attribute__ ((__packed__)) Frame3
{
	struct __attribute__ ((__packed__))
	{
		union
		{
			char			Id[4];
			unsigned int	IdFourCC;
		};
		unsigned char		SizeRaw[4];
		unsigned short		Flags;

		unsigned int getSize() const
		{
			return (SizeRaw[0] << 24) | (SizeRaw[1] << 16) | (SizeRaw[2] << 8) | SizeRaw[3];
		}
	} Header;
	unsigned char Data[1];

	bool isValid() const
	{
		for(unsigned int i = 0; i < sizeof(Header.Id) / sizeof(Header.Id[0]); i++)
		{
			char c = Header.Id[i];
			if(c < '0' || (c > '9' && c < 'A') || c > 'Z')
				return false;
		}
		return true;
	}
};


struct __attribute__ ((__packed__)) TextFrame3
{
	unsigned char	Encoding;
	char			RawString[1];
};


struct __attribute__ ((__packed__)) CommentFrame3
{
	unsigned char	Encoding;
	unsigned char	Language[3];
	char			RawShortString[1];
	//char			RawFullString[1];
};

// ============================================================================
class CFrame3
{
private:
	typedef unsigned int uint;

public:
	static CFrame3* gen(const Frame3& f_frame, uint f_uDataSize, uint* pFrameID);

public:
	virtual ~CFrame3() {}

protected:
	CFrame3() {}
};


class CRawFrame3 : public CFrame3
{
	friend class CFrame3;

public:
	virtual ~CRawFrame3() {}
	const std::string& getId() const { return m_id; }

protected:
	CRawFrame3(const Frame3& f_frame);

protected:
	std::vector<char> m_frame;
	std::string m_id;
};


class CTextFrame3 : public CFrame3
{
	friend class CFrame3;

private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	static CTextFrame3* create();

public:
	const std::string& get() const { return m_text; }

	CTextFrame3& operator=(const std::string& f_val);

	virtual ~CTextFrame3() {}

protected:
	CTextFrame3(const TextFrame3& f_frame, uint f_uFrameSize);

protected:
	Encoding	m_encodingRaw;
	std::string	m_text;
};


class CGenreFrame3 : public CTextFrame3
{
	friend class CFrame3;

private:
	typedef unsigned int uint;

public:
	const CGenre& get() const { return m_genre; }

	virtual ~CGenreFrame3() {}

protected:
	CGenreFrame3(const TextFrame3& f_frame, uint f_uFrameSize):
		CTextFrame3(f_frame, f_uFrameSize),
		m_genre(m_text)
	{}

protected:
	CGenre m_genre;
};


class CCommentFrame3 : public CFrame3
{
	friend class CFrame3;

private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	//static CTextFrame3* create();

public:
	const std::string& getShort() const { return m_short; }
	const std::string& getFull () const { return m_full ; }

	//CTextFrame3& operator=(const std::string& f_val);

	virtual ~CCommentFrame3() {}

protected:
	CCommentFrame3(const CommentFrame3& f_frame, uint f_uFrameSize);

private:
	template<typename T>
	void fill(const T* f_data, uint f_size, uint f_step = sizeof(T));

protected:
	Encoding	m_encodingRaw;
	uchar		m_lang[3];
	std::string	m_short;
	std::string	m_full;
};

#endif // __FRAME_H__

