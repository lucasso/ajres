#ifndef _AJRES_RMLPNET_H_
#define _AJRES_RMLPNET_H_

#include "common.h"
#include <deque>
#include <vector>
#include <boost/optional.hpp>

namespace ajres
{

struct ActivationFun
{
	static dt Value(dt const);
	static dt Diff(dt const);
};

struct Entry
{
	dt weight;
	dt value;
	boost::optional<dt> dif;

	Entry();
};

class NronInt
{
	dt inputValue;
	dt outputValue;
	dt outputDiff;

public:

	NronInt();

	void setInput(dt const);
	dt getInput() const;
	dt getOutput() const;
	dt getDiff() const;
};

class HiddenNron
{
	std::vector<Entry> inDelays;
	std::vector<Entry> outDelays;

	NronInt nronInt;

	// helpers during computations
	boost::optional<dt> convolution;

	// used during weights update
	typedef std::deque<dt> RecentW2Difs;
	RecentW2Difs recentW2Difs;


public:

	HiddenNron(uint32 const numInDelays, uint32 const numOutDelays);

	dt getConvolutionOfOutputDelayNrosWeightsWithRecentDifs();

	enum DelayNronType
	{
		INPUT_DELAY,
		OUTPUT_DELAY
	};

	template <DelayNronType delayNronType>
	void setW1Dif(uint32 const idx, dt const dif);

	template <DelayNronType delayNronType>
	Entry const & getEntry(uint32 const) const;

	NronInt & getNronInternal();
	NronInt const & getNronInternalConst() const;

private:

	template <DelayNronType delayNronType> std::vector<Entry> & getEntries();
	template <DelayNronType delayNronType> std::vector<Entry> const & getEntries() const;
};

class FinalNron
{
	std::vector<Entry> input;
	NronInt nronInt;

public:

	FinalNron(uint32 const numHidden);

	Entry const & getEntry(uint32 const) const;
	void setW2Dif(uint32 const idx, dt const dif);

	NronInt & getNronInternal();
	NronInt const & getNronInternalConst() const;
};

class RmlpNet
{
	uint16 const numInputDelayNrons; // without bias
	uint16 const numOutputDelayNrons;
	uint16 const numHiddenNrons; // without bias

	std::vector<NronInt> inputDelayNrons;
	std::vector<NronInt> outputDelayNrons;
	std::vector<HiddenNron> hiddenNrons;
	FinalNron finalNron;

	dt calculateW2Diff(HiddenNron const &); // non const cuz it calculates and stores convolution
	dt calculateW1Diff(uint32 const hiddenLayerIdx, dt inputLayerNronValue);
	//dt calculateW1

public:

	RmlpNet();

	dt addNewMeasurementAndGetPrediction(dt const);
};


} // ns ajres


#endif /* _AJRES_RMLPNET_H_ */
