#include "common.h"

#include "id3v1.h"
#include "id3v2.h"
#include "ape.h"

#include <cstdio>
#include <memory>


void printTagV1(FILE* f)
{
	if(fseek(f, -(int)CID3v1::getSize(), SEEK_END) == -1)
	{
		std::cout << "No ID3v1 tag (file too small)" << std::endl;
		return;
	}

	std::vector<uchar> buf(CID3v1::getSize());

	if(fread(&buf[0], CID3v1::getSize(), 1, f) != 1)
	{
		std::cout << "Failed to read " << CID3v1::getSize() << " bytes" << std::endl;
		return;
	}

	std::cout << "ID3v1" << std::endl << "================" << std::endl;
	std::auto_ptr<CID3v1> tag(CID3v1::gen(&buf[0], buf.size()));
	if(!tag.get())
	{
		std::cout << "No ID3v1 tag" << std::endl;
		return;
	}

	std::cout << "Title:   " << tag->getTitle() << std::endl <<
				 "Artist:  " << tag->getArtist() << std::endl <<
				 "Album:   " << tag->getAlbum() << std::endl <<
				 "Year:    " << tag->getYear() << std::endl <<
				 "Comment: " << tag->getComment() << std::endl <<
				 "Vesrion: " << (tag->isV11() ? "1.1" : "1") << std::endl;
	if(tag->isV11())
		std::cout << "Track:   " << tag->getTrack() << std::endl;
	std::cout << "Genre:   " << tag->getGenre() << std::endl;
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
	std::auto_ptr<CID3v2> tag(CID3v2::gen(&buf[0], fsize));
	if(!tag.get())
	{
		std::cout << "No ID3v2 tag" << std::endl;
		return;
	}

	std::cout << "Version:         " << std::hex << tag->getVersion() << std::dec << std::endl <<
				 "Track:           " << tag->getTrack()  << std::endl <<
				 "Disc:            " << tag->getDisc()   << std::endl <<
				 "BPM:             " << tag->getBPM()    << std::endl <<
				 "Title:           " << tag->getTitle()  << std::endl <<
				 "Artist:          " << tag->getArtist() << std::endl <<
				 "Album:           " << tag->getAlbum()  << std::endl <<
				 "Album Artist:    " << tag->getAlbum()  << std::endl <<
				 "Year:            " << tag->getYear()   << std::endl <<
				 "Genre:           " << tag->getGenre()  << " (";
	if(tag->isExtendedGenre())
		std::cout << tag->getGenreEx();
	else
		std::cout << tag->getGenreIndex();
	std::cout << ")" << std::endl <<
				 "Comment:         " << tag->getComment()		<< std::endl <<
				 "Composer:        " << tag->getComposer()		<< std::endl <<
				 "Publisher:       " << tag->getPublisher()		<< std::endl <<
				 "Original Artist: " << tag->getOrigArtist()	<< std::endl <<
				 "Copyright:       " << tag->getCopyright()		<< std::endl <<
				 "URL:             " << tag->getURL()			<< std::endl <<
				 "Encoded:         " << tag->getEncoded()		<< std::endl;

	std::vector<std::string> uframes = tag->getUnknownFrames();
	if(int n = (int)uframes.size())
	{
		std::cout << "Unknown frames: ";
		for(int i = 0; i < n; i++)
			std::cout << " " << uframes[i];
		std::cout << std::endl;
	}
}


void printTagAPE(FILE* f)
{
	uint offset = 0x317770;

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	if(fseek(f, offset, SEEK_SET) == -1)
	{
		std::cout << "APE tag not found" << std::endl;
		return;
	}

	std::vector<uchar> buf(fsize - offset);

	if(fread(&buf[0], buf.size(), 1, f) != 1)
	{
		std::cout << "Failed to read " << buf.size() << " bytes" << std::endl;
		return;
	}

	std::cout << "APE" << std::endl << "================" << std::endl;
	std::auto_ptr<CAPE> tag(CAPE::gen(&buf[0], buf.size()));
	if(!tag.get())
	{
		std::cout << "No APE tag" << std::endl;
		return;
	}

	std::cout << "APE tag OK" << std::endl;
}


void test_file(const char* f_path)
{
	if(FILE* f = fopen(f_path, "rb"))
	{
		std::cout << "Checking \"" << f_path << "\"..." << std::endl;

		printTagV1(f);
		std::cout << std::endl;
		printTagV2(f);
		std::cout << std::endl;
		printTagAPE(f);

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

