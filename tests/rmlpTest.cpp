#include "../rmlpNet.h"
#include "../netsFarm.h"
#include <iostream>

namespace ajres
{
namespace tests
{

template <uint32 valueReturned>
class ConstantGenerator : public RandomGenerator
{
public:
	virtual dt getWeight()
	{
		return static_cast<dt>(valueReturned)/100;
	}
};

template <class PredictionMachineT>
bool testImpl(PredictionMachineT & predictionMachine)
{
	//RmlpNet net(10,10,5, std::auto_ptr<RandomGenerator>(new ConstantGenerator<25u>));
	//std::auto_ptr<LearningFactor> lFactor = LearningFactorCreate::createAdaptativeLearningFactor();


	dt previousPrediction = 0;

	for (uint32 i = 0; i < 100; ++i)
	{
		dt const expect = (i % 4) * 0.25 + 0.12;
		//dt const expect = 3.0;
		std::cout << "prediction no " << i << " was " << previousPrediction << " expected:" << expect << "\n";
		previousPrediction = predictionMachine.addNewMeasurementAndGetPrediction(expect);

		//if (prediction == expect) break;
	}

	return true;
}

bool
constantValuePredictionTest()
{
	std::auto_ptr<LearningFactor> lFactor = LearningFactorCreate::createBisectionBasedLearningFactor(true);
	RmlpNet net(10,10,5, RandomGenerator::createDefault(), *lFactor);

	return testImpl(net);
}

bool
netsFarmTest()
{
	std::auto_ptr<LearningFactor> lFactor = LearningFactorCreate::createBisectionBasedLearningFactor(false);
	NetsFarm farm(15, *lFactor);
	return testImpl(farm);
}

}
}


int main()
{
	//ajres::tests::constantValuePredictionTest();
	ajres::tests::netsFarmTest();
	return 0;
}
