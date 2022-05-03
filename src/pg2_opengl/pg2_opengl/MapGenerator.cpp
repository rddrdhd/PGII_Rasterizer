#include "pch.h"
#include "MapGenerator.h"
#include "Camera.h"

std::default_random_engine generator;
std::uniform_real_distribution<float> distribution(0, 0.99999f);
auto rng = std::bind(distribution, generator);

static std::pair<float, float> getSphericalCoords(Vector3 vector) {
	vector.Normalize();
	float p = atan2(vector.y, vector.x);
	float phi = (p < 0) ? p + 2 * M_PI : p;
	float theta = acos(std::clamp<float>(vector.z, -1, 1));

	return std::pair<float, float>(phi, theta);
}


Texture3f MapGenerator::getIrradianceMap(int width, int height, std::string background_filepath)
{
	Texture3f result(width, height);
	Texture3f background = Texture3f(background_filepath);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * float(M_PI) / float(result.width());

		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());

			Vector3 reflectedVector(phi, theta);

			int N = 200;
			Color3f sampleSum({ 0,0,0 });
			for (int i = 0; i < N; i++) {
				auto sph_omega_i = getSphericalCoords(getCosWeightedSample(reflectedVector));

				float u = sph_omega_i.first / (2 * M_PI);
				float v = sph_omega_i.second / M_PI;

				sampleSum += background.texel(u, v);
			}
			sampleSum *= 1 / float(N);
			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = Color3f::toSRGB(sampleSum);
		}
	}
	return result;
}



Vector3 getReflectedVector(Vector3 d, Vector3 n) {
	d.Normalize();
	n.Normalize();
	Vector3 vec = d - 2 * (d.DotProduct(n)) * n;
	return vec;
}

Vector3 getGGXOmega_i(float alpha, Vector3 n, Vector3 omega_o) {	//omega_o -> eye direction
	auto omega_h = getGGXOmega_h(alpha, n);							//omega_h ->
	return getReflectedVector(omega_o * -1, omega_h);
}

Vector3 getGGXOmega_h(float alpha, Vector3 n) {
	float xi_1 = rng();
	float xi_2 = rng();

	float phi_n = 2 * float(M_PI) * xi_1;
	float theta_n = atan(alpha * sqrt(xi_2 / (1 - xi_2)));

	Vector3 omega_h(phi_n, theta_n);

	return rotateVector(omega_h, n);
}

Texture3f MapGenerator::getPrefilteredEnvMap(float alpha, int width, int height, std::string background_filepath) {
	Texture3f result(width, height);
	Texture3f background = Texture3f(background_filepath);

	for (int x = 0; x < result.width(); x++) {
		float phi = float(x) * 2.0f * float(M_PI) / float(result.width());

		for (int y = 0; y < result.height(); y++) {
			float theta = float(y) * M_PI / float(result.height());

			Vector3 normal(phi, theta);

			int N = 100;
			Color3f sampleSum({ 0,0,0 });
			for (int i = 0; i < N; i++) {
				auto sph_omega_i = getSphericalCoords(getGGXOmega_i(alpha, normal, normal));
				float u = sph_omega_i.first / (2 * M_PI);
				float v = sph_omega_i.second / M_PI;

				sampleSum += background.texel(u, v);
			}
			sampleSum *= 1 / float(N);
			auto tmp = Color3f::toSRGB(sampleSum);
			result.data()[size_t(x) + size_t(y) * size_t(result.width())] = tmp;

		}
	}


	return result;
}

Vector3 getCosWeightedSample(Vector3 omega_r) {
	float xi_1 = rng();
	float xi_2 = rng();

	Vector3 dir = {
		float(cos(2 * M_PI * xi_1) * sqrt(1 - xi_2)),
		float(sin(2 * M_PI * xi_1) * sqrt(1 - xi_2)),
		float(sqrt(xi_2))
	};
	dir.Normalize();

	auto omega_i = rotateVector(dir, omega_r);
	omega_i.Normalize();
	return omega_i;
}

Vector3 rotateVector(Vector3 v, Vector3 n) {
	Vector3 o1 = n.CrossProduct(Vector3{ 1, 0, 0 });
	if (o1.DotProduct(o1) < 0.001) {
		o1 = n.CrossProduct(Vector3{ 0, 1, 0 });
	}
	o1.Normalize();
	Vector3 o2 = o1.CrossProduct(n);
	o2.Normalize();

	return Matrix3x3{ o1, o2, n } *v;
}