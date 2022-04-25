#include "pch.h"
#include "my_main.h"

#include "Rasterizer.h"
#include "pgmath.h"

int test() {
	Camera camera = Camera(640, 480, deg2rad(45.0f), Vector3(-5.5f,-8.0f,4.0f), Vector3(0, 0, 0), 1.0f, 1000.0f);
	Matrix4x4 MVP = camera.getMatrixMVP();
	return 0;
}

int run() {
	//test();return 0;
	//rasterizer.loadMesh("../../../data/adjacent_triangles.obj");
	// 
	/*
	Rasterizer rasterizer = Rasterizer(640, 480, deg2rad(45.0f), Vector3(-200, -300, 200), Vector3(0, 0, 40), 1000.0f, 0.1f);
	rasterizer.initOpenGL();
	rasterizer.loadMesh("../../../data/avenger/6887_allied_avenger_gi2.obj");
	*/
	
	Rasterizer rasterizer = Rasterizer(640, 480, deg2rad(45.0f), Vector3(-30, -30, 30), Vector3(0, 0, 0), 1000.0f, 0.1f);
	rasterizer.initOpenGL();
	rasterizer.loadMesh("../../../data/cube/piece_02.obj");
	//rasterizer.loadMesh("../../../data/spheres/geospheres_5x5.obj");
	
	

	rasterizer.initBuffersAndTextures();
	rasterizer.initShaders("basic_shader.vert", "basic_shader.frag");
	
	rasterizer.mainLoop();
	return 0;
}
