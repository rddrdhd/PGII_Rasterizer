#include "pch.h"
#include "Rasterizer.h"
#include "utils.h"
#include "glutils.h"

/* constructor */
Rasterizer::Rasterizer(int width, int height, float fovY, Vector3 viewFrom, Vector3 viewAt, float farPlane, float nearPlane) {
	this->camera_ = Camera(width, height, fovY, viewFrom, viewAt, farPlane, nearPlane);
	//this->light_ = Camera(width, height, fovY, viewFrom + Vector3{0,200,0}, viewAt, farPlane, nearPlane);
	this->vao_ = 0;
	this->vbo_ = 0;
}

/* initing OpenGL window */
int Rasterizer::initOpenGL() {
	glfwSetErrorCallback(glfw_callback_1);

	if (!glfwInit())
	{
		return(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8); // 8 samplu na pixel
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

	this->window_ = glfwCreateWindow(this->camera_.getWidth(), this->camera_.getHeight(), "PG2 OpenGL", nullptr, nullptr);
	if (!this->window_){
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwSetWindowUserPointer(this->window_, reinterpret_cast<void*>(this));
	glfwSetKeyCallback(this->window_, key_callback);
	glfwSetCursorPosCallback(this->window_, cursor_position_callback);

	glfwSetInputMode(this->window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// zavola se resize callback
	glfwSetFramebufferSizeCallback(this->window_, framebuffer_resize_callback_1);

	//fukce opengl muzeme volat jen z vlakna, ze ktereho jsme hoh vytvorili. Drahe, ale funkcni.
	glfwMakeContextCurrent(this->window_);

	// umozni folani funkci z opengl
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		if (!gladLoadGL()) {
			return EXIT_FAILURE;
		}
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_callback_1, nullptr); // debugging je nahovno, tak tady aspon nejake zpravy o chybach

	printf("OpenGL %s, ", glGetString(GL_VERSION));
	printf("%s", glGetString(GL_RENDERER));
	printf(" (%s)\n", glGetString(GL_VENDOR));
	printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	check_gl_1();

	glEnable(GL_MULTISAMPLE);

	// map from the range of NDC coordinates <-1.0, 1.0>^2 to <0, width> x <0, height>
	glViewport(0, 0, this->camera_.getWidth(), this->camera_.getHeight());
	// GL_LOWER_LEFT (OpenGL) or GL_UPPER_LEFT (DirectX, Windows) and GL_NEGATIVE_ONE_TO_ONE or GL_ZERO_TO_ONE
	glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE); // od -1 do 1

	return 0;
}

/* laods vertexes from .obj  */
int Rasterizer::loadMesh(const std::string& file_name)
{
	LoadOBJ(file_name, this->scene_, this->materials_, false);

	// prace s mapami a shared pointry -___-
	if (file_name == "../../../data/cube/piece_02.obj") {
		auto normal_tex = std::make_shared<Texture3u>("D:/School/PGII/project/pg2/data/cube/scuffed-plastic-normal.png");
		this->materials_["white_plastic"]->set_texture(Map::kNormal, normal_tex);
		auto rma_tex = std::make_shared<Texture3u>("D:/School/PGII/project/pg2/data/cube/plastic_02_rma.png");
		this->materials_["white_plastic"]->set_texture(Map::kRMA, rma_tex);
	}
	
	// for each surface
	for (SceneGraph::iterator iter = this->scene_.begin(); iter != this->scene_.end(); ++iter)
	{
		const std::string& node_name = iter->first;
		const auto& node = iter->second;

		const auto& mesh = std::static_pointer_cast<Mesh>(node);

		if (mesh)
		{
			Triangle3 dst_triangle; // local var

			// for each triangle on the surface
			for (Mesh::iterator iter = mesh->begin(); iter != mesh->end(); ++iter) 
			{
				const auto& src_triangle = Triangle3i(**iter);
				std::shared_ptr<Material> material = iter.triangle_material();
				const int material_index = int(std::distance(std::begin(this->materials_), this->materials_.find(material->name())));
				
				Vertex3 vertex;

				//for each triangle vertex
				for (int i = 0; i < 3; ++i) 
				{
					dst_triangle.vertices[i].position = src_triangle.position(i);
					dst_triangle.vertices[i].normal = src_triangle.normal(i);
					dst_triangle.vertices[i].color = Vector3(1.0f, 1.0f, 1.0f);
					dst_triangle.vertices[i].texture_coord = Vector2(src_triangle.texture_coord(i).x, src_triangle.texture_coord(i).y);
					dst_triangle.vertices[i].tangent = src_triangle.tangent(i);
					dst_triangle.vertices[i].material_index = material_index;

					vertices_.push_back(dst_triangle.vertices[i]);
				}
				//triangles_.push_back(dst_triangle);
				
			}
			printf("Mesh loaded\n");
		}
	}
	Triangle3 triangle;

	return 0;
}

/* loads vertexes from .obj  */
int Rasterizer::initBuffers()
{
	const int vertex_stride = sizeof(Vertex3); // velikost vertexu v bytech - pro krokovani v binarnim poli

	glGenVertexArrays( 1, &this->vao_ );
	glBindVertexArray(this->vao_ );
	
	glGenBuffers( 1, &this->vbo_);
	glBindBuffer( GL_ARRAY_BUFFER, this->vbo_ );
	glBufferData( GL_ARRAY_BUFFER, this->vertices_.size()*sizeof(Vertex3), this->vertices_.data(), GL_STATIC_DRAW );
	
	
	// LAYOUT 0 == VERTEX
	// layout=0, size=3, type=GL_FLOAT, normalized=false, stride=vertex_stride, pointer=offset from the start
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(offsetof(Vertex3, position)));
	glEnableVertexAttribArray( 0 );

	// LAYOUT 1 == NORMAL vec3
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(offsetof(Vertex3, normal)));
	glEnableVertexAttribArray( 1 );
	
	// LAYOUT 2 == TANGENT vec3
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(offsetof(Vertex3, tangent)));
	glEnableVertexAttribArray( 2 );
	
	// LAYOUT 3 == TEXTURE vec2
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vertex_stride, (void*)(offsetof(Vertex3, position)));
	glEnableVertexAttribArray( 3 );

	// LAYOUT 4 == MATERIAL INDEX
	glVertexAttribIPointer(4, 1, GL_INT, vertex_stride, (void*)(offsetof(Vertex3, position)));
	glEnableVertexAttribArray( 4 ); //kazdy index, ktery popiseme, musime zenablovat
	

	this->InitIrradianceMapTexture("../../../data/maps/lebombo_irradiance_map.exr");
	this->initPrefilteredEnvMapTexture();
	//this->InitOneEnvMapTexture();//D:\School\PGII\project\pg2\data\maps\lebombo_prefiltered_env_map\0.exr

	this->initIntegrationMapTexture("../../../data/maps/brdf_integration_map_ct_ggx.exr");

	this->setIrradianceMap(); // TEXTURE 0
	this->setPrefilteredEnvMap(); // TEXTURE 1
	this->setIntegrationMap(); // TEXTURE 2
	
	return 0;
}

