#include "common.h"

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

}
