#include "rmlpNet.h"
#include <boost/assert.hpp>
#include <boost/foreach.hpp>

namespace ajres
{

Entry::Entry() :
	weight(0),
	value(0)
{
}

// ---------------------------------------------------------
// ----------------------- NronInt -------------------------
// ---------------------------------------------------------

NronInt::NronInt()
{
	this->setInput(0);
}

void
NronInt::setInput(dt const newValue)
{
	this->inputValue = newValue;
	this->outputValue = ActivationFun::Value(this->inputValue);
	this->outputDiff = ActivationFun::Diff(this->inputValue);
}

dt
NronInt::getInput() const
{
	return this->inputValue;
}

dt
NronInt::getOutput() const
{
	return this->outputValue;
}

dt
NronInt::getDiff() const
{
	return this->outputDiff;
}

// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------------------------------------------

HiddenNron::HiddenNron(uint32 const numInDelays, uint32 const numOutDelays) :
	inDelays(numInDelays),
	outDelays(numOutDelays),
	recentW2Difs(numOutDelays, 0)
{
}

NronInt &
HiddenNron::getNronInternal()
{
	return this->nronInt;
}

NronInt const &
HiddenNron::getNronInternalConst() const
{
	return this->nronInt;
}

template <> std::vector<Entry> &
HiddenNron::getEntries<INPUT_DELAY>()
{
	return this->inDelays;
}

template <> std::vector<Entry> &
HiddenNron::getEntries<OUTPUT_DELAY>()
{
	return this->outDelays;
}

template <> std::vector<Entry> const &
HiddenNron::getEntries<INPUT_DELAY>() const
{
	return this->inDelays;
}

template <> std::vector<Entry> const &
HiddenNron::getEntries<OUTPUT_DELAY>() const
{
	return this->outDelays;
}

dt
HiddenNron::getConvolutionOfOutputDelayNrosWeightsWithRecentDifs()
{
	if (!this->convolution.is_initialized())
	{
		dt result = 0;

		RecentW2Difs::const_reverse_iterator lastDifsIt = this->recentW2Difs.rbegin();
		BOOST_FOREACH(Entry const & outDelayEntry, this->outDelays)
		{
			result += outDelayEntry.weight * (*lastDifsIt);
			++ lastDifsIt;
		}

		BOOST_ASSERT(lastDifsIt == this->recentW2Difs.rend());
		this->convolution = boost::optional<dt>(result);
	}

	return this->convolution.get();
}

template <DelayNronType delayNronType> void
HiddenNron::setW1Dif(uint32 const idx, dt const dif)
{
	std::vector<Entry> & entriesVec = this->getEntries<delayNronType>();
	BOOST_ASSERT(idx < entriesVec.size());
	boost::optional<dt> & difOpt = entriesVec.at(idx).dif;

	BOOST_ASSERT(!difOpt.is_initialized());
	difOpt = boost::optional<dt>(dif);
}

template <DelayNronType delayNronType> Entry const &
HiddenNron::getEntryFromInputDelay(uint32 const idx) const
{
	return this->getEntries<delayNronType>.at(idx);
}

// ---------------------------------------------------------
// ----------------------- Final Nron ----------------------
// ---------------------------------------------------------

FinalNron::FinalNron(uint32 const numHidden) :
	input(numHidden)
{
}

Entry const &
FinalNron::getEntry(uint32 const idx) const
{
	return this->input.at(idx);
}

void
FinalNron::setW2Dif(uint32 const idx, dt const dif)
{
	BOOST_ASSERT(idx < this->input.size());
	boost::optional<dt> & difOpt = this->input.at(idx).dif;

	BOOST_ASSERT(!difOpt.is_initialized());
	difOpt = boost::optional<dt>(dif);
}

NronInt &
FinalNron::getNronInternal()
{
	return this->nronInt;
}

NronInt const &
FinalNron::getNronInternalConst() const
{
	return this->nronInt;
}

// ---------------------------------------------------------
// --------------------- ActivationFun ---------------------
// ---------------------------------------------------------

dt
ActivationFun::Value(dt const x)
{
	return x;
}

dt
ActivationFun::Diff(dt const)
{
	return 1;
}

// ---------------------------------------------------------
// ------------------------- RmlpNet -----------------------
// ---------------------------------------------------------

RmlpNet::RmlpNet() :
	numInputDelayNrons(30),
	numOutputDelayNrons(5),
	numHiddenNrons(10),
	inputDelayNrons(this->numInputDelayNrons),
	outputDelayNrons(this->numOutputDelayNrons),
	hiddenNrons(this->numHiddenNrons, HiddenNron(this->numInputDelayNrons, this->numOutputDelayNrons)),
	finalNron(this->numHiddenNrons)
{
}

dt
RmlpNet::calculateImpl(
	bool const includeHiddenLayersBias,
	dt const hiddenLayerSumAddon,
	uint32 const convolutionAddonIdx,
	dt const convolutionAddonValue
)
{
	dt result = hiddenLayerSumAddon;

	BOOST_ASSERT(!this->hiddenNrons.empty());
	std::vector<HiddenNron>::iterator const itEnd = this->hiddenNrons.end();
	std::vector<HiddenNron>::iterator it = this->hiddenNrons.begin();
	uint32 idx = 0;

	if (!includeHiddenLayerBias)
	{
		++ it;
		idx = 1;
		BOOST_ASSERT(convolutionAddonIdx != 0);
	}

	BOOST_ASSERT(it != itEnd);

	do
	{
		result +=
			this->finalNron.getEntry(idx).weight *
			it->getNronInternalConst().getDiff() *
			(it->getConvolutionOfOutputDelayNrosWeightsWithRecentDifs()
			+ (idx == convolutionAddonIdx ? convolutionAddonValue : 0.0));
		++ idx;
	}
	while ( ++it != itEnd );

	result *= this->finalNron.getNronInternalConst().getDiff();
	return result; //RmlpNet::getActivationFunDiff
}

dt
RmlpNet::calculateW2Diff(HiddenNron const & nron)
{
	return this->calculateImpl(
		true,
		nron.getNronInternalConst().getOutput(),
		0, 0.0
	);
}

dt
RmlpNet::calculateW1Diff(uint32 const hiddenLayerIdx, dt inputLayerNronValue)
{
	BOOST_ASSERT(hiddenLayerIdx > 1);
	BOOST_ASSERT(hiddenLayerIdx < this->hiddenNrons.size());

	return this->calculateImpl(
		false,
		0,
		hiddenLayerIdx, inputLayerNronValue
	);
}

void
RmlpNet::setW1Difs(std::vector<NronInt> const & delayNrons)
{
	for (uint32 delayIdx = 0; delayIdx < delayNrons.size(); ++ delayIdx)
	{
		for (uint32 hiddenLayerIdx = 1; hiddenLayerIdx < this->hiddenNrons.size(); ++ hiddenLayerIdx)
		{
			this->hiddenNrons.at(hiddenLayerIdx).setW1DifFromInputDelay(
				inputLayerIdx,
				this->calculateW1Diff(hiddenLayerIdx, inputDelayNron.outputValue)
			);
		}
	}

dt
RmlpNet::addNewMeasurementAndGetPrediction(dt const measurement)
{
	// new measurement has come, we may actualize weights

	// calculate outut per w2 weight diffs
	uint32 idx = 0;

	BOOST_FOREACH(HiddenNron const & hiddenNron, this->hiddenNrons)
	{
		this->finalNron.setW2Dif(idx, this->calculateW2Diff(hiddenNron));
		++ idx;
	}



	for (uint32 outputLayerIdx = 0; outputLayerIdx < this->outputDelayNrons.size(); ++ inputLayerIdx)
	{
		for (uint32 hiddenLayerIdx = 1; hiddenLayerIdx < this->hiddenNrons.size(); ++ hiddenLayerIdx)
		{
			this->hiddenNrons.at(hiddenLayerIdx).setW1DifFromInputDelay(
				inputLayerIdx,
				this->calculateW1Diff(hiddenLayerIdx, inputDelayNron.outputValue)
			);
		}
	}

//
//	BOOST_FOREACH(iputDelayNron, this->inputDelayNrons)
//	{
//
//	}

	return 0;
}



} // ns ajres
