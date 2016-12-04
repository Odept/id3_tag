#pragma once

#include "tag.h"
#include "genre.h"
#include "frame.h"

#include "common.h"

#include <vector>
#include <map>
#include <string>


class CID3v2 final : public Tag::IID3v2
{
public:
	struct __attribute__ ((__packed__)) Tag_t
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

			char	Id[3];
			uchar	Version;
			uchar	Revision;
			uchar	Flags;
			uint	SizeRaw;

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

				if(SizeRaw & 0x80808080)
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
						SizeRaw	== f_header.SizeRaw);
			}

			// Size (after unsychronisation and including padding, without header)
			uint size() const
			{
				auto pSize = reinterpret_cast<const uchar*>(&SizeRaw);
				return (pSize[0] << (24 - 3)) |
					   (pSize[1] << (16 - 2)) |
					   (pSize[2] << ( 8 - 1)) |
						pSize[3];
			}
		} Header;
		uchar Frames[];

		size_t getSize() const
		{
			return (sizeof(Header) +
					Header.size() +
					Header.hasFooter() * sizeof(Header));
		}
	};

	// ================================
public:
	CID3v2(const uchar* f_data, size_t f_size);
	CID3v2() = delete;
	~CID3v2() { cleanup(); }

	bool parse();

	// Getters/Setters
	uint getVersion() const { return m_version; }

#define DEF_GETTER_SETTER(Type, Name) \
	const std::string& get##Name() const final override \
	{ \
		return strTextFrame(getFrame<Type>(Frame##Name), m_strEmpty); \
	} \
	void set##Name(const std::string& f_str) final override \
	{ \
		m_modified = true; \
		if(auto pFrame = setFrame<Type>(getFrame<Type>(Frame##Name), f_str)) \
			m_frames[Frame##Name] = pFrame; \
	}
#define DEF_GETTER_SETTER_TEXT(Name)	DEF_GETTER_SETTER(CTextFrame3, Name)

	DEF_GETTER_SETTER_TEXT	(Track);
	DEF_GETTER_SETTER_TEXT	(Disc);
	DEF_GETTER_SETTER_TEXT	(BPM);

	DEF_GETTER_SETTER_TEXT	(Title);
	DEF_GETTER_SETTER_TEXT	(Artist);
	DEF_GETTER_SETTER_TEXT	(Album);
	DEF_GETTER_SETTER_TEXT	(AlbumArtist);
	DEF_GETTER_SETTER_TEXT	(Year);

	const std::string	getGenre		() const override final;
	const std::string&	getGenreEx		() const override final;
	bool				isExtendedGenre	() const override final;

	void				setGenre		(const std::string& f_text) override final;
	void				setGenre		(uint f_index) override final;

	DEF_GETTER_SETTER		(CCommentFrame3, Comment);

	DEF_GETTER_SETTER_TEXT	(Composer);
	DEF_GETTER_SETTER_TEXT	(Publisher);
	DEF_GETTER_SETTER_TEXT	(OrigArtist);
	DEF_GETTER_SETTER_TEXT	(Copyright);
	DEF_GETTER_SETTER		(CURLFrame3, URL);
	DEF_GETTER_SETTER_TEXT	(Encoded);

	//DECL_GETTER_SETTER(Picture);
	const std::vector<uchar>& getPictureData() const final override;
	const std::string& getPictureDescription() const final override;
#undef DEF_GETTER_SETTER
#undef DEF_GETTER_SETTER_TEXT

	std::vector<std::string> getUnknownFrames() const final override;

	void serialize(std::vector<uchar>& f_outStream) final override;

public:
	static int					genreIndex	(const std::string& f_text)	{ return ::genreIndex( f_text); }
	static const std::string&	genre		(uint f_index)				{ return ::genre     (f_index); }

private:
	bool parse3();

	template<typename T>
	static T* setFrame(T* f_pFrame, const std::string& f_str)
	{
		if(f_pFrame)
		{
			// Don't delete existing frames to keep a modified flag
			*f_pFrame = f_str;
			return nullptr;
		}
		else if(f_str.empty())
			return nullptr;
		else
		{
			T* pFrame = new T(f_str);
			//pFrame->setModified();
			return pFrame;
		}
	}

	static const std::string& strTextFrame(const CTextFrame3* f_pFrame, const std::string& f_default)
	{
		return f_pFrame ? f_pFrame->get() : f_default;
	}

	void cleanup();

	template<typename T>
	T*						getFrame		(FrameType f_type) const
	{
		auto it = m_frames.find(f_type);
		return (it == m_frames.end()) ? nullptr : static_cast<T*>(it->second);
	}
	const CGenreFrame3*		getGenreFrame	() const;
	const CPictureFrame3*	getPictureFrame	() const;

private:
	uint m_version;

	using frames_t = std::map<FrameType, CFrame3*>;
	frames_t m_frames;

	using unknownFrames_t = std::vector<CRawFrame3*>;
	unknownFrames_t m_framesUnknown;

	const std::string	m_strEmpty;

	// A raw tag
	std::vector<uchar> m_tag;
	// A temporary flag for simplicity
	bool m_modified;
};

