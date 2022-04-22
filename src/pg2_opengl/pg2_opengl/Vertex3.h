#pragma once
#ifndef MY_VERTEX3_H_
#define MY_VERTEX3_H_

#include "Vector2.h"

struct Vertex3
{
	Vector3 position; /* vertex position */
	Vector3 normal; /* vertex normal */
	Vector3 color; /* vertex color */
	Vector2 texture_coord; /* vertex texture coordinates */
	Vector3 tangent; /* vertex tangent */
	int material_index; /* material index */
};

#endif // !MY_VERTEX3_H_
