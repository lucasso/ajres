#ifndef __AJRES_NETS_FARM_H
#define __AJRES_NETS_FARM_H

#include <vector>
#include <fstream>

#include "common.h"

namespace ajres
{

class RmlpNet;
class LearningFactor;

class NetsFarm
{
	struct Net
	{
		RmlpNet* net;
		dt errorSquares;
		dt prediction;
		dt lastPredictionError;
		uint32 bestCount;

		std::string getName() const;

		bool operator<(Net const & other) const { return this->bestCount < other.bestCount; }

		Net(
			uint32 const inputDelayNronsCount,
			uint32 const outputDelayNronsCount,
			uint32 const hiddenNronsCount,
			LearningFactor &
		);
	};

	LearningFactor & lFactor;
	std::vector<Net> nets;
	uint32 const maxTotalNronsNum;
	uint32 samplesNum;
	dt weightedPrediction;

	std::ofstream fStream;

public:

	NetsFarm(uint32 const maxTotalNronsNum, LearningFactor &);
	~NetsFarm();

	dt addNewMeasurementAndGetPrediction(dt const);

	std::string getName() const;
};


} // ns ajres

#endif //__AJRES_NETS_FARM_H
