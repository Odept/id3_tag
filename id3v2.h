#pragma once

#include "tag.h"
#include "frame.h"

#include "common.h"

#include <vector>
#include <unordered_map>


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

	// Getters/Setters
	unsigned getMinorVersion() const final override { return m_ver_minor; }
	unsigned getRevision() const final override { return m_ver_revision; }

#define DEF_GETTER_SETTER(Type, Name) \
	const std::string& get##Name() const final override \
	{ \
		auto it = m_frames.find(Frame##Name); \
		return (it == m_frames.cend()) ? m_strEmpty : frame_cast<Type>(it->second)->getText(); \
	} \
	void set##Name(const std::string& f_str) final override \
	{ \
		m_modified = true; \
		auto it = m_frames.find(Frame##Name); \
		if(it == m_frames.end()) \
		{ \
			if(!f_str.empty()) \
				m_frames[Frame##Name] = std::make_shared<Type>(f_str); \
		} \
		else \
			frame_cast<Type>(it->second)->setText(f_str); \
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

	int					getGenreIndex		() const final override;
	void				setGenreIndex		(unsigned f_index) final override;
	const std::string&	getGenre			() const final override;
	void				setGenre			(const std::string& f_text) final override;
	bool				isExtendedGenre		() const override final;

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

	size_t getSize() const final override
	{
		ASSERT(!m_modified);
		return m_tag.size();
	}

	void serialize(std::vector<uchar>& f_outStream) final override;

private:
	void parse();
	void parse3();

	template<typename T_To, typename T_From>
	static std::shared_ptr<T_To> frame_cast(const std::shared_ptr<T_From>& f_frame)
	{
		return std::static_pointer_cast<T_To>(f_frame);
	}

	template<typename T_From>
	static std::shared_ptr<CGenreFrame3> genre_frame_cast(const std::shared_ptr<T_From>& f_frame)
	{
		return frame_cast<CGenreFrame3>(f_frame);
	}

private:
	template<typename T>
	struct EnumHasher
	{
		unsigned operator()(const T& f_key) const { return static_cast<unsigned>(f_key); }
	};
	using frames_t = std::unordered_map<FrameType, std::shared_ptr<CFrame3>, EnumHasher<FrameType>>;

private:
	uint										m_ver_minor;
	uint										m_ver_revision;

	frames_t									m_frames;
	std::vector<std::shared_ptr<CMMJBFrame3>>	m_framesMMJB;
	std::vector<std::shared_ptr<CRawFrame3>>	m_framesUnknown;

	const std::string							m_strEmpty;

	// A raw tag
	std::vector<uchar>							m_tag;
	// A temporary flag for simplicity
	bool										m_modified;
};

