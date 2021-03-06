#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <fstream>

#include <math.h>

#include "longDataReader.h"
#include "graphs.h"
#include "analyzer.h"
#include "rmlpNet.h"
#include "netsFarm.h"
#include "leastSquares.h"

namespace ajres
{

namespace po = boost::program_options;

enum AjresMode
{
	SINGLE_NET,
	ARCHITECTURE_SEARCH,
	LEAST_SQUARES
};

class AvgComputer
{
	dt sum;
	uint32 count;
public:

	AvgComputer() :
		sum(0.0),
		count(0)
	{}

	void add(dt const val)
	{
		this->sum += val;
		++ this->count;
	}

	dt getAvg() const
	{
		if (this->count) return this->sum / this->count;
		return 0.0;
	}
};

template <class PredictionMachineT>
void ajresLoop(PredictionMachineT & predictionMachine, std::vector<dt> const & input, bool const plotErrors, dt const scale)
{
	dt previousPrediction = 0;

	std::vector<dt> errors;
	errors.reserve(input.size() - 1);

	std::ofstream fStream("out/predictionsLog.txt");
	AvgComputer avgPercentError, avgRealError;

	uint32 idx = 0;
	BOOST_FOREACH(dt inputValueBeforeScaling, input)
	{
		dt const inputValue = inputValueBeforeScaling * scale;

		if (idx)
		{
			dt const predictionError = ::fabs(previousPrediction - inputValue);
			dt const predictionErrorNonScaled = predictionError/scale;
			dt const previousPredictionNonScaled = previousPrediction/scale;
			dt const predictionErrorPercent = inputValue == 0.0 ? 0.0 : predictionError * 100.0 / ::fabs(inputValue);
			fStream << "prediction no " << idx << " was " << previousPredictionNonScaled << " expected:" << inputValueBeforeScaling
				<< ", predictionError:" << predictionErrorNonScaled << ", errorPercent:" << predictionErrorPercent << "\n";
			errors.push_back(idx<20 ? 0 : predictionErrorNonScaled);

			avgPercentError.add(predictionErrorPercent);
			avgRealError.add(predictionErrorNonScaled);
		}
		previousPrediction = predictionMachine.addNewMeasurementAndGetPrediction(inputValue);
		++ idx;
	}

	fStream << "AVG, percentError:" << avgPercentError.getAvg() << ", realError:" << avgRealError.getAvg() << "\n";

	fStream.close();

	if (plotErrors)
	{
		std::string const plotName = "prediction errors [s] " + predictionMachine.getName();
		Graphs::plotGraph1D(errors, 3000, 1000, plotName.c_str(), "predictionErrors.png", true, true);
	}
}


void mainAjres(int ac, char** av)
{
	uint32 inputDelayNronsNum;
	uint32 outputDelayNronsNum;
	uint32 hiddenNronsNum;
	uint32 maxNronsIfSearch = 0;
	uint32 inputRecordsLimit;
	uint32 leastSquaresNumX = 2;
	dt scale = 1.0;
	std::string inputData;

	AjresMode ajresMode = SINGLE_NET;

	bool plotInputDataFlag = false;
	bool plotPredictionErrorFlag = false;
	bool plotLearningFactorComputationFlag = false;

	po::options_description desc("AJRES - length-of-day prediction by Artificial Neural Networks");
	desc.add_options()
	    ("help", "produce help message")
	    ("inputDelayNronsNum", po::value<uint32>(&inputDelayNronsNum)->default_value(2u),
	    	"set number of input delay neurons including bias, min 2, default 2")
	    ("outputDelayNronsNum", po::value<uint32>(&outputDelayNronsNum)->default_value(1u),
	    	"set number of output delay neurons, may be 0, default 1")
	    ("hiddenNronsNum", po::value<uint32>(&hiddenNronsNum)->default_value(2u),
	    	"set number of hidden layer neurons including bias, min 2, default 2")
	    ("searchForBestArchitecture", po::value<uint32>(),
	    	"if set parallel computations are performed on many nets and summary is printed, integer argument is the max total number of neurons, at least 4")
	    ("useLeastSquaresFitting", po::value<uint32>(),
	    	"if set no NeuralNetworks are used but we use LS fitting using last arg measurements")
	    ("inputData", po::value<std::string>(&inputData)->default_value("data/long/eopc04_IAU2000.62-now"),
	    	"file with long term earth orientation data in IAU2000A format, for details see http://www.iers.org/IERS/EN/DataProducts/EarthOrientationData/eop.html?__nnn=true")
	    ("plotInputData", "create graph of LOD parameter from input file")
	    ("plotPredictionError", "create graph of prediction errors")
	    ("plotLearningFactorComputation",
	    	"nn implementation related plots of computing optimal learningFactor at each weight update step")
	    ("inputRecordsLimit", po::value<uint32>(&inputRecordsLimit)->default_value(0u),
	    	"limit input data to the specified number of records, 0 means no limit, default 0")
	    ("scale", po::value<dt>(&scale)->default_value(1.0),
	       	"scale input data, by multiplying them by that scale factor, cuz nrons output is always between -1 and 1")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(ac, av, desc), vm);
	po::notify(vm);

