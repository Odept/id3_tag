#include "common.h"

#include "tag.h"

#include <cstdio>


#define LOG(msg)	std::cout << msg << std::endl
//#define ERROR(msg)  do { std::cerr << "ERROR @ " << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl; } while(0)


static void printTagV1(FILE* f)
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
	tagSize = Tag::IID3v1::getSize(&buf[0], 0, buf.size());
	if(!tagSize)
	{
		LOG("No ID3v1 tag");
		return;
	}

	auto tag = Tag::IID3v1::create(&buf[0], 0, tagSize);

	LOG("Title:   " << tag->getTitle());
	LOG("Artist:  " << tag->getArtist());
	LOG("Album:   " << tag->getAlbum());
	LOG("Year:    " << tag->getYear());
	LOG("Comment: " << tag->getComment());
	LOG("Vesrion: " << (tag->isV11() ? "1.1" : "1"));
	if(tag->isV11())
		LOG("Track:   " << tag->getTrack());
	auto idGenre = tag->getGenreIndex();
	LOG("Genre:   " << idGenre << " (" << Tag::genre(idGenre) << ')');
}

// ================
static std::string makeAlignedCaption(const std::string& f_name, int f_index = -1)
{
	auto str = f_name;
	if(f_index >= 0)
		str = str + '(' + std::to_string(f_index) + ')';

	auto n = 16 - str.length();
	if(n > 0)
		str.append(n, ' ');
	str.append(1, ':');

	return str;
}

using tag_frame_count_getter_t	= unsigned				(Tag::IID3v2::*)() const;
using tag_frame_getter_t		= const std::string&	(Tag::IID3v2::*)(unsigned f_index) const;
using tag_genre_index_getter_t	= int					(Tag::IID3v2::*)(unsigned f_index) const;
static void printFrames(const std::string& f_name, const Tag::IID3v2& f_tag, tag_frame_count_getter_t f_pfnCount, tag_frame_getter_t f_pfnGetter, tag_genre_index_getter_t f_pfnGenreIndex)
{
	auto nTags = (f_tag.*f_pfnCount)();
	if(nTags < 2)
	{
		auto caption = makeAlignedCaption(f_name);
		if(nTags)
		{
			caption = caption + ' ' + (f_tag.*f_pfnGetter)(0);
			if(f_pfnGenreIndex)
				caption = caption + " (" + std::to_string((f_tag.*f_pfnGenreIndex)(0)) + ')';
			LOG(caption);
		}
	}
	else
	{
		for(unsigned i = 0; i < nTags; ++i)
		{
			auto caption = makeAlignedCaption(f_name, i) + ' ' + (f_tag.*f_pfnGetter)(i);
			if(f_pfnGenreIndex)
				caption = caption + " (" + std::to_string((f_tag.*f_pfnGenreIndex)(i)) + ')';
			LOG(caption);
		}
	}
}

static void printTagV2(FILE* f)
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
	auto tagSize = Tag::IID3v2::getSize(&buf[0], 0, fsize);
	if(!tagSize)
	{
		LOG("No ID3v2 tag");
		return;
	}

	auto tag = Tag::IID3v2::create(&buf[0], 0, tagSize);

#define PRINT_FRAMES(Name, ExFn)	printFrames(#Name, *tag, &Tag::IID3v2::get##Name##Count, &Tag::IID3v2::get##Name, ExFn)
#define PRINT(Name)					PRINT_FRAMES(Name, nullptr)

	LOG(makeAlignedCaption("Version") << ' ' << tag->getMinorVersion() << '.' << tag->getRevision());
	PRINT(Track);
	PRINT(Disc);
	PRINT(BPM);
	PRINT(Title);
	PRINT(Artist);
	PRINT(Album);
	PRINT(AlbumArtist);
	PRINT(Year);
	PRINT_FRAMES(Genre, &Tag::IID3v2::getGenreIndex);
	PRINT(Comment);
	PRINT(Composer);
	PRINT(Publisher);
	PRINT(OrigArtist);
	PRINT(Copyright);
	PRINT(URL);
	PRINT(Encoded);

	std::vector<std::string> uframes = tag->getUnknownFrames();
	if(auto n = uframes.size())
	{
		std::cout << "Unknown frames  :";
		for(uint i = 0; i < n; ++i)
			std::cout << " " << uframes[i];
		std::cout << std::endl;
	}
}

// ================
static void printTagAPE(FILE* f)
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
	auto tagSize = Tag::IAPE::getSize(&buf[0], 0, buf.size());
	if(!tagSize)
	{
		LOG("No tag");
		return;
	}

	auto tag = Tag::IAPE::create(&buf[0], 0, tagSize);
	LOG("Tag OK");
}

// ====================================
static void test_file(const char* f_path)
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

