#include "pch.h"
#include "Camera.h"


constexpr double PI2 = 2 * M_PI;


static std::pair<float, float> getSphericalCoords(Vector3 vector) {
	vector.Normalize();
	float p = atan2(vector.y, vector.x);
	float phi = (p < 0) ? p + 2 * M_PI : p;
	float theta = acos(std::clamp<float>(vector.z, -1, 1));

	return std::pair<float, float>(phi, theta);
}

Camera::Camera() {
	Camera(640, 480, 0.785f, Vector3(1, 1, 1), Vector3(1, 1, 1), 99.0f, 1.0f);
};

Camera::Camera(const int width, const int height, const float fov_y, const Vector3 view_from, const Vector3 view_at, const float far_plane, const float near_plane) {
	this->width_ = width;
	this->height_ = height;

	this->view_from_ = view_from;
	this->view_at_ = view_at;

	float aspect = (float)width_ / (float)height_;
	this->fov_y_ = fov_y;
	this->fov_x_ = 2 * atan(aspect * tan(fov_y_ / 2.0f));
	this->up_ = Vector3(0, 0, 1);

	Vector3 view_dir = view_at_ - view_from_;
	view_dir.Normalize();
	auto viewDir_sph = getSphericalCoords(view_dir);
	this->yaw_ = viewDir_sph.first;		//atan2f(view_dir.y, view_dir.x) - PI2;
	this->pitch_ = viewDir_sph.second;	//acosf(-view_dir.z) - PI2;

	this->velocity_ = 5.0f;

	//this->view_direction_ = view_at_ - view_from_;
	//this->view_direction_.Normalize();

	this->near_plane_ = near_plane;
	this->far_plane_ = far_plane;

	//this->view_matrix_ = getMatrixV();
	//this->projection_matrix_ = getMatrixP();
	this->last_mouse_pos_ = Vector2(0, 0);
}


void Camera::update(int width, int height) {
	/*** Vytvorte metodu Update, ktera provede výpocet vsech potrebnych matic (V, P) na zaklade aktualnich parametru kamery.
	Jednotlive matice a jejich kombinace (MVP pro transformaci vertexu do Clip Space, MV pro transformaci vertexu do
	Eye space, MN pro transformaci normal do Eye Space apod.) pak muzete podle potreby predavat (pres ukazatel
	na prvni prvek - metoda data) ve vykreslovaci smycce pomoci funkce SetMatrix4x4 (gutils.cpp) aktualnimu shaderu.
	Zaroven ve vykreslovaci smycce zajistete, aby se kamera vhodnym zpusobem pohybovala. ***/
	
	this->width_ = width;
	this->height_ = height;

	float aspect = (float)this->width_ / (float)this->height_;
	this->fov_x_ = 2 * atan(aspect * tan(this->fov_y_ / 2.0f));

}

Matrix4x4 Camera::getMatrixM() {
	return Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);
}

Matrix4x4 Camera::getMatrixP() {
	float np, fp, a, b, w, h;
	Matrix4x4 N, M;

	np = this->near_plane_;
	fp = this->far_plane_;
	a = (np + fp) / (np - fp);
	b = (2 * np * fp) / (np - fp);

	w = 2 * np * tan(this->fov_x_ / 2);
	h = 2 * np * tan(this->fov_y_ / 2);

	M = Matrix4x4(
		np, 0,  0, 0,
		0,  np, 0, 0,
		0,  0,  a, b,
		0,  0, -1, 0
	);

	N = Matrix4x4(
		2/w, 0,   0, 0,
		0,   2/h, 0, 0,
		0,   0,   1, 0,
		0,   0,   0, 1
	);

	return N * M;
}

Matrix4x4 Camera::getMatrixV() {
	Vector3 z_e = this->view_from_ - this->view_at_;
	z_e.Normalize();
	Vector3 x_e =(-this->up_).CrossProduct(z_e);
	x_e.Normalize();
	Vector3 y_e = z_e.CrossProduct(x_e);
	y_e.Normalize();

	Matrix4x4 V = Matrix4x4(
		x_e.x, y_e.x, z_e.x, view_from_.x,
		x_e.y, y_e.y, z_e.y, view_from_.y,
		x_e.z, y_e.z, z_e.z, view_from_.z,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	V.EuclideanInverse();

	return V;
}

/* MODEL VIEW PROJECTION MATRIX */
Matrix4x4 Camera::getMatrixMVP() {
	return this->getMatrixP() * this->getMatrixV() * this->getMatrixM();
}

Matrix4x4 Camera::getMatrixMV() {
	return this->getMatrixV() * this->getMatrixM();
}

Matrix4x4 Camera::getMatrixMN() {
	Matrix4x4 MN = this->getMatrixM();
	MN.EuclideanInverse();
	MN.Transpose();
	return MN;
}

Vector3 Camera::getViewDir() {
	Vector3 result(this->yaw_, this->pitch_);
	result.Normalize();
	return result;
} 
const float cameraSpeed = 0.1f; // adjust accordingly
void Camera::moveForward() {
	
	this->view_from_ -= view_from_ * cameraSpeed;
}

void Camera::moveBackward() {
	this->view_from_ += view_from_ * cameraSpeed;
}

void Camera::moveLeft() {
	auto moveDir = view_from_.CrossProduct(up_);
	moveDir.Normalize();
	this->view_from_ +=  moveDir * velocity_;
}

void Camera::moveRight() {
	auto moveDir = view_from_.CrossProduct(up_);
	moveDir.Normalize();
	this->view_from_ -= moveDir * velocity_;
	
}
void Camera::moveCameraAngle(double pitch, double yaw) {
	/*Vector3 new_view_dir = Rz(yaw) * Rx(pitch) * Vector3(0, 1, 0);
	new_view_dir.Normalize();
	this->view_at_ = view_from_ + new_view_dir;*/

	this->setYaw(yaw);
	this->setPitch(pitch);

	Vector3 new_view_dir = Rz(this->yaw_) * Rx(this->pitch_) * Vector3(0, 1, 0);
	//Vector3 old_view_dir = this->view_from_ - this->view_at_;//- this->getViewDir();
	new_view_dir.Normalize();
	//this->view_at_ = Vector3{ -sin(y) * cos(p), -sin(p), -cos(y)*cos(p) };
	this->view_at_ = this->view_from_ + new_view_dir;
	//this->view_at_.Normalize();
}

void Camera::setYaw(double x) {
	//printf("%p->", this->yaw_);
	this->yaw_ += float(x*M_PI );
	while (this->yaw_ < 0) this->yaw_ += PI2;
	while (this->yaw_ > (PI2)) this->yaw_ -= PI2;
	//printf("%p\n", this->yaw_);
}

void Camera::setPitch(double y) {
	this->pitch_ += float(y*M_PI );
	if (this->pitch_ < 0) this->pitch_ = 0.01;
	if (this->pitch_ > M_PI) this->pitch_ = M_PI - 0.01;
}

Matrix3x3 Camera::Rx(float alpha) {
	return Matrix3x3{
		1,	0,			0,
		0,	cos(alpha),	-sin(alpha),
		0,	sin(alpha),	cos(alpha)
	};

}

Matrix3x3 Camera::Ry(float alpha) {
	return Matrix3x3{
		cos(alpha),	0,	sin(alpha),
		0,			1,	0,
		-sin(alpha),0,	cos(alpha)
	};
}

Matrix3x3 Camera::Rz(float alpha) {
	return Matrix3x3{
		cos(alpha),	-sin(alpha),0,
		sin(alpha), cos(alpha), 0,
		0,			0,			1
	};
}