	if (vm.count("help")) {
	    std::cout << desc << "\n";
	    return;
	}

	if (vm.count("useLeastSquaresFitting"))
	{
		ajresMode = LEAST_SQUARES;
		leastSquaresNumX = vm["useLeastSquaresFitting"].as<uint32>();
	}
	else if (vm.count("searchForBestArchitecture"))
	{
	    ajresMode = ARCHITECTURE_SEARCH;
	    maxNronsIfSearch = vm["searchForBestArchitecture"].as<uint32>();
	    std::cout << "ajres, ARCHITECTURE_SEARCH mode enabled, maxNrons:" << maxNronsIfSearch << "\n";
	}
	else
	{
	    std::cout << "ajres, SINGLE_NET mode enabled, inputDelayNronsNum:" << inputDelayNronsNum
	    	<< ", outputDelayNronsNum:" << outputDelayNronsNum << ", hiddenNronsNum:" << hiddenNronsNum << "\n";
	}

	if (vm.count("plotInputData")) plotInputDataFlag = true;
	if (vm.count("plotPredictionError")) plotPredictionErrorFlag = true;
	if (vm.count("plotLearningFactorComputation")) plotLearningFactorComputationFlag = true;

	std::cout << "plots, inputData:" << (plotInputDataFlag?"T":"F")
		<< ", predictionErrors:" << (plotPredictionErrorFlag?"T":"F")
		<< ", learningFactorComputation:" << (plotLearningFactorComputationFlag?"T":"F") << "\n";

	LongDataReader reader;
	LongDataReader::Items items = reader.readData(inputData);

	std::vector<dt> lods;
	//std::vector<dt> ut1utcDiffs;

	lods.reserve(items->size());
	//ut1utcDiffs.reserve(items->size());

	uint32 idx = 0;
	BOOST_FOREACH(LongDataItem ldi, *items)
	{
		if (inputRecordsLimit == 0 || idx < inputRecordsLimit )
		{
			lods.push_back(ldi.lod);
			//ut1utcDiffs.push_back(ldi.ut1utcDiff);
			++ idx;
		}
		//std::cout << ldi << "\n";
	}

	if (plotInputDataFlag)
	{
		Graphs::plotGraph1D(lods, 3000, 1000, "LOD [s]", "lod.png", true, false);
		//Graphs::plotGraph1D(ut1utcDiffs, 800, 300, "UT1 - UTC", "ut1utcDiff.png");
	}

	std::auto_ptr<LearningFactor> lFactor =
		LearningFactorCreate::createBisectionBasedLearningFactor(plotLearningFactorComputationFlag);

	switch (ajresMode)
	{
	case SINGLE_NET:
	{
		RmlpNet rmlpNet(
			inputDelayNronsNum,
			outputDelayNronsNum,
			hiddenNronsNum,
			RandomGenerator::createDefault(),
			*lFactor
		);

		ajresLoop(rmlpNet, lods, plotPredictionErrorFlag, scale);
		return;
	}
	case ARCHITECTURE_SEARCH:
	{
		NetsFarm netsFarm(
			maxNronsIfSearch,
			*lFactor
		);
		ajresLoop(netsFarm, lods, plotPredictionErrorFlag, scale);
		return;
	}
	case LEAST_SQUARES:
	{
		LeastSquares ls(leastSquaresNumX);
		ajresLoop(ls, lods, plotPredictionErrorFlag, scale);
		return;
	}
	}

	AJRES_ASSERT(false, "unexpected ajresMode:" << static_cast<uint32>(ajresMode));
}


}


int main(int ac, char** av)
{
	ajres::mainAjres(ac, av);
	return 0;
}
