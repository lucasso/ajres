#include "../rmlpNet.h"
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


bool
constantValuePredictionTest()
{
	//RmlpNet net(10,10,5, std::auto_ptr<RandomGenerator>(new ConstantGenerator<25u>));
	//std::auto_ptr<LearningFactor> lFactor = LearningFactorCreate::createAdaptativeLearningFactor();
	std::auto_ptr<LearningFactor> lFactor = LearningFactorCreate::createBisectionBasedLearningFactor(true);
	RmlpNet net(10,10,5, RandomGenerator::createDefault(), *lFactor);

	dt previousPrediction = 0;

	for (uint32 i = 0; i < 100; ++i)
	{
		dt const expect = (i % 4) * 0.25 + 0.12;
		//dt const expect = 3.0;
		std::cout << "prediction no " << i << " was " << previousPrediction << " expected:" << expect << "\n";
		previousPrediction = net.addNewMeasurementAndGetPrediction(expect);

		//if (prediction == expect) break;
	}

	return true;
}


}
}


int main()
{
	ajres::tests::constantValuePredictionTest();
	return 0;
}
