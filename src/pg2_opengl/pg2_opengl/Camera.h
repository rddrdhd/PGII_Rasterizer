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
	Vector3 up_; // up vector

	float near_plane_; // near plane distance
	float far_plane_; // far plane distance

	float velocity_;
	double yaw_, pitch_;

	Vector2 last_mouse_pos_;

public:

	Camera();
	Camera(const int width, const int height, const float fov_y,
		const Vector3 view_from, const Vector3 view_at, const float far_plane, const float near_plane);

	void update(int width, int height);

	Vector3 getViewFrom() { return this->view_from_; }
	Vector3 getViewAt() { return (this->getViewFrom() + this->getViewDir()); }// return this->view_at_; }
	Vector3 getViewDir();// { return view_at_ - view_from_; }
	int getWidth() { return this->width_; }
	int getHeight() { return this->height_; }
	float getVelocity() { return this->velocity_; }

	Matrix4x4 getMatrixM(); // jednotkova matice pro model
	Matrix4x4 getMatrixV(); // view matice
	Matrix4x4 getMatrixP(); // projekcni matice
	Matrix4x4 getMatrixMVP();
	Matrix4x4 getMatrixMV();
	Matrix4x4 getMatrixMN(); //  pro normaly

	void setVelocity(float velocity) { this->velocity_ = velocity; }

	void moveForward();
	void moveBackward();
	void moveLeft();
	void moveRight();

	void moveCameraAngle(double pitch, double yaw);
	void setYaw(double x);
	void setPitch(double y);

	Matrix3x3 Rx(float alpha); // pitch - sklon
	Matrix3x3 Rz(float alpha); // roll - natoceni
	Matrix3x3 Ry(float alpha); // yaw - kurs

	Vector2 getLastMousePos() { return this->last_mouse_pos_; };
	void setLastMousePos(Vector2 position) { this->last_mouse_pos_ = position; };

	void setPosition(Vector3 position) { this->view_from_ = position; };
	Vector3 getPosition() { return this->view_from_; };
	
};


#endif