/* inits vertex & fragment shaders, creates shader program  */
int Rasterizer::initShaders(const std::string& vert_path, const std::string& frag_path)
{
	// vytvorim vertex shader
	this->vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
	std::vector<char> shader_source;
	if (LoadShader(vert_path, shader_source) == S_OK) // natahneme zdrojak do shaderu
	{
		const char* tmp = static_cast<const char*>(&shader_source[0]);
		glShaderSource(this->vertex_shader_, 1, &tmp, nullptr); // nastavime zdrojovy soubor
		glCompileShader(this->vertex_shader_);
	}
	CheckShader(this->vertex_shader_);

	// vytvorim fragment shader
	this->fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
	if (LoadShader(frag_path, shader_source) == S_OK)
	{
		const char* tmp = static_cast<const char*>(&shader_source[0]);
		glShaderSource(this->fragment_shader_, 1, &tmp, nullptr);
		glCompileShader(this->fragment_shader_);
	}
	CheckShader(this->fragment_shader_);

	// vytvorim program
	this->shader_program_ = glCreateProgram();
	glAttachShader(this->shader_program_, this->vertex_shader_);
	glAttachShader(this->shader_program_, this->fragment_shader_);
	glLinkProgram(this->shader_program_);

	GLint program_linked;
	glGetProgramiv(this->shader_program_, GL_LINK_STATUS, &program_linked);
	if (program_linked != GL_TRUE) {
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(this->shader_program_, 1024, &log_length, message);
		printf(message);
	}

	glUseProgram(this->shader_program_);

	return 0;
}

