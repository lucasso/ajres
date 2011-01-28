#include <deque>
#include "common.h"

namespace ajres
{

class LeastSquares
{
	uint32 const xNum;
	std::deque<dt> lastValues;
	double * x;

public:

	LeastSquares(uint32 const xNum);
	~LeastSquares();

	dt addNewMeasurementAndGetPrediction(dt const);
	std::string getName() const;
};

}
