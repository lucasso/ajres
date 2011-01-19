#ifndef _AJRES_RMLPNET_H_
#define _AJRES_RMLPNET_H_

#include "common.h"
#include "learningFactor.h"
#include <deque>
#include <vector>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

namespace ajres
{

class RandomGenerator
{
public:
	virtual ~RandomGenerator();
	virtual dt getWeight() = 0;

	static std::auto_ptr<RandomGenerator> createDefault();
};

class ActivationFun
{
public:
	virtual ~ActivationFun();
	virtual dt value(dt const) const = 0;
	virtual dt diff(dt const) const = 0;
};

struct Entry
{
	dt weight;
	boost::optional<dt> dif;

	Entry(RandomGenerator &); // generator to initialize weight

	static void weightUpdateHelper(std::vector<Entry> &, dt const factor);
	static void difsResetHelper(std::vector<Entry> &);
};

class NronInt
{
	dt inputValue;
	dt outputValue;
	dt outputDiff;
	std::string name; // for debug purpose only
	bool biasNronFlag;
	boost::shared_ptr<ActivationFun> activationFun;

public:

	NronInt(std::string const & name, bool const biasNronFlag, boost::shared_ptr<ActivationFun> activationFun);

	void setInput(dt const);
	dt getInput() const;
	dt getOutput() const;
	dt getDiff() const;
	std::string const & getName() const;

	bool isBias() const;
	std::ostream & print(std::ostream &) const;

	static void shiftValuesHelper(std::vector<NronInt> &, dt const, bool const);
};

enum DelayNronType
{
	INPUT_DELAY,
	OUTPUT_DELAY
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

	// helper methods
	template <DelayNronType delayNronType> std::vector<Entry> & getEntries();
	template <DelayNronType delayNronType> std::vector<Entry> const & getEntries() const;

	friend std::ostream & operator << (std::ostream &, HiddenNron const &);

public:

	HiddenNron(
		uint32 const numInDelays, uint32 const numOutDelays,
		RandomGenerator &, std::string const & name,
		boost::shared_ptr<ActivationFun> activationFun
	);

	HiddenNron(std::string const & name); // biasNron

	dt getConvolutionOfOutputDelayNrosWeightsWithRecentDifs();

	template <DelayNronType delayNronType>
	void setW1Dif(uint32 const idx, dt const dif);
	void resetDifs();

	template <DelayNronType delayNronType>
	Entry const & getEntry(uint32 const) const;

	template <DelayNronType delayNronType>
	Entry & getEntryForHacking(uint32 const);

	void updateWeights(dt const factor);
	void addRecentW2Dif(dt const);

	NronInt & getNronInternal();
	NronInt const & getNronInternalConst() const;
};

class FinalNron
{
	std::vector<Entry> input;
	NronInt nronInt;

	friend std::ostream & operator << (std::ostream &, FinalNron const &);

public:

	FinalNron(uint32 const numHidden, RandomGenerator & gen, boost::shared_ptr<ActivationFun>);

	Entry const & getEntry(uint32 const) const;
	Entry & getEntryForHacking(uint32 const);
	void setW2Dif(uint32 const idx, dt const dif);
	void updateWeights(dt const factor);
	void resetDifs();

	NronInt & getNronInternal();
	NronInt const & getNronInternalConst() const;
};

class RmlpNet : LearningFactor::ErrorInfoDb
{
	uint16 const numInputDelayNrons; // without bias
	uint16 const numOutputDelayNrons;
	uint16 const numHiddenNrons; // without bias

	std::vector<NronInt> inputDelayNrons;
	std::vector<NronInt> outputDelayNrons;
	std::vector<HiddenNron> hiddenNrons;
	FinalNron finalNron;

	LearningFactor & learningFactor;
	dt measurement;

	dt calculateImpl(
		dt const hiddenLayerSumAddon,
		uint32 const convolutionAddonIdx,
		dt const convolutionAddonValue
	);

	dt calculateW2Diff(HiddenNron const &); // non const cuz it calculates and stores convolution
	dt calculateW1Diff(uint32 const hiddenLayerIdx, dt inputLayerNronValue);

	template <DelayNronType delayNronType>
	void setW1Difs(std::vector<NronInt> const & delayNrons);

	void computeAndSetValueOfHiddenNron(uint32 const, HiddenNron &) const;
	void computeAndSetValueOfFinalNron();

	void computeValues();
	void checkDif(Entry & entry, dt const measurement, dt const origOutput, dt const origError);
	void checkDifs(dt const measurement) const;

	virtual dt getError(dt const proposedLFactor) const; // LearningFactor::ErrorInfoDb interface

	// main steps od addNew...

	void computeDirectionOfWeigtsChange();
	void updateWeights(dt const learningFactor);
	void changeValues();

	friend std::ostream & operator << (std::ostream &, RmlpNet const &);

public:

	RmlpNet(
		uint32 const numInputDelayNrons, uint32 const numOutputDelayNrons, uint32 const numHiddenNrons,
		std::auto_ptr<RandomGenerator>, LearningFactor &);

	~RmlpNet();

	dt addNewMeasurementAndGetPrediction(dt const);
};

std::ostream & operator << (std::ostream &, Entry const &);
std::ostream & operator << (std::ostream &, NronInt const &);
std::ostream & operator << (std::ostream &, HiddenNron const &);
std::ostream & operator << (std::ostream &, FinalNron const &);
std::ostream & operator << (std::ostream &, RmlpNet const &);


} // ns ajres


#endif /* _AJRES_RMLPNET_H_ */
