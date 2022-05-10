#pragma once
#ifndef CAMERA_H_
#define CAMERA_H_

#include "Vector3.h"
#include "Vector2.h"
#include "matrix4x4.h"
static std::pair<float, float> getSphericalCoords(Vector3 vector);
class Camera
{
private:
	int width_; // image width (px)
	int height_; // image height (px)
	float fov_y_; // vertical field of view (radians)
	float fov_x_; // horizontal field of view (radians)
	Vector3 view_from_; // eye
	Vector3 view_at_; // what ya lookin at
	Vector3 up_{ Vector3(0.0f,0.0f,1.0f) }; // up vector

	float near_plane_; // near plane distance
	float far_plane_; // far plane distance

	float velocity_;
	double yaw_, pitch_;

	Vector2 last_mouse_pos_;
	Vector2 last_mouse_scroll_;

public:

	Camera();
	Camera(const int width, const int height, const float fov_y,
		const Vector3 view_from, const Vector3 view_at, const float far_plane, const float near_plane);

	void update(int width, int height);

	Vector3 getViewFrom() { return this->view_from_; }
	int getWidth() { return this->width_; }
	int getHeight() { return this->height_; }
	float getVelocity() { return this->velocity_; }

	Matrix4x4 getMatrixM(); // jednotkova matice pro model
	Matrix4x4 getMatrixV(); // view matice
	Matrix4x4 getMatrixP(); // projekcni matice
	Matrix4x4 getMatrixMVP();
	Matrix4x4 getMatrixMV();
	Matrix4x4 getMatrixMn(); //  pro normaly

	void moveCameraX(bool toTheRight);
	void moveCameraY(bool forward);
	void moveCameraZ(bool up);

	void moveUp() { moveCameraZ(true); };
	void moveDown() { moveCameraZ(false); };
	void moveLeft() { moveCameraX(true); };
	void moveRight() { moveCameraX(false); };

	void moveCameraAngle(double pitch, double yaw);

	Matrix3x3 Rx(float alpha); // pitch - sklon
	Matrix3x3 Rz(float alpha); // yaw - kurs
	Matrix3x3 Ry(float alpha); // roll - natoceni, nepouzivame

	Vector2 getLastScrollPos() { return this->last_mouse_scroll_; };
	void setLastScrollPos(Vector2 position) { this->last_mouse_scroll_ = position; };

	void setPosition(Vector3 position) { this->view_from_ = position; };
	Vector3 getPosition() { return this->view_from_; };
	
};


#endif