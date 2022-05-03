#include "pch.h"
#include "my_main.h"

#include "Rasterizer.h"
#include "MapGenerator.h"
#include "pgmath.h"

int test() {
	Camera camera = Camera(640, 480, deg2rad(45.0f), Vector3(-5.5f,-8.0f,4.0f), Vector3(0, 0, 0), 1.0f, 1000.0f);
	Matrix4x4 MVP = camera.getMatrixMVP();
	return 0;
}

int run() {
	int option = 3; // 0, 1, 2 for avenger, cube and geospheres
	// 3 for generating maps
	if (option == 0) {
		Rasterizer rasterizer = Rasterizer(640, 480, deg2rad(45.0f), Vector3(-200, -300, 200), Vector3(0, 0, 40), 1000.0f, 0.1f);
		rasterizer.initOpenGL();
		rasterizer.loadMesh("../../../data/avenger/6887_allied_avenger_gi2.obj");
		rasterizer.initBuffersAndTextures();
		rasterizer.initShaders("basic_shader.vert", "basic_shader.frag");

		rasterizer.mainLoop();
	}
	else if (option == 1) {
		Rasterizer rasterizer = Rasterizer(640, 480, deg2rad(45.0f), Vector3(-30, -30, 30), Vector3(0, 0, 0), 1000.0f, 0.1f);
		rasterizer.initOpenGL();
		rasterizer.loadMesh("../../../data/cube/piece_02.obj");
		rasterizer.initBuffersAndTextures();
		rasterizer.initShaders("basic_shader.vert", "basic_shader.frag");

		rasterizer.mainLoop();
	}
	else if (option == 2) {
		Rasterizer rasterizer = Rasterizer(640, 480, deg2rad(45.0f), Vector3(-30, -30, 30), Vector3(0, 0, 0), 1000.0f, 0.1f);
		rasterizer.initOpenGL();
		rasterizer.loadMesh("../../../data/spheres/geospheres_5x5.obj");
		rasterizer.initBuffersAndTextures();
		rasterizer.initShaders("basic_shader.vert", "basic_shader.frag");

		rasterizer.mainLoop();
	}
	else if (option == 3) {
		auto g = new MapGenerator();;
		//g->getIrradianceMap(512, 256, "../../../data/maps/lebombo_prefiltered_env_map/0.exr").Save("../../../data/maps/generated_irradiance.exr");
		g->getPrefilteredEnvMap(1.0f, 512, 256, "../../../data/maps/lebombo_prefiltered_env_map/0.exr").Save("../../../data/maps/generated_prefenv_0.exr");
	}

	return 0;
}
