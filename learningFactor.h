#ifndef LEARNINGFACTOR_H_
#define LEARNINGFACTOR_H_

#include "common.h"

namespace ajres
{


class LearningFactor
{
	dt const maxAllowedErrorIncrease;
	dt const decreaseFactor;
	dt const increaseFactor;

	dt lFactor;
	boost::optional<dt> error;

public:

	LearningFactor(
		dt const initialLfactor,
		dt const maxAllowedErrorIncrease = 1.04,
		dt const decreaseFactor = 0.7,
		dt const increaseFactor = 1.05
	);

	dt getLfactor() const;
	dt computeLfactorUsingCurrentError(dt const currentStepError);
};

}


#endif /* LEARNINGFACTOR_H_ */
