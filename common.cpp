#include "common.h"
#include <execinfo.h>
#include <sstream>

namespace ajres
{

Printable::~Printable()
{
}

std::ostream & operator << (std::ostream & osek, boost::optional<dt> const & val)
{
	if (val.is_initialized())
	{
		osek << (*val);
	}
	else
	{
		osek << "not-initialized";
	}
	return osek;
}

std::string backtrace()
{
	enum Constants {
		BUFFER_SIZE = 128
	};

	void* funPtrBuffer[BUFFER_SIZE];
	int const numFunctions = ::backtrace(funPtrBuffer, BUFFER_SIZE);

	char** funStrings = ::backtrace_symbols(funPtrBuffer, numFunctions);

	std::ostringstream osek;
	if (funStrings == NULL)
	{
		osek << "?";
	}
	else
	{
		for (int32 i = 1; i < numFunctions; ++i)
		{
			osek << funStrings[i] << "\n";
		}
	}

	::free(funStrings);

	return osek.str();
}

}
