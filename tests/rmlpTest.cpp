#include "../rmlpNet.h"
#include <iostream>

namespace ajres
{
namespace tests
{

bool
constantValuePredictionTest()
{
	RmlpNet net(2,0,2);

	for (uint32 i = 0; i < 100; ++i)
	{
		std::cout << "prediction no " << i << " is " << net.addNewMeasurementAndGetPrediction(3.1415) << "\n";
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
