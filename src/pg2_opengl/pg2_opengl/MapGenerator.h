#pragma once
#include "texture.h"
#include "Vector3.h"
#include "Matrix3x3.h"

Vector3 rotateVector(Vector3 v, Vector3 n);
Vector3 getCosWeightedSample(Vector3 omega_r);

class MapGenerator {
public:
	Texture3f getIrradianceMap(int width, int height, std::string background_filepath);
};

