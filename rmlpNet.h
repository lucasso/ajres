#ifndef _AJRES_RMLPNET_H_
#define _AJRES_RMLPNET_H_

#include "common.h"
#include <deque>
#include <boost/optional.hpp>

namespace ajres
{

struct WeightWithDif
{
	dt weight;
	boost::optional<dt> dif;
};

struct HiddenNron
{
	std::vector<WeightWithDif> inDelays;
	std::vector<WeightWithDif> outDelays;
	WeightWithDif finalOut;

	// helpers during computations
	boost::optional<dt> convolution;

	typedef std::deque<dt> RecentW2Difs;
	RecentDiffs recentW2Difs;

public:

	dt getConvolutionOfOutputDelayNrosWeightsWithRecentDifs();

	void setOutputDif(dt const);
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

	dt calculateW2Diff(HiddenNron const &) const;
	dt calculateW1

public:

	RmlpNet();

	dt addNewMeasurementAndGetPrediction(dt const);
};


} // ns ajres


#endif /* _AJRES_RMLPNET_H_ */
