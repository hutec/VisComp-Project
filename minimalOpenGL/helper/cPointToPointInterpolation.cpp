#include "cPointToPointInterpolation.h"

/**
	Initializes the interpolateable point.
	Sets the initial position to vXT
	@param vXT - the initial value
*/
cPointToPointInterpolation::cPointToPointInterpolation()
	: vX0( glm::vec3(0.0f) )
	, vXT( glm::vec3(0.0f, 0.0f, 1.0f) )
    , vDirection ( glm::vec3(0.0f) )
    , vCurrentPosition( vX0 )
	, fRequiredTime( 1.0f )
    , fSpeed ( 0.0f )
    , fPassedTime( 0.0f )
    , fLength ( 0.0f )
	, active(false)

{
	vDirection = glm::normalize(vXT - vX0);
	fLength = glm::length(vXT - vX0);
	fSpeed = fLength / fRequiredTime;
}

cPointToPointInterpolation::~cPointToPointInterpolation(void)
{
}

bool cPointToPointInterpolation::interpolationActive()
{
	return active;
}

/**
	Updates the currently active interpolation.
	Calculates the new interpolated vector.
	The interpolation stops at the target vector if the overall
	required time is reached.
	@param fTime - the time passed since the last frame
	@returm the interpolated value
*/
glm::vec3 cPointToPointInterpolation::update(float fTime)
{
	fPassedTime += fTime;

	if (fPassedTime <= fRequiredTime)
	{
		vCurrentPosition = vX0 + fPassedTime * fSpeed * vDirection;		// calculate the current position
		return vCurrentPosition;
	}
	else
	{
		active = false;
		return vXT;
	}
		
}


/**
	Starts a new linear interpolation between the two given points.
	The interpolation takes {@param requiredTime} seconds.
	@param X0 - the start point
	@param XT - the end point
	@param requiredTime - the time the interpolation should take in seconds
*/
void cPointToPointInterpolation::startLinearInterpolation(glm::vec3 X0, glm::vec3 XT, float requiredTime)
{
	// Check if we already reached the target position
	if (glm::length(X0 - XT) < 0.0001f || requiredTime < 0.00001f)
	{
		active = false;
		return;
	}

	active = true;
	vX0 = X0;
	vXT = XT;
	fRequiredTime = requiredTime;

	// calc required values for the interpolation
	glm::vec3 vTemp = (XT - vX0);
	fLength = glm::length(vTemp);
	vDirection = glm::normalize(vTemp);
	fSpeed = fLength / fRequiredTime;

	// reset the timer
	fPassedTime = 0.0f;
}