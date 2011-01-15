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
	RmlpNet net(2,1,2, std::auto_ptr<RandomGenerator>(new ConstantGenerator<2u>));

	for (uint32 i = 0; i < 10; ++i)
	{
		std::cout << "prediction no " << i << " is " << net.addNewMeasurementAndGetPrediction(3.0) << "\n";
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
