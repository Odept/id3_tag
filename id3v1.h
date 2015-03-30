#ifndef __ID3_V1_H__
#define __ID3_V1_H__

#pragma once


struct Tag;


class CID3v1
{
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	static CID3v1* create();
	static CID3v1* gen(const uchar* f_pData, unsigned long long f_size);

public:
	bool isV11() const;
	void setV11(bool f_val);

#define DECL_GETTER_SETTER(Type, Name) \
	const Type get##Name() const; \
	void set##Name(const Type)

	DECL_GETTER_SETTER(char*, Title     );
	DECL_GETTER_SETTER(char*, Artist    );
	DECL_GETTER_SETTER(char*, Album     );
	DECL_GETTER_SETTER(uint , Year      );
	DECL_GETTER_SETTER(char*, Comment   );
	DECL_GETTER_SETTER(uint , Track     );
	DECL_GETTER_SETTER(uint , GenreIndex);
	DECL_GETTER_SETTER(char*, Genre     );
#undef DECL_GETTER_SETTER
		
private:
	CID3v1(const Tag& f_tag);
	CID3v1();

	void copyField(char* f_dst, const char* f_src, uint f_size);

private:
	bool m_v11;

	char m_title[31];
	char m_artist[31];
	char m_album[31];
	uint m_year;
	char m_comment[31];
	uint m_track;
	uint m_genre;

public:
	static const uint TagSize = 128;
};

#endif // __ID3_V1_H__

