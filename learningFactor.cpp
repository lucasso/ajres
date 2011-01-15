#include "learningFactor.h"
#include <math.h>

namespace ajres
{

LearningFactor::LearningFactor(
	dt const initialLfactor,
	dt const maxAllowedErrorIncreaseArg,
	dt const decreaseFactorArg,
	dt const increaseFactorArg
) :
	maxAllowedErrorIncrease(maxAllowedErrorIncreaseArg),
	decreaseFactor(decreaseFactorArg),
	increaseFactor(increaseFactorArg),
	lFactor(initialLfactor)
{
	AJRES_ASSERT(this->maxAllowedErrorIncrease > 1,
		"maxAllowedErrorIncrease:" << this->maxAllowedErrorIncrease << " should be greater than 1");
	AJRES_ASSERT(this->decreaseFactor < 1,
		"decreaseFactor:" << this->decreaseFactor << " should be less than 1");
	AJRES_ASSERT(this->increaseFactor > 1,
		"increaseFactor:" << this->increaseFactor << " should be greater than 1");
	AJRES_ASSERT(this->lFactor > 0,
		"increaseFactor:" << this->increaseFactor << " should be greater than 0");
}


dt
LearningFactor::getLfactor() const
{
	return this->lFactor;
}

dt
LearningFactor::computeLfactorUsingCurrentError(dt const currentStepError)
{
	if (this->error.is_initialized())
	{
		if (::fabs(currentStepError) > ::fabs(*(this->error)) * maxAllowedErrorIncrease)
		{
			//std::cout << "decrease lFactor\n";
			this->lFactor *= this->decreaseFactor;
		}
		else
		{
			//std::cout << "increase lFactor\n";
			this->lFactor *= this->increaseFactor;
		}
	}

	this->error = boost::optional<dt>(currentStepError);
	return this->lFactor;
}


} // ns ajres
