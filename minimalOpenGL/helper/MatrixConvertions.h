#pragma once
#ifndef CONVERSIONS
#define CONVERSIONS

#include "../matrix.h"
#include <glm.hpp>

inline glm::mat4 Matrix4x4ToGLM(Matrix4x4 m) {
	glm::mat4 glmMat;

	const float *f = m.data;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			glmMat[j][i] = f[i*4 + j];
		}
	}

	return glmMat;
}

#endif 