int Rasterizer::mainLoop()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST); // zrusi pouziti z-bufferu, vykresleni se provede bez ohledu na poradi fragmentu z hlediska jejich pseudohloubky
	glEnable(GL_CULL_FACE); // zrusi zahazovani opacne orientovanych ploch
	glDepthFunc(GL_LESS);
	glDepthRangef(0.0f, 1.0f);
	// main loop
	while (!glfwWindowShouldClose(this->window_)) {
		
		#pragma region ---first pass ---
		//SHADOW STUFF GOES HERE

		// draw the scene
		glBindVertexArray(this->vao_);
		glDrawArrays(GL_TRIANGLES, 0, this->vertices_.size());
		glBindVertexArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// map from the range of NDC coordinates <-1.0, 1.0>^2 to <0, width> x <0, height>
		glViewport(0, 0, this->camera_.getWidth(), this->camera_.getHeight());
		// SET BACK THE MAIN SHADER PROGRAM AND THE VIEWPORT
		glUseProgram(this->shader_program_);
		
		#pragma endregion
		
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state setting function
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // state using function


		Matrix4x4 MVP = this->camera_.getMatrixMVP();
		SetMatrix4x4(this->shader_program_, MVP.data(), "MVP");

		Matrix4x4 Mn = this->camera_.getMatrixMn();
		SetMatrix4x4(this->shader_program_, Mn.data(), "Mn");

		Matrix4x4 MV = this->camera_.getMatrixMV();
		SetMatrix4x4(this->shader_program_, MV.data(), "MV");

		Matrix4x4 M = this->camera_.getMatrixM();
		SetMatrix4x4(this->shader_program_, M.data(), "M");

		//Matrix4x4 MLP = light_.getMatrixMVP();	
		//SetMatrix4x4(shader_program_, MLP.data(), "MLP");

		
		Vector3 view_from = this->camera_.getViewFrom();
		std::vector<float> vector = { view_from.x, view_from.y, view_from.z };
		SetVector3(this->shader_program_, vector.data(), "camera_pos");
		
		setIrradianceMap();
		setPrefilteredEnvMap();
		setIntegrationMap();

		glBindVertexArray(this->vao_);

		glDrawArrays(GL_TRIANGLES, 0, this->vertices_.size());

		glfwSwapBuffers(this->window_);
		glfwPollEvents();
	}

	glDeleteShader(this->vertex_shader_);
	glDeleteShader(this->fragment_shader_);
	glDeleteProgram(this->shader_program_);

	glDeleteBuffers(1, &this->vbo_);
	glDeleteVertexArrays(1, &this->vao_);

	glfwTerminate();

	return EXIT_SUCCESS;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		Camera& camera = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window))->getCamera();
		if(glfwGetKey(window, GLFW_KEY_A)==GLFW_PRESS)camera.moveLeft();
		if(glfwGetKey(window, GLFW_KEY_D)==GLFW_PRESS)camera.moveRight();
		if(glfwGetKey(window, GLFW_KEY_W)==GLFW_PRESS)camera.moveForward();
		if(glfwGetKey(window, GLFW_KEY_S)==GLFW_PRESS)camera.moveBackward();
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	// TODO
	//printf("x:%f y:%f\n", xpos, ypos);
	Camera& camera = reinterpret_cast<Rasterizer*>(glfwGetWindowUserPointer(window))->getCamera();
	/*auto last_pos = camera.getLastMousePos();
	float xmove, ymove;
	xmove = (last_pos.x - float(xpos));
	ymove = (last_pos.y - float(ypos));
	camera.adjustPitchAndYaw(ymove, xmove);
	camera.setLastMousePos(Vector2(xpos, ypos));*/

	float rotX = ((float)((xpos - camera.getWidth() / 2)) / camera.getWidth());
	float rotZ = ((float)((ypos - camera.getHeight() / 2)) / camera.getHeight());
	printf("rx:%f rz:%f\n", rotX, rotZ);

	//camera.moveCameraAngle(-rotX, -rotZ);

	glfwSetCursorPos(window, 0, 0);
}

/* TEXTURE 0*/
void Rasterizer::InitIrradianceMapTexture(const std::string& file_name)
{
	Texture3f irradiance_map = Texture3f(file_name);

	glGenTextures(1, &this->tex_irradiance_map_);
	glBindTexture(GL_TEXTURE_2D, tex_irradiance_map_);
	if (glIsTexture(tex_irradiance_map_)) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// for HDR images use GL_RGB32F or GL_RGB16F as internal format !!!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
			irradiance_map.width(), irradiance_map.height(), 0,
			GL_RGB, GL_FLOAT, irradiance_map.data());
		//glGenerateMipmap( GL_TEXTURE_2D );
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	
}
/* TEXTURE 0 */
int Rasterizer::setIrradianceMap()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->tex_irradiance_map_);

	SetSampler(this->shader_program_, 0, "irradiance_map");

	return S_OK;
}


