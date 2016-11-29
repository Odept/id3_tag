#pragma once


#include <vector>
#include <memory>


namespace Tag
{
	class ISerialize
	{
	public:
		virtual void serialize(std::vector<unsigned char>& f_outStream) const = 0;
	};


	class IAPE : public ISerialize
	{
	public:
		// To make sure the size argument of the create() is a result of the getSize()
		using TagSize = size_t;

	public:
		static TagSize					getSize	(const unsigned char* f_data, size_t f_size);
		static std::shared_ptr<IAPE>	create	(const unsigned char* f_data, TagSize f_size);
	};
}

