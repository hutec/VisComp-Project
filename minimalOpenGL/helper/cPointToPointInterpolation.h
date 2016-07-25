#pragma once 
#include <glm.hpp>


class cPointToPointInterpolation {
public:
	cPointToPointInterpolation();
	~cPointToPointInterpolation();

	glm::vec3 update(float fTime);
	bool interpolationActive();

	void startLinearInterpolation(glm::vec3 X0, glm::vec3 XT, float requiredTime);

private:
	glm::vec3 vX0;				// starting point ( t=0 )
	glm::vec3 vXT;				// end point ( t=end )
	glm::vec3 vDirection;		// normalized vector from vX0 to vXT
	glm::vec3 vCurrentPosition;

	float fRequiredTime;
	float fSpeed;
	float fPassedTime;				// time since the last interpolation was started
	float fLength;					// distance between vX0 and vXT
	bool active;
};