#include "common.h"

#include "tag.h"

#include <cstdio>


#define LOG(msg)	std::cout << msg << std::endl


void printTagV1(FILE* f)
{
	auto tagSize = Tag::IID3v1::size();
	if(fseek(f, -static_cast<long>(tagSize), SEEK_END) == -1)
	{
		LOG("No ID3v1 tag (file too small)");
		return;
	}

	std::vector<uchar> buf(tagSize);

	if(fread(&buf[0], buf.size(), 1, f) != 1)
	{
		LOG("Failed to read " << buf.size() << " bytes");
		return;
	}

	LOG("ID3v1" << std::endl << "================");
	tagSize = Tag::IID3v1::getSize(&buf[0], buf.size());
	if(!tagSize)
	{
		LOG("No ID3v1 tag");
		return;
	}

	auto tag = Tag::IID3v1::create(&buf[0], tagSize);

	LOG("Title:   " << tag->getTitle());
	LOG("Artist:  " << tag->getArtist());
	LOG("Album:   " << tag->getAlbum());
	LOG("Year:    " << tag->getYear());
	LOG("Comment: " << tag->getComment());
	LOG("Vesrion: " << (tag->isV11() ? "1.1" : "1"));
	if(tag->isV11())
		LOG("Track:   " << tag->getTrack());
	LOG("Genre:   " << tag->getGenre());
}


void printTagV2(FILE* f)
{
	fseek(f, 0, SEEK_END);
	auto fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<uchar> buf(fsize);
	if(fread(&buf[0], fsize, 1, f) != 1)
	{
		LOG("Failed to read the file");
		return;
	}

	LOG("ID3v2" << std::endl << "================");
	auto tagSize = Tag::IID3v2::getSize(&buf[0], fsize);
	if(!tagSize)
	{
		LOG("No ID3v2 tag");
		return;
	}

	auto tag = Tag::IID3v2::create(&buf[0], tagSize);

	LOG("Version:         " << std::hex << tag->getVersion() << std::dec);
	LOG("Track:           " << tag->getTrack());
	LOG("Disc:            " << tag->getDisc());
	LOG("BPM:             " << tag->getBPM());
	LOG("Title:           " << tag->getTitle());
	LOG("Artist:          " << tag->getArtist());
	LOG("Album:           " << tag->getAlbum());
	LOG("Album Artist:    " << tag->getAlbum());
	LOG("Year:            " << tag->getYear());
	LOG("Genre:           " << tag->getGenre() << " (" << (tag->isExtendedGenre() ? tag->getGenreEx() : tag->getGenre()) << ")");
	LOG("Comment:         " << tag->getComment());
	LOG("Composer:        " << tag->getComposer());
	LOG("Publisher:       " << tag->getPublisher());
	LOG("Original Artist: " << tag->getOrigArtist());
	LOG("Copyright:       " << tag->getCopyright());
	LOG("URL:             " << tag->getURL());
	LOG("Encoded:         " << tag->getEncoded());

	std::vector<std::string> uframes = tag->getUnknownFrames();
	if(auto n = uframes.size())
	{
		std::cout << "Unknown frames: ";
		for(uint i = 0; i < n; ++i)
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
		LOG("APE tag not found");;
		return;
	}

	std::vector<uchar> buf(fsize - offset);

	if(fread(&buf[0], buf.size(), 1, f) != 1)
	{
		LOG("Failed to read " << buf.size() << " bytes");
		return;
	}

	LOG("APE" << std::endl << "================");
	auto tagSize = Tag::IAPE::getSize(&buf[0], buf.size());
	if(!tagSize)
	{
		LOG("No tag");
		return;
	}

	auto tag = Tag::IAPE::create(&buf[0], tagSize);
	LOG("Tag OK");
}


void test_file(const char* f_path)
{
	if(FILE* f = fopen(f_path, "rb"))
	{
		LOG("Checking \"" << f_path << "\"...");

		printTagV1(f);
		LOG("");
		printTagV2(f);
		LOG("");
		printTagAPE(f);

		fclose(f);
	}
	else
		LOG("Failed to open \"" << f_path << "\"");
}

int main(int, char**)
{
	//test_header(0x44e0fbff);
	test_file("test.mp3");

	return 0;
}

