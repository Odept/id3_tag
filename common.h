#ifndef _MPEG_COMMON_H_
#define _MPEG_COMMON_H_

#include <cstdlib>
#include <iostream>

#define ASSERT(X) if(!(X)) { std::cout << "Abort @ " << __FILE__ << ":" << __LINE__ << ": \"" << #X << "\"" << std::endl; std::abort(); }

typedef unsigned int	uint;
typedef unsigned short	ushort;
typedef unsigned char	uchar;

#endif // _MPEG_COMMON_H_

