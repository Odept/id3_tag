#ifndef __UTF8_H__
#define __UTF8_H__

#pragma once

#include <vector>
#include <string>


class UTF8
{
public:
	static std::string fromUCS2   (const char* p, size_t sz) { return fromU16(p, sz, "UCS-2");    }
	static std::string fromUTF16BE(const char* p, size_t sz) { return fromU16(p, sz, "UTF-16BE"); }

private:
	static std::string fromU16(const char* f_data, size_t f_size, const char* f_format);
};

#endif //__UTF8_H__

