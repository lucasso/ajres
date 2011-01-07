#include "rmlpNet.h"

#include <boost/assert.hpp>
#include <boost/foreach.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>

namespace ajres
{

// ---------------------------------------------------------
// ------------------- Random Generator --------------------
// ---------------------------------------------------------

class RandomGenerator
{
	enum Constants
	{
		GRANULARITY = 1000,
		MAX_CHANGE = 3 // FIXME: should go from parameters or config
	};
	boost::mt19937 genSource;
	boost::uniform_int<int> genDistrib;
	boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > gen;

public:

	RandomGenerator();
	dt getWeight();
};

RandomGenerator::RandomGenerator() :
	genSource(),
	genDistrib(-GRANULARITY,GRANULARITY),
	gen(this->genSource, this->genDistrib)
{
}

dt
RandomGenerator::getWeight()
{
	int32 const randomNum = this->gen();
	BOOST_ASSERT(randomNum >= -1000);
	BOOST_ASSERT(randomNum <= 1000);
	dt const factor = (randomNum == 0) ? 1.0 : static_cast<dt>(randomNum * MAX_CHANGE) / GRANULARITY;
	dt const retVal = (factor < 0)
		? static_cast<dt>(1.0) / (-factor)
		: static_cast<dt>(1.0) * factor;

	std::cout << "randomNum:" << randomNum << ", factor:" << factor << ", retVal:" << retVal << std::endl;
	return retVal;
}

// ---------------------------------------------------------
// ------------------------ Entry --------------------------
// ---------------------------------------------------------

Entry::Entry(RandomGenerator & gen) :
	weight(gen.getWeight())
{
}

void
Entry::weightUpdateHelper(std::vector<Entry> & entries, dt const factor)
{
	BOOST_FOREACH(Entry & entry, entries)
	{
		BOOST_ASSERT(entry.dif.is_initialized());
		entry.weight += factor * (*entry.dif);
	}
}

void
Entry::difsResetHelper(std::vector<Entry> & entries)
{
	BOOST_FOREACH(Entry & entry, entries)
	{
		BOOST_ASSERT(entry.dif.is_initialized());
		entry.dif = boost::optional<dt>();
	}
}
std::ostream & operator << (std::ostream & osek, Entry const & entry)
{
	osek << "Entry{weight:" << entry.weight << ", dif:" << entry.dif << "}";
	return osek;
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

std::ostream & operator << (std::ostream & osek, NronInt const & nronInt)
{
	osek << "NronInt{i:" << nronInt.getInput() << ",o:" << nronInt.getOutput() << ",d:" << nronInt.getDiff() << "}";
	return osek;
}

// ---------------------------------------------------------
// ------------------- Hidden Nron -------------------------
// ---------------------------------------------------------

HiddenNron::HiddenNron(uint32 const numInDelays, uint32 const numOutDelays, RandomGenerator & gen) :
	inDelays(), // can not use (num, obj) ctor because nrons have to have different weights
	outDelays(),
	recentW2Difs(numOutDelays, 0)
{
	this->inDelays.reserve(numInDelays);
	this->outDelays.reserve(numOutDelays);
	for (uint32 i=0; i<numInDelays; ++i) this->inDelays.push_back(Entry(gen));
	for (uint32 i=0; i<numOutDelays; ++i) this->outDelays.push_back(Entry(gen));
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
	std::vector<Entry> & entriesVec = this->template getEntries<delayNronType>();
	BOOST_ASSERT(idx < entriesVec.size());
	boost::optional<dt> & difOpt = entriesVec.at(idx).dif;

	BOOST_ASSERT(!difOpt.is_initialized());
	difOpt = boost::optional<dt>(dif);
}

void
HiddenNron::resetDifs()
{
	Entry::difsResetHelper(this->inDelays);
	Entry::difsResetHelper(this->outDelays);
}

template <DelayNronType delayNronType> Entry const &
HiddenNron::getEntry(uint32 const idx) const
{
	return this->template getEntries<delayNronType>().at(idx);
}

void
HiddenNron::updateWeights(dt const factor)
{
	Entry::weightUpdateHelper(this->inDelays, factor);
	Entry::weightUpdateHelper(this->outDelays, factor);
}

void
HiddenNron::addRecentW2Dif(dt const val)
{
	BOOST_ASSERT(!this->recentW2Difs.empty()); // should be equal to num of output signal delays
	this->recentW2Difs.push_front(val);
	this->recentW2Difs.pop_back();
}

std::ostream & operator << (std::ostream & osek, HiddenNron const & hiddenNron)
{
	osek << "HiddenNron{" << hiddenNron.nronInt << ",conv:" << hiddenNron.convolution
		<< "\n   inDelays:" << ContainerPrinter<std::vector<Entry>,'\n'>(hiddenNron.inDelays)
		<< "\n   outDelays:" << ContainerPrinter<std::vector<Entry>,'\n'>(hiddenNron.outDelays)
		<< "\n   recentW2Difs:" << ContainerPrinter<std::deque<dt> >(hiddenNron.recentW2Difs) << "}";
	return osek;
}

// ---------------------------------------------------------
// ----------------------- Final Nron ----------------------
// ---------------------------------------------------------

FinalNron::FinalNron(uint32 const numHidden, RandomGenerator & gen) :
	input()
{
	this->input.reserve(numHidden);
	for (uint32 i=0; i<numHidden; ++i) this->input.push_back(Entry(gen));
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

void
FinalNron::resetDifs()
{
	Entry::difsResetHelper(this->input);
}

void
FinalNron::updateWeights(dt const factor)
{
	Entry::weightUpdateHelper(this->input, factor);
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

std::ostream & operator << (std::ostream & osek, FinalNron const & finalNron)
{
	osek << "FinalNron{" << finalNron.getNronInternalConst()
		<< ",input:" << ContainerPrinter<std::vector<Entry> >(finalNron.input) << "}";
	return osek;
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
	randomGenerator(new RandomGenerator),
	inputDelayNrons(this->numInputDelayNrons),
	outputDelayNrons(this->numOutputDelayNrons),
	hiddenNrons(this->numHiddenNrons, HiddenNron(this->numInputDelayNrons, this->numOutputDelayNrons, *this->randomGenerator)),
	finalNron(this->numHiddenNrons, *this->randomGenerator),
	learningFactor(0.1)
{
	// random initialization of weights

	RandomGenerator gen;

}

RmlpNet::~RmlpNet()
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

	if (!includeHiddenLayersBias)
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

template <DelayNronType delayNronType> void
RmlpNet::setW1Difs(std::vector<NronInt> const & delayNrons)
{
	for (uint32 delayIdx = 0; delayIdx < delayNrons.size(); ++ delayIdx)
	{
		dt const delayNronOutputValue = delayNrons.at(delayIdx).getOutput();

		for (uint32 hiddenLayerIdx = 1; hiddenLayerIdx < this->hiddenNrons.size(); ++ hiddenLayerIdx)
		{
			this->hiddenNrons.at(hiddenLayerIdx).template setW1Dif<delayNronType>(
				delayIdx,
				this->calculateW1Diff(hiddenLayerIdx, delayNronOutputValue)
			);
		}
	}
}

dt
RmlpNet::addNewMeasurementAndGetPrediction(dt const measurement)
{
	// new measurement has come, we may actualize weights

	// calculate w1 and w2 weight diffs

	uint32 idx = 0;

	BOOST_FOREACH(HiddenNron const & hiddenNron, this->hiddenNrons)
	{
		this->finalNron.setW2Dif(idx, this->calculateW2Diff(hiddenNron));
		++ idx;
	}

	this->setW1Difs<INPUT_DELAY>(this->inputDelayNrons);
	this->setW1Difs<OUTPUT_DELAY>(this->outputDelayNrons);

	// adjust weights according to newly computet\d w1 and w2 difs

	dt const weightsChangefactor = this->learningFactor * (measurement - this->finalNron.getNronInternalConst().getOutput());

	this->finalNron.updateWeights(weightsChangefactor);

	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		hiddenNron.updateWeights(weightsChangefactor);
	}

	// update recentW2diffs to be used in next w1 w2 dif compatations

	idx = 0;
	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		boost::optional<dt> const & difOpt = this->finalNron.getEntry(idx).dif;
		BOOST_ASSERT(difOpt.is_initialized());
		hiddenNron.addRecentW2Dif(*difOpt);
		++ idx;
	}

	// reset w1 and w2 difs

	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		hiddenNron.resetDifs();
	}

	this->finalNron.resetDifs();

	// TO BE DONE: update values

	return 0;
}

std::ostream & operator << (std::ostream & osek, RmlpNet const & rmlpNet)
{
	osek << "RmlpNet{ learningFactor:" << rmlpNet.learningFactor
		<< "\n INPUT_DELAY_NRONS:\n" << ContainerPrinter<std::vector<NronInt>,'\n'>(rmlpNet.inputDelayNrons)
		<< "\n OPUTPUT_DELAY_NRONS:\n" << ContainerPrinter<std::vector<NronInt>,'\n'>(rmlpNet.outputDelayNrons)
		<< "\n HIDDEN_NRONS:\n" << ContainerPrinter<std::vector<HiddenNron>,'\n'>(rmlpNet.hiddenNrons)
		<< "\n FINAL_NRON:\n" << rmlpNet.finalNron << "}\n";
	return osek;
}

} // ns ajres
