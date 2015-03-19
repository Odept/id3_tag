#include "common.h"

#include "id3v1.h"
#include "id3v2.h"

#include <stdio.h>


void printTagV1(FILE* f)
{
	if(fseek(f, -(int)CID3v1::TagSize, SEEK_END) == -1)
	{
		std::cout << "No ID3v1 tag (file too small)" << std::endl;
		return;
	}

	std::vector<uchar> buf(CID3v1::TagSize);

	if(fread(&buf[0], CID3v1::TagSize, 1, f) != 1)
	{
		std::cout << "Failed to read " << CID3v1::TagSize << " bytes" << std::endl;
		return;
	}

	std::cout << "ID3v1" << std::endl << "================" << std::endl;
	CID3v1 tag(buf);
	if(!tag.isValid())
	{
		std::cout << "No ID3v1 tag" << std::endl;
		return;
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


void printTagV2(FILE* f)
{
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<uchar> buf(fsize);
	if(fread(&buf[0], fsize, 1, f) != 1)
	{
		std::cout << "Failed to read the file";
		return;
	}

	std::cout << "ID3v2" << std::endl << "================" << std::endl;
	CID3v2 tag(buf);
	if(!tag.isValid())
	{
		std::cout << "No ID3v2 tag" << std::endl;
		return;
	}

	std::cout << "Version: " << tag.getVersion() << std::endl <<
				 "Track:   " << tag.getTrack() << std::endl <<
				 "Disk:    " << tag.getDisk() << std::endl <<
				 "Title:   " << tag.getTitle() << std::endl <<
				 "Artist:  " << tag.getArtist() << std::endl <<
				 "Album:   " << tag.getAlbum() << std::endl <<
				 "Year:    " << tag.getYear() << std::endl <<
				 "Genre:   " << tag.getGenre() << " (";
	if(tag.isExtendedGenre())
		std::cout << tag.getGenreEx();
	else
		std::cout << tag.getGenreIndex();
	std::cout << ")" << std::endl;
				 //"Comment: " << tag.getComment() << std::endl <<
}

void test_file(const char* f_path)
{
	if(FILE* f = fopen(f_path, "rb"))
	{
		std::cout << "Checking \"" << f_path << "\"..." << std::endl;

		printTagV1(f);
		printTagV2(f);

		fclose(f);
	}
	else
		std::cout << "Failed to open \"" << f_path << "\"" << std::endl;
}

int main(int, char**)
{
	//test_header(0x44e0fbff);
	test_file("test.mp3");

	return 0;
}

