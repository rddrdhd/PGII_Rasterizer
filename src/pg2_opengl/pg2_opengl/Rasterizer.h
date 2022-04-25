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



class Rasterizer {

	public:
		Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float farPlane, float nearPlane);
		int initOpenGL();
		int loadMesh(const std::string& file_name); 
		int initBuffers();
		int initShaders(const std::string& vert_path, const std::string& frag_path);
		int mainLoop();
		std::vector<Vertex3> getVertices() { return this->vertices_; };

		Camera& getCamera() { return this->camera_; };
		Camera& getLight() { return this->light_; };
		//std::vector<Triangle3> getTriangles() { return this->triangles_; };


	private:
		SceneGraph scene_;
		MaterialLibrary materials_;

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

		GLuint tex_rma_map_;
		void initRMATexture(const std::string& file_name);
		int setRMATexture();

		GLuint tex_normal_map_;
		void initNormalTexture(const std::string& file_name);
		int setNormalTexture();

		GLuint vertex_shader_;
		GLuint fragment_shader_;
		GLuint shader_program_;

		void resize(const int width, const int height);

		int LoadShader(const std::string& file_name, std::vector<char>& shader);
		GLint CheckShader(const GLenum shader);

		void InitOneEnvMapTexture();
		
		
};


#endif // !MY_RASTERIZER_H_