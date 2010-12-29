#ifndef __ARES_COMMON_H
#define __ARES_COMMON_H

#include <string>
#include <memory>

namespace ajres
{

typedef unsigned int uint32;
typedef unsigned short int uint16;
typedef double dt;

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


}

#endif /* __ARES_COMMON_H */
