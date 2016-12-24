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
			char	Id[3];
			uchar	Version;
			uchar	Revision;
			uchar	Flags;
			uint	SizeRaw;


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

#define DEF_COUNT_GETTER(Name) \
	unsigned get##Name##Count() const final override \
	{ \
		auto it = m_frames.find(Frame##Name); \
		return (it == m_frames.cend()) ? 0 : it->second.size(); \
	}
#define DEF_GETTER(Name, FrameType, Method, ValType) \
	ValType get##Name(unsigned f_index) const final override \
	{ \
		auto it = m_frames.find(Frame##Name); \
		if(it == m_frames.cend()) \
			throw std::out_of_range(__FUNCTION__); \
		auto& vec = it->second; \
		return frame_cast<FrameType>(vec.at(f_index))->Method(); \
	}
#define DEF_SETTER(Name, FrameType, Method, ValType) \
	void set##Name(unsigned f_index, ValType f_val) final override \
	{ \
		ASSERT(!"Untested"); \
		auto& vec = m_frames[Frame##Name]; \
		if(f_index == vec.size()) \
			vec.emplace_back( std::make_shared<FrameType>(f_val) ); \
		else if(f_index < vec.size()) \
			frame_cast<FrameType>(vec[f_index])->Method(f_val); \
		else \
			throw std::out_of_range(__FUNCTION__); \
		m_modified = true; \
	}
#define DEF_GETTER_SETTER_TEXT_GENERAL(Name, FrameType) \
	DEF_COUNT_GETTER(Name) \
	DEF_GETTER(Name, FrameType, getText, const std::string&) \
	DEF_SETTER(Name, FrameType, setText, const std::string&)
#define DEF_GETTER_SETTER_TEXT(Name)	DEF_GETTER_SETTER_TEXT_GENERAL(Name, CTextFrame3)

	DEF_GETTER_SETTER_TEXT			(Track)
	DEF_GETTER_SETTER_TEXT			(Disc)
	DEF_GETTER_SETTER_TEXT			(BPM)

	DEF_GETTER_SETTER_TEXT			(Title)
	DEF_GETTER_SETTER_TEXT			(Artist)
	DEF_GETTER_SETTER_TEXT			(Album)
	DEF_GETTER_SETTER_TEXT			(AlbumArtist)
	DEF_GETTER_SETTER_TEXT			(Year)

	#define FrameGenreIndex FrameGenre
	DEF_GETTER_SETTER_TEXT_GENERAL	(Genre, CGenreFrame3)
	DEF_GETTER						(GenreIndex, CGenreFrame3, getIndex, int)
	DEF_SETTER						(GenreIndex, CGenreFrame3, setIndex, unsigned)
	bool							isExtendedGenre(unsigned f_index) const override final;
	#undef FrameGenreIndex

	DEF_GETTER_SETTER_TEXT_GENERAL	(Comment, CCommentFrame3)

	DEF_GETTER_SETTER_TEXT			(Composer)
	DEF_GETTER_SETTER_TEXT			(Publisher)
	DEF_GETTER_SETTER_TEXT			(OrigArtist)
	DEF_GETTER_SETTER_TEXT			(Copyright)
	DEF_GETTER_SETTER_TEXT_GENERAL	(URL, CURLFrame3)
	DEF_GETTER_SETTER_TEXT			(Encoded)

	#define FramePictureData FramePicture
	#define FramePictureDescription FramePicture
	DEF_COUNT_GETTER				(Picture)
	DEF_GETTER						(PictureData, CPictureFrame3, getData, const std::vector<uchar>&)
	DEF_GETTER						(PictureDescription, CPictureFrame3, getDescription, const std::string&)
	#undef FramePictureDescription
	#undef FramePictureData
#undef DEF_GETTER_SETTER_TEXT
#undef DEF_GETTER_SETTER_TEXT_GENERAL
#undef DEF_SETTER
#undef DEF_GETTER
#undef DEF_COUNT_GETTER

	std::vector<std::string> getUnknownFrames() const final override;

	size_t getSize() const final override
	{
		ASSERT(!m_modified);
		return m_tag.size();
	}

	bool hasIssues() const final override { return m_warnings; }

	void serialize(std::vector<uchar>& f_outStream) final override;

private:
	void parse();
	void parse3();

	template<typename T_To, typename T_From>
	static std::shared_ptr<T_To> frame_cast(const std::shared_ptr<T_From>& f_frame)
	{
		return std::static_pointer_cast<T_To>(f_frame);
	}

private:
	template<typename T>
	struct EnumHasher
	{
		unsigned operator()(const T& f_key) const { return static_cast<unsigned>(f_key); }
	};
	using frames_t = std::unordered_map<FrameType, std::vector<std::shared_ptr<CFrame3>>, EnumHasher<FrameType>>;

private:
	uint										m_ver_minor;
	uint										m_ver_revision;

	frames_t									m_frames;
	std::vector<std::shared_ptr<CMMJBFrame3>>	m_framesMMJB;
	std::vector<std::shared_ptr<CRawFrame3>>	m_framesUnknown;

	// A raw tag
	std::vector<uchar>							m_tag;
	// A temporary flag for simplicity
	bool										m_modified;

	uint										m_warnings;
};

