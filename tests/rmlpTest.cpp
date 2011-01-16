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
		return static_cast<dt>(valueReturned);
	}
};


bool
constantValuePredictionTest()
{
	RmlpNet net(10,10,5, std::auto_ptr<RandomGenerator>(new ConstantGenerator<2u>));

	for (uint32 i = 0; i < 10000; ++i)
	{
		dt const expect = (i % 3) * 1.5;
		dt const prediction = net.addNewMeasurementAndGetPrediction(expect);
		std::cout << "prediction no " << i << " is " << prediction << " expected:" << expect << "\n";
		if (prediction == expect) break;
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
