#ifndef __ID3_GENRE_H__
#define __ID3_GENRE_H__

#pragma once

#include <string>


class CGenre
{
private:
	typedef unsigned int uint;

public:
	static const char* get(uint f_index);

public:
	CGenre(uint f_index):
		m_indexV1(f_index),
		m_extended(false)
	{
		if(const char* szGenre = get(f_index))
			m_genre = szGenre;
	}
	CGenre(const std::string& f_genre);

	const std::string&	str()			const { return m_genre;    }
	int					getIndex()		const { return m_indexV1;  }
	bool				isExtended()	const { return m_extended; }

private:
	CGenre();

private:
	std::string	m_genre;
	int			m_indexV1;
	bool		m_extended;
};

#endif // __ID3_GENRE_H__

