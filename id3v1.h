#pragma once


#include "tag.h"

#include "common.h"


class CID3v1 : public Tag::IID3v1
{
public:
	union Tag_t
	{
		struct __attribute__ ((__packed__))
		{
			char	Id		[3];
			char	Title	[30];
			char	Artist	[30];
			char	Album	[30];
			char	Year	[4];
			union
			{
				char		Comment[30];
				struct
				{
					char	Comment11[28];
					uchar	v10;
					uchar	Track;
				};
			};
			uchar	Genre;
		};
		uchar Raw[128];

		bool isValid() const { return (Id[0] == 'T' && Id[1] == 'A' && Id[2] == 'G'); }
		bool isV11() const { return (v10 == 0); }
	};
	static_assert(sizeof(Tag_t) == sizeof(Tag_t::Raw), "Invalid size of the ID3v1 tag structure");

private:
	enum class ModMask
	{
		Title		= 1 << 0,
		Artist		= 1 << 1,
		Album		= 1 << 2,
		Year		= 1 << 3,
		Comment		= 1 << 4,
		Comment11	= 1 << 4,
		Track		= 1 << 5,
		Genre		= 1 << 6
	};

	// ================================
public:
	CID3v1(const Tag_t& f_tag);
	CID3v1() = delete;

#define DECL_GETTER(Type, Name) \
	Type get##Name() const final override
#define DECL_SETTER(Type, Name, FieldSuffix) \
	void set##Name(const Type f_##FieldSuffix) final override
#define SET_IF_MODIFIED(Name, FieldSuffix) \
	if(m_##FieldSuffix != f_##FieldSuffix) \
	{ \
		m_maskModified |= static_cast<uint>(ModMask::Name); \
		m_##FieldSuffix = f_##FieldSuffix; \
	}

#define DEF_GETTER(Type, Name, FieldSuffix) \
	DECL_GETTER(Type, Name) { return m_##FieldSuffix; }
#define DEF_SETTER(Type, Name, FieldSuffix) \
	DECL_SETTER(Type, Name, FieldSuffix) \
	{ \
		SET_IF_MODIFIED(Name, FieldSuffix); \
	}

#define DEF_GETTER_SETTER_MODIFIED_STR(Name, FieldSuffix) \
	DEF_GETTER(const std::string&, Name, FieldSuffix); \
	DEF_SETTER(std::string&, Name, FieldSuffix);

	DEF_GETTER_SETTER_MODIFIED_STR(Title	, title		);
	DEF_GETTER_SETTER_MODIFIED_STR(Artist	, artist	);
	DEF_GETTER_SETTER_MODIFIED_STR(Album	, album		);
	DEF_GETTER_SETTER_MODIFIED_STR(Year		, year		);
	DEF_GETTER_SETTER_MODIFIED_STR(Comment	, comment	);

	DECL_GETTER(unsigned, Track)
	{
		ASSERT(isV11());
		return m_track;
	}
	DECL_SETTER(unsigned, Track, track)
	{
		ASSERT(isV11());
		ASSERT(isUint8(f_track));
		SET_IF_MODIFIED(Track, track);
	}

	DEF_GETTER(unsigned, GenreIndex, genre);
	DECL_SETTER(unsigned, GenreIndex, genre)
	{
		ASSERT(isUint8(f_genre));
		SET_IF_MODIFIED(Genre, genre);
	}
#undef DECL_GETTER
#undef DECL_SETTER
#undef SET_IF_MODIFIED
#undef DEF_GETTER
#undef DEF_SETTER
#undef DEF_GETTER_SETTER_MODIFIED_STR

	bool isV11() const final override { return m_v11; }
	//void setV11(bool f_val) { m_v11 = f_val; }

	void serialize(std::vector<unsigned char>& f_outStream) final override;

private:
	static bool isUint8(uint f_val) { return (f_val <= 0xFF); }

	void flushChanges();

private:
	bool m_v11;

	std::string m_title;
	std::string m_artist;
	std::string m_album;
	std::string m_year;
	std::string m_comment;
	uint m_track;
	uint m_genre;

	uint m_maskModified;

	// A raw tag
	Tag_t m_tag;
};

