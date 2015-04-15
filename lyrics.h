#ifndef __LYRICS_H__
#define __LYRICS_H__

#pragma once

#include <vector>


class CLyrics
{
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	static CLyrics* gen(const uchar* f_pData, unsigned long long f_size, uint* f_puTagSize = 0);

private:
	CLyrics(const uchar* f_data, uint f_size);
	CLyrics();

private:
	std::vector<uchar> m_data;
};

#endif // __LYRICS_H__

