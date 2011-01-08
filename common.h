#ifndef __ARES_COMMON_H
#define __ARES_COMMON_H

#include <string>
#include <ostream>
#include <memory>
#include <boost/optional.hpp>

namespace ajres
{

typedef unsigned int uint32;
typedef unsigned short int uint16;
typedef double dt;
typedef int int32;

using std::string;
using std::auto_ptr;

class Printable
{
public:
	~Printable();
	virtual std::ostream & print(std::ostream &) const = 0;
};

inline std::ostream & operator << (std::ostream & osek, Printable const & prtble)
{
	return prtble.print(osek);
}

std::ostream & operator << (std::ostream &, boost::optional<dt> const &);

template <class T, char separator = ','>
class ContainerPrinter : public Printable
{
	T const & container;
public:
	ContainerPrinter(T const & containerArg) :
		container(containerArg)
	{
	}

	virtual std::ostream & print(std::ostream & osek) const
	{
		if (this->container.empty())
		{
			osek << "empty";
			return osek;
		}

		typename T::const_iterator it = this->container.begin();
		typename T::const_iterator itEnd = this->container.end();
		uint32 idx = 0;

		while (1)
		{
			osek << idx << ":" << *it;
			if (++ it != itEnd)
			{
				osek << separator;
				++idx;
			}
			else
			{
				return osek;
			}
		}
	}
};

#define AJRES_ASSERT(exp, log) \
{ \
	if (!(exp)) { \
		std::cout << "ASSERTION FAILED: " << log << "\n" << backtrace() << "\n"; \
		BOOST_ASSERT(exp); \
	} \
}



}

#endif /* __ARES_COMMON_H */