/* TEXTURE 1 */
void Rasterizer::initPrefilteredEnvMapTexture() {
	int max_level = 6;

	glGenTextures(1, &this->tex_prefilteredEnv_map_);
	glBindTexture(GL_TEXTURE_2D, tex_prefilteredEnv_map_);

	if (glIsTexture(tex_prefilteredEnv_map_)) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_level);
		GLint level;

		for ( level = 0; level <= max_level; ++level) {
			auto filename = "D:/School/PGII/project/pg2/data/maps/lebombo_prefiltered_env_map/" + std::to_string(level) + ".exr";
			auto tex = Texture3f(filename);
			glTexImage2D(GL_TEXTURE_2D, level, GL_RGB32F, tex.width(), tex.height(), 0, GL_RGB, GL_FLOAT, tex.data());
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

/* TEXTURE 1 */
void Rasterizer::InitOneEnvMapTexture()
{
	auto filename = "D:/School/PGII/project/pg2/data/maps/lebombo_prefiltered_env_map/2.exr";
	Texture3f prefilteredEnv_map = Texture3f(filename);

	glGenTextures(1, &this->tex_prefilteredEnv_map_);
	glBindTexture(GL_TEXTURE_2D, this->tex_prefilteredEnv_map_);
	if (glIsTexture(this->tex_prefilteredEnv_map_))
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// for HDR images use GL_RGB32F or GL_RGB16F as internal format !!!
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
			prefilteredEnv_map.width(), prefilteredEnv_map.height(), 0,
			GL_RGB, GL_FLOAT, prefilteredEnv_map.data());
		//glGenerateMipmap( GL_TEXTURE_2D );
	}
	glBindTexture(GL_TEXTURE_2D, 0);


}

/* TEXTURE 1 */
int Rasterizer::setPrefilteredEnvMap()
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->tex_prefilteredEnv_map_);

	SetSampler(this->shader_program_, 1, "prefilteredEnv_map");

	return S_OK;
}

/* TEXTURE 2 */
void Rasterizer::initIntegrationMapTexture(const std::string& file_name)
{
	glGenTextures(1, &this->tex_integration_map_);
	glBindTexture(GL_TEXTURE_2D, this->tex_integration_map_);

	if (glIsTexture(this->tex_integration_map_)) {
		auto tex = Texture3f(file_name);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, tex.width(), tex.height(), 0, GL_RGB, GL_FLOAT, tex.data());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

/* TEXTURE 2 */
int Rasterizer::setIntegrationMap()
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_integration_map_);

	SetSampler(shader_program_, 2, "integration_map");

	return S_OK;
}

/* load shader code from the text file */
int Rasterizer::LoadShader(const std::string& file_name, std::vector<char>& shader)
{
	FILE* file = fopen(file_name.c_str(), "rt");

	if (file == NULL)
	{
		printf("IO error: File '%s' not found.\n", file_name.c_str());
		return NULL;
	}

	int result = S_FALSE;

	const size_t file_size = static_cast<size_t>(GetFileSize64(file_name.c_str()));

	if (file_size < 1)
	{
		printf("Shader error: File '%s' is empty.\n", file_name.c_str());
	}
	else
	{
		/* in glShaderSource we don't set the length in the last parameter,
		so the string must be null terminated, therefore +1 and reset to 0 */
		shader.clear();
		shader.resize(file_size + 1);

		size_t bytes = 0; // number of already loaded bytes

		do
		{
			bytes += fread(shader.data(), sizeof(char), file_size, file);
		} while (!feof(file) && (bytes < file_size));

		if (!feof(file) && (bytes != file_size))
		{
			printf("IO error: Unexpected end of file '%s' encountered.\n", file_name.c_str());
		}
		else
		{
			printf("Shader file '%s' loaded successfully.\n", file_name.c_str());
			result = S_OK;
		}
	}

	fclose(file);
	file = nullptr;

	return result;
}

/* check shader for completeness */
GLint Rasterizer::CheckShader(const GLenum shader)
{
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	printf("Shader compilation %s.\n", (status == GL_TRUE) ? "was successful" : "FAILED");

	if (status == GL_FALSE)
	{
		int info_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
		char* info_log = new char[info_length];
		memset(info_log, 0, sizeof(*info_log) * info_length);
		glGetShaderInfoLog(shader, info_length, &info_length, info_log);

		printf("Error log: %s\n", info_log);

		SAFE_DELETE_ARRAY(info_log);
	}

	return status;
}

/* OpenGL check state */
bool check_gl_1(const GLenum error) {
	if (error != GL_NO_ERROR) {
		return false;
	}
	return true;
}

/* glfw callback */
void glfw_callback_1(const int error, const char* description) {
	printf("GLFW Error (%d): %s\n", error, description);
}

/* OpenGL messaging callback */
void GLAPIENTRY gl_callback_1(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param) {
	printf("GL %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "Error" : "Message"),
		type, severity, message);
}

/* invoked when window is resized */
void framebuffer_resize_callback_1(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void Rasterizer::resize(const int width, const int height) {
	glViewport(0, 0, width, height);
	this->camera_.update(width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glDeleteRenderbuffers(1, &tex_shadow_map_);
	//glDeleteFramebuffers(1, &fbo_shadow_map);

	initBuffers();
}
