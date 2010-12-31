#ifndef _AJRES_RMLPNET_H_
#define _AJRES_RMLPNET_H_

#include "common.h"
#include <deque>

namespace ajres
{

struct HiddenNron
{
	dt weight;

	typedef std::deque<dt> RecentDiffs;
	RecentDiffs recentOutputDiffsPerOurOutWeight;

};

struct InputNron
{

};

struct GlobalOutputNron
{

};

typedef std::vector<WeightAndDiff> NronsWeightsVec;

class RmlpNet
{
	uint16 const numInputDelayNrons; // without bias
	uint16 const numOutputDelayNrons;
	uint16 const numHiddenNrons; // without bias

	std::vector<HiddenNron> hiddenNrons;
	NronsWeightsVec outputNronsWeights;

	std::vector<dt> delayNronsValues;
	std::vector<dt> hiddenNronsValues;
	dt finalNronValue; // this is also current prediction

	static dt getActivationFunValue(dt const);
	static dt getActivationFunDiff(dt const);

public:

	RmlpNet();

	dt addNewMeasurementAndGetPrediction(dt const);
};


} // ns ajres


#endif /* _AJRES_RMLPNET_H_ */
