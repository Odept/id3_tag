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
		enum
		{
			FUnsynchronisation	= 0x80,
			FMaskV0				= FUnsynchronisation,

			FExtendedHeader		= 0x40,
			FExperimental		= 0x20,
			FMaskV3				= FMaskV0 | FExtendedHeader | FExperimental,

			FFooter				= 0x10,
			FMaskV4				= FMaskV3 | FFooter
		};

		char			Id[3];
		unsigned char	Version;
		unsigned char	Revision;
		unsigned char	Flags;
		unsigned int	Size;

		bool isValid() const
		{
			if(Id[0] != 'I' || Id[1] != 'D' || Id[2] != '3')
				return false;

			if(Version == 0xFF || Revision == 0xFF)
				return false;

			if((Flags & ~FMaskV0) && Version == 0)
				return false;
			if((Flags & ~FMaskV3) && Version <= 3)
				return false;
			if((Flags & ~FMaskV4) && Version <= 4)
				return false;

			if(Size & 0x80808080)
				return false;

			return true;
		}

		bool hasFooter() const { return Flags & FFooter; }
		bool isValidFooter(const Header_t& f_header) const
		{
			return (Id[0] == '3' &&
					Id[1] == 'D' &&
					Id[2] == 'I' &&
					Version	== f_header.Version	&&
					Flags	== f_header.Flags	&&
					Size	== f_header.Size);
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

	unsigned int getSize() const
	{
		return (sizeof(Header) +
				Header.getSize() +
				Header.hasFooter() * sizeof(Header));
	}
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


struct __attribute__ ((__packed__)) URLFrame3
{
	unsigned char	Encoding;
	char			Description[1];
	//char			URL[1];
};


struct __attribute__ ((__packed__)) PictureFrame3
{
	unsigned char	Encoding;
	char			MIME[1];
};

// ============================================================================
class CFrame3
{
protected:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

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

public:
	//static CCommentFrame3* create();

public:
	const std::string& getShort() const { return m_short; }
	const std::string& getFull () const { return m_full ; }

	//CCommentFrame3& operator=(const std::string& f_val);

	virtual ~CCommentFrame3() {}

protected:
	CCommentFrame3(const CommentFrame3& f_frame, uint f_uFrameSize);

protected:
	Encoding	m_encodingRaw;
	uchar		m_lang[3];
	std::string	m_short;
	std::string	m_full;
};


class CURLFrame3 : public CFrame3
{
	friend class CFrame3;

public:
	//static CURLFrame3* create();

public:
	const std::string& getDescription() const { return m_description; }
	const std::string& getURL        () const { return m_url;         }

	//CURLFrame3& operator=(const std::string& f_val);

	virtual ~CURLFrame3() {}

protected:
	CURLFrame3(const URLFrame3& f_frame, uint f_uFrameSize);

private:
	//template<typename T>
	//void fill(const T* f_data, uint f_size, uint f_step = sizeof(T));

protected:
	Encoding	m_encodingRaw;
	std::string	m_description;
	std::string	m_url;
};


class CPictureFrame3 : public CFrame3
{
	friend class CFrame3;

private:
	enum PictureType
	{
		PTOther			= 0x00,
		PTIcon32PNG		= 0x01,
		PTIcon			= 0x02,
		PTCoverFront	= 0x03,
		PTCoverBack		= 0x04,
		PTLeafletPage	= 0x05,
		PTMedia			= 0x06,
		PTLeadArtist	= 0x07,
		PTArtist		= 0x08,
		PTConductor		= 0x09,
		PTBand			= 0x0A,
		PTComposer		= 0x0B,
		PTLyricist		= 0x0C,
		PTRecLocation	= 0x0D,
		PTDuringRec		= 0x0E,
		PTDuringPerf	= 0x0F,
		PTVideo			= 0x10,
		PTColourFish	= 0x11,
		PTIllustration	= 0x12,
		PTLogoArtist	= 0x13,
		PTLogoPublisher	= 0x14
	};

public:
	//const getPicture()
	//const std::string& getDescription() const { return m_description; }

	//CURLFrame3& operator=(const std::string& f_val);

	virtual ~CPictureFrame3() {}

protected:
	CPictureFrame3(const PictureFrame3& f_frame, uint f_uFrameSize);

private:
	//template<typename T>
	//void fill(const T* f_data, uint f_size, uint f_step = sizeof(T));

protected:
	Encoding	m_encodingRaw;
	std::string m_mime;
	PictureType	m_type;
	std::string	m_description;
	//std::vector<uchar>	m_url;
};

#endif // __FRAME_H__

