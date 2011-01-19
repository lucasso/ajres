#include "learningFactor.h"
#include <math.h>

namespace ajres
{

LearningFactor::~LearningFactor()
{
}

LearningFactor::ErrorInfoDb::~ErrorInfoDb()
{
}

// ========================================================================

class AdaLearningFactor : public LearningFactor
{
	dt const maxAllowedErrorIncrease;
	dt const decreaseFactor;
	dt const increaseFactor;

	dt lFactor;
	boost::optional<dt> error;

public:

	AdaLearningFactor(
		dt const initialLfactor,
		dt const maxAllowedErrorIncrease = 1.04,
		dt const decreaseFactor = 0.7,
		dt const increaseFactor = 1.05
	);

	virtual dt getLfactor() const;
	virtual dt computeLfactor(ErrorInfoDb &);
};

AdaLearningFactor::AdaLearningFactor(
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
AdaLearningFactor::getLfactor() const
{
	return this->lFactor;
}

dt
AdaLearningFactor::computeLfactor(LearningFactor::ErrorInfoDb & errorInfo)
{
	dt const currentStepError = ::fabs(errorInfo.getError(0));

	if (this->error.is_initialized())
	{
		if ( currentStepError > (*(this->error)) * maxAllowedErrorIncrease)
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

// ========================================================================

class BisectionLearningFactor : public LearningFactor
{

	dt lFactor;

public:

	BisectionLearningFactor();

	virtual dt getLfactor() const;
	virtual dt computeLfactor(ErrorInfoDb &);
};

BisectionLearningFactor::BisectionLearningFactor() :
	lFactor(0)
{
}

dt
BisectionLearningFactor::getLfactor() const
{
	return this->lFactor;
}

dt
BisectionLearningFactor::computeLfactor(ErrorInfoDb & errorInfoDb)
{
	static int32 const precision = 100;
	dt minError = errorInfoDb.getError(0);
	dt bestLearningFactor = 0;
	for (int32 i=-precision; i<precision; ++i)
	{
		dt const learningFactorExamined = static_cast<dt>(i)/static_cast<dt>(precision);
		dt const errorOfExamiedLfactor = errorInfoDb.getError(learningFactorExamined);
		if (errorOfExamiedLfactor < minError)
		{
			minError = errorOfExamiedLfactor;
			bestLearningFactor = learningFactorExamined;
		}
	}

	this->lFactor = bestLearningFactor;
	return this->lFactor;
}

// ========================================================================

std::auto_ptr<LearningFactor>
LearningFactorCreate::createAdaptativeLearningFactor(dt const initialFactor)
{
	return std::auto_ptr<LearningFactor>(new AdaLearningFactor(initialFactor));
}



std::auto_ptr<LearningFactor>
LearningFactorCreate::createSecondOrderPolynomialBasedLearningFactor()
{
	return std::auto_ptr<LearningFactor>();
}

std::auto_ptr<LearningFactor>
LearningFactorCreate::createBisectionBasedLearningFactor()
{
	return std::auto_ptr<LearningFactor>(new BisectionLearningFactor);
}

} // ns ajres
