#pragma once

#include "genre.h"

#include "common.h"

#include <vector>
#include <string>


enum FrameType
{
	FrameInvalid,
	FrameTrack,
	FrameDisc,
	FrameBPM,
	FrameTitle,
	FrameArtist,
	FrameAlbum,
	FrameAlbumArtist,
	FrameYear,
	FrameGenre,
	FrameComment,
	FrameComposer,
	FramePublisher,
	FrameOrigArtist,
	FrameCopyright,
	FrameURL,
	FrameEncoded,
	FramePicture,
	FrameUnknown,

	FrameDword = 0xFFFFFFFF
};


enum Encoding
{
	EncRaw		= 0x00,	/*ISO-8859-1 (LATIN-1)*/
	EncUCS2		= 0x01,	/*UCS-2 (UTF-16, with BOM)*/
	EncUTF16BE	= 0x02,	/*UTF-16BE (without BOM, since v2.4)*/
	EncUTF8		= 0x03	/*UTF-8 (since v2.4)*/
};


struct __attribute__ ((__packed__)) Frame3
{
	struct __attribute__ ((__packed__)) Header_t
	{
		union
		{
			char	Id[4];
			uint	IdFourCC;
		};
		uchar		SizeRaw[4];
		ushort		Flags;

		bool isValid() const
		{
			for(uint i = 0; i < sizeof(Id) / sizeof(Id[0]); ++i)
			{
				char c = Id[i];
				if(c < '0' || (c > '9' && c < 'A') || c > 'Z')
					return false;
			}
			return true;
		}

// change to "size()"
		size_t getSize() const
		{
			return (SizeRaw[0] << 24) | (SizeRaw[1] << 16) | (SizeRaw[2] << 8) | SizeRaw[3];
		}
	} Header;
	uchar Data[];
};


struct __attribute__ ((__packed__)) TextFrame3
{
	uchar	Encoding;
	char	RawString[];
};


struct __attribute__ ((__packed__)) CommentFrame3
{
	uchar	Encoding;
	uchar	Language[3];
	char	RawShortString[];
	//char	RawFullString[];
};


struct __attribute__ ((__packed__)) URLFrame3
{
	uchar	Encoding;
	char	Description[];
	//char	URL[];
};


struct __attribute__ ((__packed__)) PictureFrame3
{
	uchar	Encoding;
	char	MIME[];
};

// ============================================================================
class CFrame3
{
public:
	static FrameType getFrameType(const Frame3::Header_t& f_header);

public:
	virtual ~CFrame3() {}

	//bool isModified() const { return m_modified; }
//	void setModified() { m_modified = true; }

//protected:
//	CFrame3(): m_modified(false) {}

//protected:
//	bool m_modified;
};


class CRawFrame3 : public CFrame3
{
public:
	CRawFrame3(const Frame3& f_frame);
	virtual ~CRawFrame3() {}
	const std::string& getId() const { return m_id; }

protected:
	std::vector<char> m_frame;
	std::string m_id;
};


class CTextFrame3 : public CFrame3
{
public:
	CTextFrame3(const Frame3& f_frame);
	CTextFrame3(const std::string f_text):
		m_encodingRaw(EncUCS2),
		m_text(f_text)
	{}
	virtual ~CTextFrame3() {}

	const std::string& get() const { return m_text; }

	virtual CTextFrame3& operator=(const std::string& f_text)
	{
		m_text = f_text;
		//setModified();
		return *this;
	}

protected:
	CTextFrame3(): m_encodingRaw(EncUCS2) {}

protected:
	Encoding	m_encodingRaw;
	std::string	m_text;
};


class CGenreFrame3 : public CTextFrame3
{
public:
	CGenreFrame3(const Frame3& f_frame):
		CTextFrame3(f_frame)
	{
		// m_text can be modified so don't use it as a parameter
		std::string genre = m_text;
		init(genre);
	}
	CGenreFrame3(uint f_index):
		m_indexV1(f_index),
		m_extended(false)
	{}
	CGenreFrame3(const std::string& f_text) { init(f_text); }
	virtual ~CGenreFrame3() {}

	int    getIndex() const { return m_indexV1;  }
	bool isExtended() const { return m_extended; }

	virtual CTextFrame3& operator=(const std::string& f_text)
	{
		m_indexV1 = -1;
		return CTextFrame3::operator=(f_text);
	}
	virtual CGenreFrame3& operator=(uint f_index)
	{
		ASSERT(!genre(f_index).empty());

		m_text = std::string();
		m_indexV1 = f_index;
		//setModified();

		return *this;
	}

private:
	void init(const std::string& f_text);

protected:
	int		m_indexV1;
	bool	m_extended;
};


class CCommentFrame3 : public CTextFrame3
{
public:
	CCommentFrame3(const Frame3& f_frame);
	CCommentFrame3(const std::string f_text):
		CTextFrame3(f_text)
	{
		m_lang[0] = 'e';
		m_lang[1] = 'n';
		m_lang[2] = 'g';
	}
	virtual ~CCommentFrame3() {}

	const std::string& getShort() const { return m_short; }

protected:
	uchar		m_lang[3];
	std::string	m_short;
};


class CURLFrame3 : public CTextFrame3
{
public:
	CURLFrame3(const Frame3& f_frame);
	CURLFrame3(const std::string f_text): CTextFrame3(f_text) {}
	virtual ~CURLFrame3() {}

	const std::string& getDescription() const { return m_description; }

	//CURLFrame3& operator=(const std::string& f_val);

protected:
	std::string	m_description;
};


class CPictureFrame3 : public CFrame3
{
public:
	CPictureFrame3(const Frame3& f_frame);
	virtual ~CPictureFrame3() {}

	const std::vector<uchar>& getData()	const { return m_data;			}
	//const std::string& getType()		const { return m_type;			}
	const std::string& getDescription()	const { return m_description;	}

	//CURLFrame3& operator=(const std::string& f_val);

private:
	//template<typename T>
	//void fill(const T* f_data, size_t f_size, uint f_step = sizeof(T));

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

protected:
	Encoding			m_encodingRaw;
	std::string			m_mime;
	PictureType			m_type;
	std::string			m_description;
	std::vector<uchar>	m_data;
};

