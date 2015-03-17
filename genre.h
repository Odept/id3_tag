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
		m_index(f_index)
	{
		if(const char* szGenre = get(f_index))
			m_genre = szGenre;
	}
	CGenre(const std::string& f_genre):
		m_genre(f_genre),
		m_index(-1)
	{
		for(uint i = 0; get(i); i++)
		{
			if(f_genre.compare( get(i) ) != 0)
				continue;
			m_index = i;
			break;
		}
	}

	const std::string&	str()		const { return m_genre; }
	int					getIndex()	const { return m_index; }

private:
	CGenre();

private:
	std::string	m_genre;
	int			m_index;
};

#endif // __ID3_GENRE_H__

