#include <gsl/gsl_fit.h>

#include <boost/foreach.hpp>
#include "leastSquares.h"


namespace ajres
{

LeastSquares::LeastSquares(uint32 const xNumArg) :
	xNum(xNumArg)
{
	this->x = new double[this->xNum];
	for (uint32 i = 0; i<this->xNum; ++i)
	{
		dt xVal = this->xNum;
		x[i] = (-xVal+i);
	}
}

LeastSquares::~LeastSquares()
{
	delete[] this->x;
}

dt
LeastSquares::addNewMeasurementAndGetPrediction(dt const val)
{
	if (this->xNum == 1)
	{
		return val;
	}

	this->lastValues.push_back(val);
	dt prediction = 0;

	if (this->lastValues.size() > this->xNum)
	{
		this->lastValues.pop_front();
	}

	if (this->lastValues.size() == this->xNum)
	{
		double * y = new double[this->xNum];
		uint32 idx = 0;
		BOOST_FOREACH(dt yVal, this->lastValues)
		{
			y[idx] = yVal;
			++idx;
		}

		double c0, c1, cov00, cov01, cov11, sumsq;

		gsl_fit_linear(this->x, 1, y, 1, this->xNum, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);

		prediction = c0;

		delete[] y;
	}

	return prediction;
}

std::string
LeastSquares::getName() const
{
	return "LeastSquares";
}


}
