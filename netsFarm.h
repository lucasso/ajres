#ifndef __AJRES_NETS_FARM_H
#define __AJRES_NETS_FARM_H

#include <vector>

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

	std::auto_ptr<LearningFactor> const lFactor;
	std::vector<Net> nets;
	uint32 samplesNum;
	dt weightedPrediction;

public:

	NetsFarm(uint32 const maxTotalNronsNum);
	~NetsFarm();

	dt addNewMeasurementAndGetPrediction(dt const);

};


} // ns ajres

#endif //__AJRES_NETS_FARM_H
