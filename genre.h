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
	CGenre(uint f_index);
	CGenre(const std::string& f_genre);

	const std::string&	str()			const;
	int					getIndex()		const;
	bool				isExtended()	const;

private:
	CGenre();

private:
	std::string	m_genre;
	int			m_indexV1;
	bool		m_extended;
};

#endif // __ID3_GENRE_H__

