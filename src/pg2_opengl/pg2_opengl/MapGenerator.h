#pragma once
#include "texture.h"
#include "Vector3.h"
#include "Matrix3x3.h"

Vector3 rotateVector(Vector3 v, Vector3 n);
Vector3 getCosWeightedSample(Vector3 omega_r);
Vector3 getGGXOmega_h(float alpha, Vector3 n);
Vector3 getGGXOmega_i(float alpha, Vector3 n, Vector3 omega_o);
Vector3 getReflectedVector(Vector3 d, Vector3 n);

class MapGenerator {
public:
	Texture3f getIrradianceMap(int width, int height, std::string background_filepath);
	Texture3f getPrefilteredEnvMap(float alpha, int width, int height, std::string background_filepath);
};

