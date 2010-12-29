#include <boost/foreach.hpp>

#include <iostream>

#include "longDataReader.h"
#include "graphs.h"
#include "analyzer.h"

namespace ajres
{

void mainAjres()
{
	LongDataReader reader;
	LongDataReader::Items items = reader.readData("data/long/eopc04_IAU2000.62-now");

	std::vector<dt> lods;
	std::vector<dt> ut1utcDiffs;

	lods.reserve(items->size());
	ut1utcDiffs.reserve(items->size());

	BOOST_FOREACH(LongDataItem ldi, *items)
	{
		lods.push_back(ldi.lod);
		ut1utcDiffs.push_back(ldi.ut1utcDiff);
		//std::cout << ldi << "\n";
	}

	Graphs::plotGraph1D(lods, 300, 100, "LOD", "lod.png");
	Graphs::plotGraph1D(ut1utcDiffs, 800, 300, "UT1 - UTC", "ut1utcDiff.png");
}


}


int main()
{
	std::cout << "Ajres running...\n";
	ajres::mainAjres();
	return 0;
}
