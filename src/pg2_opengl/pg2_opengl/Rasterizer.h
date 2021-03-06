#pragma once
#ifndef MY_RASTERIZER_H_
#define MY_RASTERIZER_H_

#include "objloader.h"

#include "Triangle3.h"
#include "Camera.h"

void glfw_callback_1(const int error, const char* description);
bool check_gl_1(const GLenum error = glGetError());
void GLAPIENTRY gl_callback_1(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param);
void framebuffer_resize_callback_1(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#pragma pack(push, 1)
struct GLMaterial {
	// DIFFUSE
	Color3f albedo; // 3 * GB
	GLbyte pad0[4]; // + 4 B = 16 B
	GLuint64 tex_diffuse_handle{ 0 }; // 1 * 8 B
	GLbyte pad1[8]; // + 8 B = 16 B

	// ROUGHNESS, METALNESS, ALPHA
	Color3f rma;	// alpha = 1
	GLbyte pad2[4];
	GLuint64 tex_rma_handle{ 0 };
	GLbyte pad3[8];

	// NORMALS
	Color3f normal;	// (0,0,1) if bump map is not present
	GLbyte pad4[4];
	GLuint64 tex_normal_handle{ 0 };
	GLbyte pad5[8];
};
#pragma pack( pop )


class Rasterizer {

	public:
		Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float farPlane, float nearPlane);
		int initOpenGL();
		int loadMesh(const std::string& file_name); 
		int initBuffersAndTextures();
		int initShaders(const std::string& vert_path, const std::string& frag_path);
		int initMaterials(bool for_cube);
		int mainLoop();
		std::vector<Vertex3> getVertices() { return this->vertices_; };

		Camera& getCamera() { return this->camera_; };
		Camera& getLight() { return this->light_; };
		//std::vector<Triangle3> getTriangles() { return this->triangles_; };


	private:
		SceneGraph scene_;
		MaterialLibrary materials_;
		std::vector<GLMaterial> materials_gl;

		//std::vector<Material*> materials;

		Camera camera_;
		Camera light_;
		std::vector<Vertex3> vertices_;
		//std::vector<Triangle3> triangles_;
		GLFWwindow* window_;
		GLuint vao_, vbo_;

		GLuint tex_irradiance_map_;
		void initIrradianceMapTexture(const std::string& file_name);
		int setIrradianceMap();

		GLuint tex_prefilteredEnv_map_{ 0 };
		void initPrefilteredEnvMapTexture();
		int setPrefilteredEnvMap();
		
		GLuint tex_integration_map_{ 0 };
		void initIntegrationMapTexture(const std::string& file_name);
		int setIntegrationMap();
		float far_plane_, near_plane_;

		GLuint vertex_shader_;
		GLuint fragment_shader_;
		GLuint shader_program_;

		void resize(const int width, const int height);

		int LoadShader(const std::string& file_name, std::vector<char>& shader);
		GLint CheckShader(const GLenum shader);

		void InitOneEnvMapTexture();
		
		
};


#endif // !MY_RASTERIZER_H_