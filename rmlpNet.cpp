#include "rmlpNet.h"
#include <boost/assert.hpp>
#include <boost/foreach.hpp>

namespace ajres
{

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



// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------------------------------------------

FinalNron::FinalNron(uint32 const numHidden)
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
	boost::optional<dt> & difOpt = this->input[idx].dif;

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
RmlpNet::calculateW2Diff(HiddenNron const & nron)
{
	dt result = nron.getNronInternalConst().getOutput();

	uint32 idx = 0;
	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		result +=
			this->finalNron.getEntry(idx).weight *
			hiddenNron.getNronInternalConst().getDiff() *
			hiddenNron.getConvolutionOfOutputDelayNrosWeightsWithRecentDifs();
	}

	result *= this->finalNron.getNronInternalConst().getDiff();
	return result; //RmlpNet::getActivationFunDiff
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

//	BOOST_FOREACH(iputDelayNron, this->inputDelayNrons)
//	{
//
//	}
//
//	BOOST_FOREACH(iputDelayNron, this->inputDelayNrons)
//	{
//
//	}

	return 0;
}



} // ns ajres
