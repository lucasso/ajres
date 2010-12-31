#include "rmlpNet.h"
#include <boost/assert.hpp>

namespace ajres
{

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

//dt
//RmlpNet::

dt
RmlpNet::addNewMeasurementAndGetPrediction(dt const measurement)
{
	// new measurement has come, we may actualize weights

	// calculate outut per w2 weight diffs
	std::vector<dt> newDiffsPerW2;

	BOOST_FOREACH(hiddenNron, this->hiddenNrons)
	{
		dt eq11_bracketSum = hiddenNron.value;

		BOOST_FOREACH(hiddenNronAux, this->hiddenNrons)
		{
			dt eq11_secondSum = 0;

			RecentDiffs::const_reverse_iterator lastDiffsPerNronAuxIt = hiddenNronAux.recentOutputDiffsPerOurW2.rbegin();
			BOOST_FOREACH(weightFromOutDelayNronToNronAux, hiddenNronAux.weightsFromOutDelayNrons)
			{
				eq11_secondSum += weightFromOutDelayNronToNronAux * *lastDiffsPerNronAuxIt;
				++ lastDiffsPerNronAuxIt;
			}

			BOOST_ASSERT(lastDiffsPerNronAuxIt == hiddenNronAux.rend());

			eq11_bracketSum +=
				hiddenNronAux.outWeight *
				RmlpNet::getActivationFunDiff(hiddenNronAux.inputValue) *
				eq11_secondSum;
		}

		newDiffsPerW2.push_back(
			RmlpNet::getActivationFunDiff(this->globalOutputNron) * eq11_bracketSum;
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
