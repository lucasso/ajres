#ifndef LEARNINGFACTOR_H_
#define LEARNINGFACTOR_H_

#include "common.h"

namespace ajres
{

class LearningFactor
{
public:

	virtual ~LearningFactor();

	class ErrorInfoDb
	{
	public:
		virtual ~ErrorInfoDb();
		virtual dt getError(dt const proposedLFactor) const = 0;
	};

	virtual dt getLfactor() const = 0;
	virtual dt computeLfactor(ErrorInfoDb &) = 0;
};

struct LearningFactorCreate
{
	static std::auto_ptr<LearningFactor> createAdaptativeLearningFactor(dt const initialFactor = 1.0);
	static std::auto_ptr<LearningFactor> createSecondOrderPolynomialBasedLearningFactor();
	static std::auto_ptr<LearningFactor> createBisectionBasedLearningFactor();
};

}


#endif /* LEARNINGFACTOR_H_ */
