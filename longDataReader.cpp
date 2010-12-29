#include "longDataReader.h"
#include <fstream>

namespace ajres
{

LongDataItem::LongDataItem(
	uint32 daySeqNumArg,
	uint16 yearArg,
	uint16 monthArg,
	uint16 dayOfTheMonthArg,
	dt xArg,
	dt yArg,
	dt ut1utcDiffArg,
	dt lodArg,
	dt dxArg,
	dt dyArg,
	dt xErrArg,
	dt yErrArg,
	dt ut1utcDiffErrArg,
	dt lodErrArg,
	dt dxErrArg,
	dt dyErrArg
) :
	daySeqNum(daySeqNumArg),
	year(yearArg),
	month(monthArg),
	dayOfTheMonth(dayOfTheMonthArg),
	x(xArg),
	y(yArg),
	ut1utcDiff(ut1utcDiffArg),
	lod(lodArg),
	dx(dxArg),
	dy(dyArg),
	xErr(xErrArg),
	yErr(yErrArg),
	ut1utcDiffErr(ut1utcDiffErrArg),
	lodErr(lodErrArg),
	dxErr(dxErrArg),
	dyErr(dyErrArg)
{
}

std::ostream &
LongDataItem::print(std::ostream & osek) const
{
	osek << "daySeqNum:" << this->daySeqNum
		<< ", year:"  << this->year
		<< ", month:" << this->month
		<< ", dayOfTheMonth:" << this->dayOfTheMonth
		<< ", x:" << this->x
		<< ", y:" << this->y
		<< ", ut1utcDiff:" << this->ut1utcDiff
		<< ", lod:" << this->lod
		<< ", dx:" << this->dx
		<< ", dy:" << this->dy
		<< ", xErr:" << this->xErr
		<< ", yErr:" << this->yErr
		<< ", ut1utcDiffErr:" << this->ut1utcDiffErr
		<< ", lodErr:" << this->lodErr
		<< ", dxErr:" << this->dxErr
		<< ", dyErr:" << this->dyErr;
	return osek;
}

LongDataReader::Items
LongDataReader::readData(string fileName)
{
	enum {
		BUFSIZE = 1024,
		IGNORED_LINES_COUNT = 14
	};

	char buf[BUFSIZE];
	std::ifstream inputFile(fileName.c_str());

	for (uint16 i = 0; i < IGNORED_LINES_COUNT; ++ i) {
		inputFile.getline(buf, BUFSIZE, '\n');
	}

	std::vector<LongDataItem> * items = new std::vector<LongDataItem>;

	while (inputFile)
	{
		uint32 daySeqNum;
		uint16 year;
		uint16 month; // 1..12
		uint16 dayOfTheMonth; // 1..31

		inputFile >> year;
		inputFile >> month;
		inputFile >> dayOfTheMonth;
		inputFile >> daySeqNum;

		dt x;
		dt y;

		inputFile >> x;
		inputFile >> y;

		dt ut1utcDiff;
		dt lod;

		inputFile >> ut1utcDiff;
		inputFile >> lod;

		dt dx;
		dt dy;

		inputFile >> dx;
		inputFile >> dy;

		dt xErr;
		dt yErr;

		inputFile >> xErr;
		inputFile >> yErr;

		dt ut1utcDiffErr;
		dt lodErr;

		inputFile >> ut1utcDiffErr;
		inputFile >> lodErr;

		dt dxErr;
		dt dyErr;

		inputFile >> dxErr;
		inputFile >> dyErr;

		items->push_back(LongDataItem(
			daySeqNum, year, month, dayOfTheMonth,
			x, y,
			ut1utcDiff, lod,
			dx, dy,
			xErr, yErr,
			ut1utcDiffErr, lodErr,
			dxErr, dyErr
		));
	}

	return Items(items);
}

}
