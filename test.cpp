#include "common.h"

#include "id3v1.h"

#include <stdio.h>


void test_file(const char* f_path)
{
	FILE* f;

	f = fopen(f_path, "rb");
	if(!f)
	{
		std::cout << "Failed to open \"" << f_path << "\"" << std::endl;
		return;
	}

	do
	{
		if(fseek(f, -(int)CID3v1::TagSize, SEEK_END) == -1)
		{
			std::cout << "No ID3v1 tag (file too small)" << std::endl;
			break;
		}

		std::vector<uchar> buf;
		buf.resize(CID3v1::TagSize);

		if(fread(&buf[0], CID3v1::TagSize, 1, f) != 1)
		{
			std::cout << "Failed to read " << CID3v1::TagSize << " bytes" << std::endl;
			break;
		}

		CID3v1 tag(buf);
		std::cout << f_path << std::endl << "================" << std::endl;
		if(!tag.isValid())
		{
			std::cout << "No ID3v1 tag" << std::endl;
			break;
		}

		std::cout << "Title:   " << tag.getTitle() << std::endl <<
					 "Artist:  " << tag.getArtist() << std::endl <<
					 "Album:   " << tag.getAlbum() << std::endl <<
					 "Year:    " << tag.getYear() << std::endl <<
					 "Comment: " << tag.getComment() << std::endl <<
					 "Vesrion: " << (tag.isV11() ? "1.1" : "1") << std::endl;
		if(tag.isV11())
			std::cout << "Track:   " << tag.getTrack() << std::endl;
		std::cout << "Genre:   " << tag.getGenre() << std::endl;
	}
	while(0);

	fclose(f);
}

int main(int, char**)
{
	//test_header(0x44e0fbff);
	test_file("test.mp3");

	return 0;
}

