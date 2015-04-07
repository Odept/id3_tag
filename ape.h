#ifndef __APE_H__
#define __APE_H__

#pragma once

#include <vector>


class CAPE
{
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	static CAPE* gen(const uchar* f_pData, unsigned long long f_size, uint* f_puTagSize = NULL);

public:

private:
	CAPE(const uchar* f_data, uint f_size);
	CAPE();

private:
	std::vector<uchar> m_data;
};

#endif // __APE_H__

