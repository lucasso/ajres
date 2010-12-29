#ifndef __LONG_DATA_READER_H
#define __LONG_DATA_READER_H

#include <vector>
#include "common.h"

namespace ajres
{

struct LongDataItem
	: public Printable
{
	uint32 daySeqNum;
	uint16 year;
	uint16 month; // 1..12
	uint16 dayOfTheMonth; // 1..31

	dt x;
	dt y;

	dt ut1utcDiff;
	dt lod;

	dt dx;
	dt dy;

	dt xErr;
	dt yErr;

	dt ut1utcDiffErr;
	dt lodErr;

	dt dxErr;
	dt dyErr;

	LongDataItem(uint32, uint16, uint16, uint16, dt, dt, dt, dt, dt, dt, dt, dt, dt, dt, dt, dt);

	std::ostream & print(std::ostream &) const;
};


class LongDataReader
{
public:

	typedef auto_ptr<std::vector<LongDataItem> > Items;
	Items readData(string fileName);
};



}

#endif /* __LONG_DATA_READER_H */
