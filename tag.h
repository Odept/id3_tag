#pragma once


#include <vector>
#include <memory>


namespace Tag
{
	class ISerialize
	{
	public:
		virtual void serialize(std::vector<unsigned char>& f_outStream) = 0;
	};


	class IAPE : public ISerialize
	{
	public:
		// To make sure the size argument of the create() is a result of the getSize()
		using TagSize = size_t;

		static TagSize					getSize	(const unsigned char* f_data, size_t f_size);
		static std::shared_ptr<IAPE>	create	(const unsigned char* f_data, TagSize f_size);
	};


	class ILyrics : public ISerialize
	{
	public:
		// To make sure the size argument of the create() is a result of the getSize()
		using TagSize = size_t;

		static TagSize					getSize	(const unsigned char* f_data, size_t f_size);
		static std::shared_ptr<ILyrics>	create	(const unsigned char* f_data, TagSize f_size);
	};


	class IID3v1 : public ISerialize
	{
	public:
		// To make sure the size argument of the create() is a result of the getSize()
		using TagSize = size_t;

		static size_t					getSize	(const unsigned char* f_data, size_t f_size);
		static std::shared_ptr<IID3v1>	create	(const unsigned char* f_data, size_t f_size);
		static std::shared_ptr<IID3v1>	create	();

	public:
		virtual bool				isV11				() const					= 0;

		virtual const std::string&	getTitle			() const					= 0;
		virtual void				setTitle			(const std::string& f_str)	= 0;

		virtual const std::string&	getArtist			() const					= 0;
		virtual void				setArtist			(const std::string& f_str)	= 0;

		virtual const std::string&	getAlbum			() const					= 0;
		virtual void				setAlbum			(const std::string& f_str)	= 0;

		virtual const std::string&	getYear				() const					= 0;
		virtual void				setYear				(const std::string& f_str)	= 0;

		virtual const std::string&	getComment			() const					= 0;
		virtual void				setComment			(const std::string& f_str)	= 0;

		virtual unsigned			getTrack			() const					= 0;
		virtual void				setTrack			(unsigned f_track)			= 0;

		virtual unsigned			getGenreIndex		() const					= 0;
		virtual void				setGenreIndex		(unsigned f_index)			= 0;
		virtual const std::string&	getGenre			() const					= 0;
		//virtual void				setGenre			(std::string& f_str)		= 0;
		//virtual bool				isModifiedGenre		() const					= 0;
	};
}

