
#include "SHAREDSTRINGS.h"
#include <Logic/Biff_records/SST.h>
#include <Logic/Biff_records/Continue.h>

namespace XLS
{


SHAREDSTRINGS::SHAREDSTRINGS(const unsigned short code_page)
: code_page_(code_page)
{
	size_ = 0 ;
}


SHAREDSTRINGS::~SHAREDSTRINGS()
{
}


BaseObjectPtr SHAREDSTRINGS::clone()
{
	return BaseObjectPtr(new SHAREDSTRINGS(*this));
}


// SHAREDSTRINGS = SST *Continue
const bool SHAREDSTRINGS::loadContent(BinProcessor& proc)
{
    SST sst(code_page_);
    if(!proc.mandatory(sst))
	{
		return false;
	}
	proc.repeated<Continue>(0, 0);

	size_ = sst.rgb.size();
	return true;
}

int SHAREDSTRINGS::serialize(std::wostream & stream)
{
	for (std::list<XLS::BaseObjectPtr>::iterator it = elements_.begin(); it != elements_.end(); it++)
	{
		(*it)->serialize(stream);
	}
	return 0;
}


} // namespace XLS

