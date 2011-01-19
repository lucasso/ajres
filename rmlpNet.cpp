#include "rmlpNet.h"

#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_int.hpp>

#include <math.h>

namespace ajres
{

// ---------------------------------------------------------
// ------------------- Random Generator --------------------
// ---------------------------------------------------------

enum RandomGenMode
{
	POSITIVES_AROUND_1,
	AROUND_0
};

template <RandomGenMode genMode>
class RandomGeneratorImpl : public RandomGenerator
{
	enum Constants
	{
		GRANULARITY = 1000,
	};
	boost::mt19937 genSource;
	boost::uniform_int<int> genDistrib;
	boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > gen;

	dt const maxChange;

public:

	RandomGeneratorImpl(dt const maxChange);
	virtual dt getWeight();
};

template <RandomGenMode genMode>
RandomGeneratorImpl<genMode>::RandomGeneratorImpl(dt const maxChangeArg) :
	genSource(),
	genDistrib(-GRANULARITY,GRANULARITY),
	gen(this->genSource, this->genDistrib),
	maxChange(maxChangeArg)
{
}

template <RandomGenMode genMode> dt
RandomGeneratorImpl<genMode>::getWeight()
{
	int32 const randomNum = this->gen();
	BOOST_ASSERT(randomNum >= -GRANULARITY);
	BOOST_ASSERT(randomNum <= GRANULARITY);
	dt const factor = this->maxChange * randomNum / GRANULARITY;

	switch (genMode)
	{
	case POSITIVES_AROUND_1:
	{
		dt const retVal = (factor < 0)
			? static_cast<dt>(1.0) / (-factor+1.0)
			: static_cast<dt>(1.0) * (factor+1.0);
		//std::cout << "rand:" << randomNum << ", fact:" << factor << ", retVal:" << retVal << "\n";
		BOOST_ASSERT(retVal <= this->maxChange);
		BOOST_ASSERT(retVal >= 1/this->maxChange);
		return retVal;
	}
	case AROUND_0:
	{
		return factor;
	}
	}
}

RandomGenerator::~RandomGenerator()
{
}

std::auto_ptr<RandomGenerator>
RandomGenerator::createDefault()
{
	return std::auto_ptr<RandomGenerator>(new RandomGeneratorImpl<AROUND_0>(1.0));
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
		std::cout << "updating entry:" << entry << " by factor * dif:" << *(entry.dif) << "\n";
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

NronInt::NronInt(std::string const & nameArg, bool const biasNronFlagArg, boost::shared_ptr<ActivationFun> activationFunArg) :
	name(nameArg),
	biasNronFlag(biasNronFlagArg),
	activationFun(activationFunArg)
{
	if (this->biasNronFlag)
	{
		this->outputValue = 1.0;
		this->inputValue = 0.0;
		this->outputDiff = 0.0;
	}
	else
	{
		this->setInput(0);
	}
}

void
NronInt::setInput(dt const newValue)
{
	BOOST_ASSERT(!this->biasNronFlag);
	this->inputValue = newValue;
	this->outputValue = this->activationFun->value(this->inputValue);
	this->outputDiff = this->activationFun->diff(this->inputValue);
}

dt
NronInt::getInput() const
{
	AJRES_ASSERT(!this->biasNronFlag, "trying to get input rom bias nron, " << *this);
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
	AJRES_ASSERT(!this->biasNronFlag, "unexpected bias nron found:" << *this);
	return this->outputDiff;
}

bool
NronInt::isBias() const
{
	return this->biasNronFlag;
}

std::string const &
NronInt::getName() const
{
	return this->name;
}

void
NronInt::shiftValuesHelper(std::vector<NronInt> & nronInts, dt const newValue, bool const skipFirst)
{
	bool firstSkipped = !skipFirst;
	dt valueShifted = newValue;
	BOOST_FOREACH(NronInt & nronInt, nronInts)
	{
		if (!firstSkipped)
		{
			firstSkipped = true;
			AJRES_ASSERT(nronInt.isBias(), "only bias nrons should be skipped buttrying to skip:" << nronInt);
			AJRES_ASSERT(nronInt.getOutput() == 1.0, "bias nrons output should be 1, but there is not, " << nronInt);
		}
		else
		{
			dt const prevInput = nronInt.getInput();
			nronInt.setInput(valueShifted);
			valueShifted = prevInput;
		}
	}
}

std::ostream &
NronInt::print(std::ostream & osek) const
{
	if (this->biasNronFlag)
	{
		osek << "NronInt{" << this->name << ",bias,o:" << this->outputValue << "}";
	}
	else
	{
		osek << "NronInt{" << this->name << ",i:" << this->inputValue << ",o:" << this->outputValue << ",d:" << this->outputDiff << "}";
	}
	return osek;
}

std::ostream & operator << (std::ostream & osek, NronInt const & nronInt)
{
	return nronInt.print(osek);
}

// ---------------------------------------------------------
// ------------------- Hidden Nron -------------------------
// ---------------------------------------------------------

HiddenNron::HiddenNron(
	uint32 const numInDelays, uint32 const numOutDelays,
	RandomGenerator & gen, std::string const & name,
	boost::shared_ptr<ActivationFun> activationFun
) :
	inDelays(), // can not use (num, obj) ctor because nrons have to have different weights
	outDelays(),
	nronInt(name, false, activationFun),
	recentW2Difs(numOutDelays, 0)
{
	this->inDelays.reserve(numInDelays);
	this->outDelays.reserve(numOutDelays);
	for (uint32 i=0; i<numInDelays; ++i) this->inDelays.push_back(Entry(gen));
	for (uint32 i=0; i<numOutDelays; ++i) this->outDelays.push_back(Entry(gen));
}

HiddenNron::HiddenNron(std::string const & name) :
	nronInt(name, true, boost::shared_ptr<ActivationFun>()),
	convolution(0.0)
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
		BOOST_ASSERT(this->recentW2Difs.size() == this->outDelays.size());
		BOOST_ASSERT(!this->nronInt.isBias()); // outDelays may be empty even for non bias nrons

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
	BOOST_ASSERT(!this->nronInt.isBias());

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

template <DelayNronType delayNronType> Entry &
HiddenNron::getEntryForHacking(uint32 const idx)
{
	return this->template getEntries<delayNronType>().at(idx);
}

void
HiddenNron::updateWeights(dt const factor)
{
	BOOST_ASSERT(!this->nronInt.isBias());
	std::cout << "updating inDelays for " << this->nronInt << "\n";
	Entry::weightUpdateHelper(this->inDelays, factor);
	std::cout << "updating outDelays for " << this->nronInt << "\n";
	Entry::weightUpdateHelper(this->outDelays, factor);
}

void
HiddenNron::addRecentW2Dif(dt const val)
{
	// should be equal to num of output signal delays in nonbias nrons
	BOOST_ASSERT(this->recentW2Difs.size() == this->outDelays.size());
	BOOST_ASSERT(!this->nronInt.isBias());

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

FinalNron::FinalNron(uint32 const numHidden, RandomGenerator & gen, boost::shared_ptr<ActivationFun> activationFun) :
	input(),
	nronInt("final", false, activationFun)
{
	this->input.reserve(numHidden);
	for (uint32 i=0; i<numHidden; ++i) this->input.push_back(Entry(gen));
}

Entry const &
FinalNron::getEntry(uint32 const idx) const
{
	return this->input.at(idx);
}

Entry &
FinalNron::getEntryForHacking(uint32 const idx)
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
	std::cout << "update weights of final nron\n";
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

ActivationFun::~ActivationFun()
{
}

class LinearActivationFun : public ActivationFun
{
public:
	dt value(dt const x) const
	{
		return x;
	}

	dt
	diff(dt const) const
	{
		return 1;
	}
};

template <uint32 betaFactor>
class SigmoidalActivationFun : public ActivationFun
{
public:
	dt value(dt const x) const
	{
		return ::tanh(x * static_cast<dt>(betaFactor));
	}

	dt
	diff(dt const x) const
	{
		return static_cast<dt>(betaFactor) * ( 1 - ::pow(this->value(x), 2.0));
	}
};

// ---------------------------------------------------------
// ------------------------- RmlpNet -----------------------
// ---------------------------------------------------------

RmlpNet::RmlpNet(
	uint32 const numInputDelayNronsArg,
	uint32 const numOutputDelayNronsArg,
	uint32 const numHiddenNronsArg,
	std::auto_ptr<RandomGenerator> randomGenerator,
	LearningFactor & learningFactorArg
) :
	numInputDelayNrons(numInputDelayNronsArg),
	numOutputDelayNrons(numOutputDelayNronsArg),
	numHiddenNrons(numHiddenNronsArg),
	inputDelayNrons(),
	outputDelayNrons(),
	hiddenNrons(),
	finalNron(this->numHiddenNrons, *randomGenerator, boost::shared_ptr<ActivationFun>(new SigmoidalActivationFun<1u>)),
	learningFactor(learningFactorArg)
{
	boost::shared_ptr<ActivationFun> sigmoidalActivationFun(new SigmoidalActivationFun<1u>);

	this->inputDelayNrons.push_back(NronInt("inDel_0_b", true, boost::shared_ptr<ActivationFun>()));
	for (uint32 i = 1; i < this->numInputDelayNrons; ++i)
	{
		std::string const name = "inDel_" + boost::lexical_cast<std::string>(i);
		this->inputDelayNrons.push_back(NronInt(name, false, sigmoidalActivationFun));
	}
	for (uint32 i = 0; i < this->numOutputDelayNrons; ++i)
	{
		std::string const name = "outDel_" + boost::lexical_cast<std::string>(i);
		this->outputDelayNrons.push_back(NronInt(name, false, sigmoidalActivationFun));
	}
	this->hiddenNrons.push_back(HiddenNron("hidd_0_b"));
	for (uint32 i = 1; i < this->numHiddenNrons; ++i)
	{
		std::string const name = "hidd_" + boost::lexical_cast<std::string>(i);
		this->hiddenNrons.push_back(HiddenNron(
			this->numInputDelayNrons, this->numOutputDelayNrons, *randomGenerator, name, sigmoidalActivationFun));
	}

	BOOST_ASSERT(inputDelayNrons.size() >= 2u); // at least bias and current input value
	BOOST_ASSERT(hiddenNrons.size() >= 2u); // at least bias and one hidden nron
}

RmlpNet::~RmlpNet()
{
}

dt
RmlpNet::calculateImpl(
	dt const hiddenLayerSumAddon,
	uint32 const convolutionAddonIdx,
	dt const convolutionAddonValue
)
{
	dt result = hiddenLayerSumAddon;

	BOOST_ASSERT(!this->hiddenNrons.empty());
	std::vector<HiddenNron>::iterator const itEnd = this->hiddenNrons.end();
	std::vector<HiddenNron>::iterator it = (++ this->hiddenNrons.begin());
	uint32 idx = 1;

	BOOST_ASSERT(it != itEnd);

	do
	{
		result +=
			this->finalNron.getEntry(idx).weight *
			it->getNronInternalConst().getDiff() * // juju: nie mozna brac tej pochodnej z 1go hidden nronu ktory jest biasem
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
		nron.getNronInternalConst().getOutput(),
		0, 0.0
	);
}

dt
RmlpNet::calculateW1Diff(uint32 const hiddenLayerIdx, dt inputLayerNronValue)
{
	BOOST_ASSERT(hiddenLayerIdx >= 1);
	BOOST_ASSERT(hiddenLayerIdx < this->hiddenNrons.size());

	return this->calculateImpl(
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

void
RmlpNet::computeAndSetValueOfHiddenNron(uint32 const idx, HiddenNron & hiddenNron) const
{
	dt newValue = 0.0;
	uint32 delayIdx = 0;
	BOOST_FOREACH(NronInt const & nronInt, this->inputDelayNrons)
	{
		newValue += nronInt.getOutput() * hiddenNron.getEntry<INPUT_DELAY>(delayIdx).weight;
		++ delayIdx;
	}
	delayIdx = 0;
	BOOST_FOREACH(NronInt const & nronInt, this->outputDelayNrons)
	{
		newValue += nronInt.getOutput() * hiddenNron.getEntry<OUTPUT_DELAY>(delayIdx).weight;
		++ delayIdx;
	}

	hiddenNron.getNronInternal().setInput(newValue);
}

void
RmlpNet::computeAndSetValueOfFinalNron()
{
	dt newValue = 0.0;
	uint32 idx = 0;

	BOOST_FOREACH(HiddenNron const & hiddenNron, this->hiddenNrons)
	{
		newValue += hiddenNron.getNronInternalConst().getOutput() * this->finalNron.getEntry(idx).weight;
		++ idx;
	}

	this->finalNron.getNronInternal().setInput(newValue);
}

void
RmlpNet::computeValues()
{
	uint32 idx = 0;
	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		if (idx != 0)
		{
			this->computeAndSetValueOfHiddenNron(idx, hiddenNron);
		}
		++ idx;
	}

	this->computeAndSetValueOfFinalNron();
}

void
RmlpNet::checkDif(Entry & entry, dt const measurement, dt const origOutput, dt const origError)
{
	dt const origWeight = entry.weight;
	entry.weight *= 1.0001;
	dt const weightDiff = entry.weight - origWeight;
	this->computeValues();
	dt const modifOutput = this->finalNron.getNronInternalConst().getOutput();
	dt const modifError = ::pow(measurement - modifOutput, 2.0) / 2.0;
	dt const errorDiff = modifError - origError;
	dt const difApprox = errorDiff / weightDiff;
	std::cout << "measurement:" << measurement << ", origOut:" << origOutput << ", origError:" << origError << ", origWeight:" << origWeight << ", weightDiff:" << weightDiff
		<< ", modifOutput:" << modifOutput << ", modifError:" << modifError << ", errorDiff:" << errorDiff
		<< ", difComputed:" << *(entry.dif) << ", difApprox:" << difApprox
		<< ", changeComputed:" << (*(entry.dif) * (measurement - origOutput))<< "\n";
}

void
RmlpNet::checkDifs(dt const measurement) const
{
	dt const origOutput = this->finalNron.getNronInternalConst().getOutput();
	dt const origError = ::pow(measurement - this->finalNron.getNronInternalConst().getOutput(),2.0) / 2.0;

	for (uint32 hiddenNronIdx = 1; hiddenNronIdx < this->hiddenNrons.size(); ++hiddenNronIdx)
	{
		for (uint32 inputDelayIdx = 0; inputDelayIdx < this->inputDelayNrons.size(); ++inputDelayIdx)
		{
			RmlpNet tmpNet(*this);
			std::cout << "hiddenNron:" << hiddenNronIdx << ", entry from inDelay:" << inputDelayIdx <<"\n";
			Entry & entry = tmpNet.hiddenNrons[hiddenNronIdx].getEntryForHacking<INPUT_DELAY>(inputDelayIdx);
			tmpNet.checkDif(entry, measurement, origOutput, origError);
		}

		for (uint32 outputDelayIdx = 0; outputDelayIdx < this->outputDelayNrons.size(); ++outputDelayIdx)
		{
			RmlpNet tmpNet(*this);
			std::cout << "hiddenNron:" << hiddenNronIdx << ", entry from outDelay:" << outputDelayIdx <<"\n";
			Entry & entry = tmpNet.hiddenNrons[hiddenNronIdx].getEntryForHacking<OUTPUT_DELAY>(outputDelayIdx);
			tmpNet.checkDif(entry, measurement, origOutput, origError);
		}
	}

	for (uint32 hiddenNronIdx = 0; hiddenNronIdx < this->hiddenNrons.size(); ++hiddenNronIdx)
	{
		RmlpNet tmpNet(*this);
		std::cout << "final nron, entry:" << hiddenNronIdx << "\n";
		Entry & entry = tmpNet.finalNron.getEntryForHacking(hiddenNronIdx);
		tmpNet.checkDif(entry, measurement, origOutput, origError);
	}
}

dt
RmlpNet::getError(dt const proposedLFactor) const
{
	dt const measurement = this->inputDelayNrons[1].getInput();

	if (proposedLFactor == 0)
	{
		dt const predictionError = measurement - this->finalNron.getNronInternalConst().getOutput();
		return predictionError;
	}
	else
	{
		RmlpNet tmpNet(*this);
		tmpNet.updateWeights(proposedLFactor);
		tmpNet.changeValues();
		dt const predictionError = measurement - tmpNet.finalNron.getNronInternalConst().getOutput();
		return predictionError;
		//AJRES_ASSERT(false, "not-implemented");
	}
}

void
RmlpNet::computeDirectionOfWeigtsChange()
{
	uint32 idx = 0;
	BOOST_FOREACH(HiddenNron const & hiddenNron, this->hiddenNrons)
	{
		this->finalNron.setW2Dif(idx, this->calculateW2Diff(hiddenNron));
		++ idx;
	}

	this->setW1Difs<INPUT_DELAY>(this->inputDelayNrons);
	this->setW1Difs<OUTPUT_DELAY>(this->outputDelayNrons);

	// update recentW2Difs to be used in next iteration

	idx = 0;
	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		if (idx != 0)
		{
			boost::optional<dt> const & difOpt = this->finalNron.getEntry(idx).dif;
			BOOST_ASSERT(difOpt.is_initialized());
			hiddenNron.addRecentW2Dif(*difOpt);
		}
		++ idx;
	}
}

void
RmlpNet::updateWeights(dt const lFactorValue)
{
	dt const weightsChangefactor = lFactorValue * this->getError(0); // predictionError;

	this->finalNron.updateWeights(weightsChangefactor);

	uint32 idx = 0;
	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		if (idx) hiddenNron.updateWeights(weightsChangefactor);
		++ idx;
	}

	std::cout << "measurement:" << measurement << ", weights update	d using factor:" << weightsChangefactor << "\n" << *this << "\n";

	// difs are no longer needed

	BOOST_FOREACH(HiddenNron & hiddenNron, this->hiddenNrons)
	{
		hiddenNron.resetDifs();
	}

	this->finalNron.resetDifs();
}

void
RmlpNet::changeValues()
{
	NronInt::shiftValuesHelper(this->outputDelayNrons, this->finalNron.getNronInternalConst().getOutput(), false);
	NronInt::shiftValuesHelper(this->inputDelayNrons, this->measurement, true);

	this->computeValues();
}

dt
RmlpNet::addNewMeasurementAndGetPrediction(dt const measurement)
{

	// mam jakies wejscie i obliczam wyjscie i je zwracam
	// (
	//   licze kierunek w jakim wyjscie zalezy od wag
	//   pomiar
	// ) - mozna zamienic miejscami
	// teraz w tym kiedynku sprawdzam jak zmienic wagi by wyjscie zmienilo sie na takie z min bledem
	// pomiar to teraz moje 'mam jakies wejscie', obl wyjscie i zwracam do usera

	// czyli bedzie tak:

	// pomiar
	// licze kierunek
	// spr jak zmienic wagi, bo mam wreszcie blad
	// zmieniam wagi
	// zmieniam watrosci nronow
	// zwracam predykcje do usera

	this->computeDirectionOfWeigtsChange();
	this->measurement = measurement;
	this->learningFactor.computeLfactor(*this);
	this->updateWeights(this->learningFactor.getLfactor());

	std::cout << "weight update factor is: " << (this->learningFactor.getLfactor() * this->getError(0)) << " = lFactor:"
			<< this->learningFactor.getLfactor() << " * predictionError:" << this->getError(0) << "\n";

	this->changeValues();

	// new measurement has come



	// actualize weights

	// calculate w1 and w2 weight diffs

	//juju
	std::cout << "new measurement:" << measurement << ", values updated, difs computed\n" << *this << "\n\n";

	//this->checkDifs(measurement);

	// adjust weights according to newly computed w1 and w2 difs




	// update recentW2diffs to be used in next w1 w2 dif compatations



	// reset w1 and w2 difs



	// update values

	return this->finalNron.getNronInternalConst().getOutput();
}

std::ostream & operator << (std::ostream & osek, RmlpNet const & rmlpNet)
{
	osek << "RmlpNet{ learningFactor:" << rmlpNet.learningFactor.getLfactor()
		<< "\n INPUT_DELAY_NRONS:\n" << ContainerPrinter<std::vector<NronInt>,'\n'>(rmlpNet.inputDelayNrons)
		<< "\n OPUTPUT_DELAY_NRONS:\n" << ContainerPrinter<std::vector<NronInt>,'\n'>(rmlpNet.outputDelayNrons)
		<< "\n HIDDEN_NRONS:\n" << ContainerPrinter<std::vector<HiddenNron>,'\n'>(rmlpNet.hiddenNrons)
		<< "\n FINAL_NRON:\n" << rmlpNet.finalNron << "}\n";
	return osek;
}

} // ns ajres
