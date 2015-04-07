#ifndef __ID3_V2_H__
#define __ID3_V2_H__

#pragma once

#include <vector>
#include <map>
#include <string>


struct Tag;

class CFrame3;
class CRawFrame3;
class CTextFrame3;
class CGenreFrame3;
class CCommentFrame3;
class CURLFrame3;


enum FrameID
{
	FrameUnknown,
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

	FrameDword = 0xFFFFFFFF
};

// ============================================================================
class CID3v2
{
private:
	typedef unsigned int	uint;
	typedef unsigned short	ushort;
	typedef unsigned char	uchar;
	
public:
	static CID3v2* create();
	static CID3v2* gen(const uchar* f_pData, unsigned long long f_size, uint* f_puTagSize = NULL);

public:
	~CID3v2();

	uint		getVersion()	const;

#define DECL_GETTER_SETTER(Name) \
	const std::string& get##Name() const; \
	void set##Name(const std::string&)

	DECL_GETTER_SETTER(Track);
	DECL_GETTER_SETTER(Disc);
	DECL_GETTER_SETTER(BPM);

	DECL_GETTER_SETTER(Title);
	DECL_GETTER_SETTER(Artist);
	DECL_GETTER_SETTER(Album);
	DECL_GETTER_SETTER(AlbumArtist);
	DECL_GETTER_SETTER(Year);

	bool				isExtendedGenre()	const;
	const std::string	getGenre()			const;
	const std::string&	getGenreEx()		const;
	int					getGenreIndex()		const;

	//DECL_GETTER_SETTER(Comment);
	const std::string& getComment();

	DECL_GETTER_SETTER(Composer);
	DECL_GETTER_SETTER(Publisher);
	DECL_GETTER_SETTER(OrigArtist);
	DECL_GETTER_SETTER(Copyright);
	//DECL_GETTER_SETTER(URL);
	const std::string& getURL();
	DECL_GETTER_SETTER(Encoded);
#undef DECL_GETTER_SETTER

	std::vector<std::string> getUnknownFrames() const;

private:
	static const Tag* findTag(const uchar* f_pData, unsigned long long f_size);

private:
	// Although only a header is needed here, pass a tag to avoid Tag class definition
	CID3v2(const Tag& f_header);
	CID3v2();

	bool parse(const Tag& f_tag);
	bool parse3(const Tag& f_tag);

	void cleanup();

	CTextFrame3* getTextFrame(FrameID f_id) const;
	const CGenreFrame3* getGenreFrame() const;
	const CCommentFrame3* getCommentFrame() const;
	const CURLFrame3* getURLFrame() const;

	const std::string& strTextFrame(FrameID f_id) const;
	void setTextFrame(FrameID f_id, const std::string& f_val);

private:
	uint m_version;

	typedef std::map<FrameID, CFrame3*> frames_t;
	frames_t m_frames;

	typedef std::vector<CRawFrame3*> unknownFrames_t;
	unknownFrames_t m_framesUnknown;

	std::string m_strEmpty;
};

#endif // __ID3_V2_H__

