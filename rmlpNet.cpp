#include "rmlpNet.h"
#include <boost/assert.hpp>

namespace ajres
{

dt
HiddenNron::getConvolutionOfOutputDelayNrosWeightsWithRecentDifs()
{
	if (!this->convolution.is_initialized())
	{
		dt result = 0;

		RecentW2Difs::const_reverse_iterator lastDifsIt = this->recentW2Difs.rbegin();
		BOOST_FOREACH(outDelay, this->outDelays)
		{
			result += outDelay.weight * (*lastDifsIt);
			++ lastDiffsPerNronAuxIt;
		}

		BOOST_ASSERT(lastDifsIt == this->recentW2Difs.rend());
		this->convolution = boost::optional<dt>(result);
	}

	return this->convolution.get();
}

void
HiddenNron::setOutputDif(dt const newOutputDif)
{
	BOOST_ASSERT(!this->finalOut.dif.is_initialized());
	this->finalOut.dif = boost::optional<dt>(newOutputDif);
}


dt
RmlpNet::getActivationFunValue(dt const)
{
	return dt;
}

dt
RmlpNet::getActivationFunDiff(dt const)
{
	return 1;
}

RmlpNet::RmlpNet() :
	numInputDelayNrons(30),
	numOutputDelayNrons(5),
	numHiddenNrons(10)
{
}

void
RmlpNet::calculateW2Diff(HiddenNron & nron)
{
	dt result = nron.getOuputValue();

	BOOST_FOREACH(HiddenNron const & hiddenNron, this->hiddenNrons)
	{
		result +=
			hiddenNron.getOutputWeight() *
			RmlpNet::getActivationFunDiff(hiddenNron.getInputValue()) *
			hiddenNron.getConvolutionOfOutputDelayNrosWeightsWithRecentDifs();
	}

	nron.setOutputDif(RmlpNet::getActivationFunDiff(this->globalOutputNron) * result);
}

dt
RmlpNet::addNewMeasurementAndGetPrediction(dt const measurement)
{
	// new measurement has come, we may actualize weights

	// calculate outut per w2 weight diffs
	std::vector<dt> newDiffsPerW2;

	BOOST_FOREACH(hiddenNron, this->hiddenNrons)
	{
		newDiffsPerW2.push_back(

					);

	}

	BOOST_FOREACH(iputDelayNron, this->inputDelayNrons)
	{

	}

	BOOST_FOREACH(iputDelayNron, this->inputDelayNrons)
	{

	}
}



} // ns ajres
