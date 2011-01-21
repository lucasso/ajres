#include <sstream>
#include <boost/foreach.hpp>

#include <math.h> // ::fabs

#include "netsFarm.h"
#include "rmlpNet.h"

namespace ajres
{

NetsFarm::Net::Net(
	uint32 const inputDelayNronsCount,
	uint32 const outputDelayNronsCount,
	uint32 const hiddenNronsCount,
	LearningFactor & lFactor
) :
	net(new RmlpNet(
		inputDelayNronsCount, outputDelayNronsCount, hiddenNronsCount,
		RandomGenerator::createDefault(), lFactor
	)),
	errorSquares(0),
	prediction(0),
	bestCount(0)
{
}

std::string
NetsFarm::Net::getName() const
{
	return this->net->getName();
}

NetsFarm::NetsFarm(uint32 const maxTotalNronsNumArg, LearningFactor & lFactorArg) :
	lFactor(lFactorArg),
	maxTotalNronsNum(maxTotalNronsNumArg),
	samplesNum(0),
	weightedPrediction(0),
	fStream("out/netsFarmLog.txt")
{
	AJRES_ASSERT(maxTotalNronsNum >= 4, "at least 4 nrons required, but max was set to " << maxTotalNronsNum);

	for (uint32 inputDelayNrons = 2; inputDelayNrons <= (maxTotalNronsNum-2); ++inputDelayNrons)
	{
		for (uint32 outputDelayNrons = 0; outputDelayNrons <= (maxTotalNronsNum-2-inputDelayNrons); ++outputDelayNrons)
		{
			AJRES_ASSERT(inputDelayNrons + outputDelayNrons + 2 <= maxTotalNronsNum,
				"too many delay nrons, at least 2 should be left for hiddenlayer");
			uint32 const hiddenNrons = maxTotalNronsNum - inputDelayNrons - outputDelayNrons;

			this->nets.push_back(Net(inputDelayNrons, outputDelayNrons, hiddenNrons, this->lFactor));
		}
	}
}

dt
NetsFarm::addNewMeasurementAndGetPrediction(dt const measurement)
{
	dt weightsTotalSum = 0;
	dt weightedPredictionsTotalSum = 0;

	if (this->samplesNum > 0)
	{
		this->fStream << "sampleNum:" << this->samplesNum << " predictionWas:" << this->weightedPrediction
			<< ", measurementIs:" << measurement << ", error:" << ::fabs(this->weightedPrediction - measurement) << "\n";

		Net* netOfTheBestCurrentPrediction = NULL;

		BOOST_FOREACH(Net & net, this->nets)
		{
			net.lastPredictionError = ::fabs(measurement - net.prediction);
			dt const currentPredictionWeight = 1.0 / (net.lastPredictionError*net.lastPredictionError);
			net.errorSquares += currentPredictionWeight;

			if (netOfTheBestCurrentPrediction == NULL ||
					netOfTheBestCurrentPrediction->lastPredictionError > net.lastPredictionError)
			{
				netOfTheBestCurrentPrediction = &net;
			}

			net.prediction = net.net->addNewMeasurementAndGetPrediction(measurement);
			weightsTotalSum += net.errorSquares;
			weightedPredictionsTotalSum += net.errorSquares * net.prediction;
		}

		this->fStream << "best prediction was made by " << netOfTheBestCurrentPrediction->getName()
			<< ", error:" << netOfTheBestCurrentPrediction->lastPredictionError << "\n";
		++ netOfTheBestCurrentPrediction->bestCount;
	}
	else
	{
		BOOST_FOREACH(Net & net, this->nets)
		{
			net.prediction = net.net->addNewMeasurementAndGetPrediction(measurement);
			weightsTotalSum += 1.0;
			weightedPredictionsTotalSum += net.prediction;
		}
	}
	++ this->samplesNum;

	this->weightedPrediction = weightedPredictionsTotalSum / weightsTotalSum;
	return this->weightedPrediction;
}

std::string
NetsFarm::getName() const
{
	std::ostringstream oss;
	oss << "NetsFarm{max:" << this->maxTotalNronsNum << "}";
	return oss.str();
}

NetsFarm::~NetsFarm()
{
	std::sort(this->nets.begin(), this->nets.end());

	BOOST_FOREACH(Net & net, this->nets)
	{
		this->fStream << "net:" << net.getName() << ", bestCount:" << net.bestCount << "\n";
		delete net.net;
	}

	this->fStream.close();
}



} // ns ajres
