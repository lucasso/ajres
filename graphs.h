#ifndef __GRAPHS_H_
#define __GRAPHS_H_

#include <vector>

#include "common.h"

namespace ajres
{

struct Graphs
{
	static void plotGraph1D(
		std::vector<dt> const &, uint32 const xSize, uint32 const ySize,
		std::string const & graphTitle, std::string const & fileName,
		bool const dumpBinaryData = false
	);
};

}


#endif // __GRAPHS_H_
