/*
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

// For models - we already include files that shouldnt be redefined somewhere so extra checks in place
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))



static GLFWwindow* window;
static int windowWidth = 2048;
static int windowHeight = 1536;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void cursor_callback(GLFWwindow* window, double xpos, double ypos);

// OpenGL camera view parameters
static glm::vec3 cameraPosition(0.0f, 75.0f, 0.0f);
static glm::vec3 cameraLookVector(0, 0, -1.0f);	// start looking at back wall
static glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 0.1f;
static float zFar = 5000.0f;

// initialising variables for mouse camera controls
double yaw = -90.0f;	// has to start at -90.0 degrees so doesnt start pointing right
double pitch = 0.0f;
double previousX = windowWidth / 2.0;
double previousY = windowHeight / 2.0;

// Getting time differences between frames - allows for smoove movement
float deltaTime = 0.0f;	// time between this frame and the previous frame
float previousTime = 0.0f;	// time of the last frame

// Lighting control
const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 5.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);


// For heightmap Heightmap.c
#define MAX_CIRCLE_SIZE (50.0f)
#define MAX_DISPLACEMENT (5.0f)
#define DISPLACEMENT_SIGN_LIMIT (0.3f)
#define MAX_ITER (2000)
#define MAP_SIZE (2000.0f)
#define MAP_NUM_VERTICES (100)
#define MAP_NUM_TOTAL_VERTICES (MAP_NUM_VERTICES*MAP_NUM_VERTICES)



// function for loading textures
static GLuint LoadTextureTileBox(const char* texture_file_path, bool flip) {
	int w, h, channels;
	stbi_set_flip_vertically_on_load(flip); // This flips our texture along its x axis. Whether or not to flip is decide by parameter flip
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);// This or Linear
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (img) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		/////////TEST
		int testVariable = 1;    // This line is throwing the excepttion
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture " << texture_file_path << std::endl;
	}
	stbi_image_free(img);

	return texture;
}

struct Skybox {
	glm::vec3 position;		// Position of the box - should be equal
	glm::vec3 scale;		// Size of the skybox in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// if we swap position of two opposite corners, then rotate 90 degrees to right, it swaps
		// left - all good
		//  z,    y,   x
		-1.0f, -1.0f, 1.0f,		// top left
		-1.0f, 1.0f, 1.0f,		// bottom left
		1.0f, 1.0f, 1.0f,		// bottom right
		1.0f, -1.0f, 1.0f,		// top right

		// right - All good
		1.0f, -1.0f, -1.0f,		// bottom right
		1.0f, 1.0f, -1.0f,		// top right
		-1.0f, 1.0f, -1.0f,		// top left
		-1.0f, -1.0f, -1.0f,	// bottom left

		// front - All good
		-1.0f, -1.0f, -1.0f,	// bottom left
		-1.0f, 1.0f, -1.0f,		// top left
		-1.0f, 1.0f, 1.0f,		// top right
		-1.0f, -1.0f, 1.0f,		// bottom right

		// behind - All good
		1.0f, -1.0f, 1.0f,		// bottom right
		1.0f, 1.0f, 1.0f,		// top right
		1.0f, 1.0f, -1.0f,		// top left
		1.0f, -1.0f, -1.0f,		// bottom left

		// top - All good
		-1.0f, 1.0f, -1.0f,		//
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,		//
		-1.0f, 1.0f, 1.0f,

		// bottom - pointing outwards (flip by swapping zs)
		1.0f, -1.0f, -1.0f,		//
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,		//
		1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};


	// Here I am making everything end 1 before it actually should because I was getting black lines in the skybox
	GLfloat uv_buffer_data[48] = {
		// x, y
		// left
		0.25f, 0.334f,		// bottom right
		0.25f, 0.666f,		// top right
		0.0f, 0.666f,		// top left					BRING BACK
		0.0f, 0.334f,		// bottom left
		// right
		0.75f, 0.334f,
		0.75f, 0.666f,
		0.5f, 0.666f,
		0.5f, 0.334f,
		// front
		0.5f, 0.334f,		// bottom right
		0.5f, 0.666f,		// top right
		0.25f, 0.666f,		// top left
		0.25f, 0.334f,		// bottom left
		// behind
		1.0f, 0.334f,
		1.0f, 0.666f,
		0.75f, 0.666f,
		0.75f, 0.334f,
		// Top
		0.5f, 0.667f,		//
		0.5f, 1.0f,			//
		0.251f, 1.0f,		// X is higher to avoid a black line
		0.251f, 0.667f,		//
		// Bottom
		0.499f, 0.0f,
		0.499f, 0.334f,
		0.25f, 0.334f,
		0.25f, 0.0f,

	};

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	void initialize(glm::vec3 position, glm::vec3 scale) {
		// Define scale of the building geometry
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the UV data
		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		//programID = LoadShadersFromFile("../lab2/box.vert", "../lab2/box.frag");
		programID = LoadShadersFromFile("../../../wonderland/Skybox_Files/skybox.vert",
			"../../../wonderland/Skybox_Files/skybox.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");

		// Load a texture
		textureID = LoadTextureTileBox("../../../wonderland/Skybox_Files/Skybox_1.png", true);

		// Get a handle to texture sampler
		textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glBindVertexArray(vertexArrayID); // solves skybox dissapearing when lamp rendered

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// Model transform
		glm::mat4 modelMatrix = glm::mat4();

		// Translate the box so it is always centered on the player as they move around
		modelMatrix = glm::translate(modelMatrix, position);
		// Scale the box along each axis
		modelMatrix = glm::scale(modelMatrix, scale);

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


		// Enable UV buffer and texture sampler
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Set textureSampler to use texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);


		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		//glDisableVertexAttribArray(2);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteBuffers(1, &uvBufferID);
		glDeleteTextures(1, &textureID);
		glDeleteProgram(programID);
	}
};

struct CornellBox {

	GLfloat vertex_buffer_data[60];
	GLfloat normal_buffer_data[60];
	GLfloat color_buffer_data[60];

	// Refer to original Cornell Box data
	// from https://www.graphics.cornell.edu/online/box/data.html

	GLuint index_buffer_data[30] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,
	};

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint normalBufferID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;

	void initialize(GLfloat vertex_buffer_data[60], GLfloat normal_buffer_data[60], GLfloat color_buffer_data[60]) {

		memcpy(this->vertex_buffer_data, vertex_buffer_data, sizeof(this->vertex_buffer_data));
		memcpy(this->normal_buffer_data, normal_buffer_data, sizeof(this->normal_buffer_data));
		memcpy(this->color_buffer_data, color_buffer_data, sizeof(this->color_buffer_data));

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertex_buffer_data), this->vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->color_buffer_data), this->color_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the vertex normals
		glGenBuffers(1, &normalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->normal_buffer_data), this->normal_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../../../wonderland/wonderland_window.vert", "../../../wonderland/wonderland_window.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		// Set light data
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			30,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteBuffers(1, &normalBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteProgram(programID);
	}
};

struct Heightmap
{
	static constexpr int N = MAP_NUM_VERTICES;
	static constexpr int TOTAL = MAP_NUM_TOTAL_VERTICES;

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);	// The starting camera position but with y = 0

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec2> uvs;

	GLuint vertexArrayID, vertexBufferID, indexBufferID, uvBufferID, textureID;
	GLuint programID;
	GLuint mvpMatrixID;
	GLuint textureSamplerID;

	void initialize()
	{
		initialiseMap();
		generateIndices();

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
		// Using Dynamic Draw because the map can often change

		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

		programID = LoadShadersFromFile(
			"../../../wonderland/heightmap.vert",
			"../../../wonderland/heightmap.frag"
		);

		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		mvpMatrixID = glGetUniformLocation(programID, "MVP");

		//Loading the texture
		textureID = LoadTextureTileBox("../../../wonderland/Ground_Textures/IMGP1394.jpg", false);

		// Get a handle to texture sampler
		textureSamplerID = glGetUniformLocation(programID, "terrainTextureSampler");
	}

	// Generate vertices and indices for the heightmap
	void initialiseMap()
	{
		vertices.resize(TOTAL);
		uvs.resize(TOTAL);

		GLfloat step = MAP_SIZE / (MAP_NUM_VERTICES - 1);
		GLfloat x = -MAP_SIZE / 2;	//0,0 should be its center
		GLfloat z = -MAP_SIZE / 2;
		// Create a flat grid
		int k = 0;
		for (int i = 0; i < MAP_NUM_VERTICES; ++i)
		{
			for (int j = 0; j < MAP_NUM_VERTICES; ++j)
			{
				vertices[k] = glm::vec3(
					x,
					0.0f,
					z
				);
				z += step;

				// setting up the uv
				float u = (float)i / (MAP_NUM_VERTICES - 1);
				float v = (float)j / (MAP_NUM_VERTICES - 1);
				uvs[k] = glm::vec2(u, v);

				++k;
			}
			x += step;
			z = -MAP_SIZE / 2;
		}
	}

	void generateIndices()
	{
		for (int x = 0; x < N - 1; ++x)
		{
			for (int z = 0; z < N - 1; ++z)
			{
				int i = x * N + z;

				indices.push_back(i);
				indices.push_back(i + 1);
				indices.push_back(i + N);

				indices.push_back(i + 1);
				indices.push_back(i + N + 1);
				indices.push_back(i + N);
			}
		}
	}

	void updateMap()
	{
		float center_x = ((float)rand() / RAND_MAX - 0.5f) * MAP_SIZE;
		float center_z = ((float)rand() / RAND_MAX - 0.5f) * MAP_SIZE;
		float circle_size = (MAX_CIRCLE_SIZE * rand()) / RAND_MAX;
		float sign = ((float)rand() / RAND_MAX) < DISPLACEMENT_SIGN_LIMIT ? -1.0f : 1.0f;
		float disp = (sign * (MAX_DISPLACEMENT * rand())) / RAND_MAX;

		//disp = disp / 2.0f;   -  Removing this

		for (auto& v : vertices) // For all vertices
		{
			GLfloat dx = center_x - v.x;	// x distance
			GLfloat dz = center_z - v.z;	// y distance from
			GLfloat pd = (sqrt((dx * dx) + (dz * dz))) / circle_size; // it recommends removing the 2 *
			if (fabs(pd) <= 1.0f)
			{
				v.y += disp * cos(pd*pd * glm::pi<float>());
			}
		}

		// Upload new heights to GPU
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data());
	}

	void render(glm::mat4 cameraMatrix)
	{
		glUseProgram(programID);

		glBindVertexArray(vertexArrayID); // not sure if this is needed

		// Adding these in to match previou stuff
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


		glEnableVertexAttribArray(1);	// Think thi needs to change to 1 - but some other stuff also needs to change
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// TODO: make the size the same as the skybox, and center it on the player
		glm::mat4 modelMatrix = glm::mat4();

		modelMatrix = glm::translate(modelMatrix, position); //  Have to make this Y not change

		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


		// Set textureSampler to use texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);

		glDrawElements(
			GL_TRIANGLES,
			indices.size(),
			GL_UNSIGNED_INT,
			(void*)0);

		//Another add in to match
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	// A function so the ground's y position doesnt change
	void updatePosition(glm::vec3 cameraPosition) {
		cameraPosition.y = 0;
		position = cameraPosition;
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &indexBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteProgram(programID);
	}
};

struct Lampost {
	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint lightPositionID;
	GLuint lightIntensityID;
	GLuint programID;

	tinygltf::Model model;

	struct PrimitiveObject {
		GLuint vao;
		std::map<int, GLuint> vbos;
	};
	std::vector<PrimitiveObject> primitiveObjects;


	glm::mat4 getNodeTransform(const tinygltf::Node& node) {
		glm::mat4 transform(1.0f);

		if (node.matrix.size() == 16) {
			transform = glm::make_mat4(node.matrix.data());
		}
		else {
			if (node.translation.size() == 3)
				transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));

			if (node.rotation.size() == 4) {
				glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
				transform *= glm::mat4_cast(q);
			}

			if (node.scale.size() == 3)
				transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}
		return transform;
	}


	bool loadModel(tinygltf::Model& model, const char* filename) {
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}

		if (!err.empty()) {
			std::cout << "ERR: " << err << std::endl;
		}

		if (!res)
			std::cout << "Failed to load glTF: " << filename << std::endl;
		else
			std::cout << "Loaded glTF: " << filename << std::endl;

		return res;
	}

	void initialize() {
		// Modify your path if needed
		if (!loadModel(model, "../../../wonderland/Lampost/rusticLamps.gltf")) {
			return;
		}

		// Prepare buffers for rendering
		primitiveObjects = bindModel(model);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../../../wonderland/lampost.vert", "../../../wonderland/lampost.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for GLSL variables
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		lightPositionID = glGetUniformLocation(programID, "lightPosition");
		lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
	}


	void bindMesh(std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model& model, tinygltf::Mesh& mesh) {

		std::map<int, GLuint> vbos;
		for (size_t i = 0; i < model.bufferViews.size(); ++i) {
			const tinygltf::BufferView& bufferView = model.bufferViews[i];

			int target = bufferView.target;

			if (bufferView.target == 0) {
				// The bufferView with target == 0 in our model refers to
				// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes.
				// So it is considered safe to skip the warning.
				//std::cout << "WARN: bufferView.target is zero" << std::endl;
				continue;
			}

			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
			GLuint vbo;
			glGenBuffers(1, &vbo);
			glBindBuffer(target, vbo);
			glBufferData(target, bufferView.byteLength,
				&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

			vbos[i] = vbo;
		}

		// Each mesh can contain several primitives (or parts), each we need to
		// bind to an OpenGL vertex array object
		for (size_t i = 0; i < mesh.primitives.size(); ++i) {

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			GLuint vao;
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			for (auto& attrib : primitive.attributes) {
				tinygltf::Accessor accessor = model.accessors[attrib.second];
				int byteStride =
					accessor.ByteStride(model.bufferViews[accessor.bufferView]);
				glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

				int size = 1;
				if (accessor.type != TINYGLTF_TYPE_SCALAR) {
					size = accessor.type;
				}

				int vaa = -1;
				if (attrib.first.compare("POSITION") == 0) vaa = 0;
				if (attrib.first.compare("NORMAL") == 0) vaa = 1;
				if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
				if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
				if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
				if (vaa > -1) {
					glEnableVertexAttribArray(vaa);
					glVertexAttribPointer(vaa, size, accessor.componentType,
						accessor.normalized ? GL_TRUE : GL_FALSE,
						byteStride, BUFFER_OFFSET(accessor.byteOffset));
				}
				else {
					std::cout << "vaa missing: " << attrib.first << std::endl;
				}
			}

			// Record VAO for later use
			PrimitiveObject primitiveObject;
			primitiveObject.vao = vao;
			primitiveObject.vbos = vbos;
			primitiveObjects.push_back(primitiveObject);

			glBindVertexArray(0);
		}
	}

	void bindModelNodes(std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model& model,
		tinygltf::Node& node) {
		// Bind buffers for the current mesh at the node
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}

		// Recursive into children nodes
		for (size_t i = 0; i < node.children.size(); i++) {
			assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
		}
	}

	std::vector<PrimitiveObject> bindModel(tinygltf::Model& model) {
		std::vector<PrimitiveObject> primitiveObjects;

		const tinygltf::Scene& scene = model.scenes[model.defaultScene];
		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
			bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
		}

		return primitiveObjects;
	}

	void drawMesh(const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model& model, tinygltf::Mesh& mesh) {

		for (size_t i = 0; i < mesh.primitives.size(); ++i)
		{
			GLuint vao = primitiveObjects[i].vao;
			std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

			glBindVertexArray(vao);

			tinygltf::Primitive primitive = mesh.primitives[i];
			tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

			glDrawElements(primitive.mode, indexAccessor.count,
				indexAccessor.componentType,
				BUFFER_OFFSET(indexAccessor.byteOffset));

			glBindVertexArray(0);
		}
	}

	void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model& model, tinygltf::Node& node, glm::mat4 parentTransform,
		glm::mat4 cameraMatrix) {

		glm::mat4 localTransform = parentTransform * getNodeTransform(node);

		// Draw the mesh at the node, and recursively do so for children nodes
		if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
			glm::mat4 mvp = cameraMatrix * localTransform;
			glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
			drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
		}
		for (size_t i = 0; i < node.children.size(); i++) {
			drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]], localTransform, cameraMatrix);
		}
	}

	void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
		tinygltf::Model& model, glm::mat4 cameraMatrix,
		glm::mat4 worldTransform) {
		// Draw all nodes
		const tinygltf::Scene& scene = model.scenes[model.defaultScene];

		for (size_t i = 0; i < scene.nodes.size(); ++i) {
			drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]], worldTransform,
				cameraMatrix);
		}
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);
		glEnable(GL_DEPTH_TEST);

		//
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 50.0f, -200.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(200.0f));

		// Set camera
		// glm::mat4 mvp = cameraMatrix;
		// glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


		// setting just a generic color for now
		glm::vec3 objectColor(1.0f, 0.5f, 0.0f);
		glUniform3fv(glGetUniformLocation(programID, "color"), 1, &objectColor[0]);

		// Set light data
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

		// Draw the GLTF model
		drawModel(primitiveObjects, model, cameraMatrix, modelMatrix);
	}

	void cleanup() {
		glDeleteProgram(programID);
	}
};


// ------------------------------------------------------
// ------------------------------------------------------

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "Wonderland", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	glfwSetCursorPosCallback(window, cursor_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // capture our mouse so it stays centred in our frame

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}


	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);




	GLfloat tallBoxVertexBufferData[60] = {
		// This doesnt include one of the vertexes  - its fine its not visible
		-423.0, 330.0, -247.0,
		-265.0, 330.0, -296.0,
		-314.0, 330.0, -456.0,
		-472.0, 330.0, -406.0,

		-423.0,   0.0, -247.0,
		-423.0, 330.0, -247.0,
		-472.0, 330.0, -406.0,
		-472.0,   0.0, -406.0,

		-472.0,   0.0, -406.0,
		-472.0, 330.0, -406.0,
		-314.0, 330.0, -456.0,
		-314.0,   0.0, -456.0,

		-314.0,   0.0, -456.0,
		-314.0, 330.0, -456.0,
		-265.0, 330.0, -296.0,
		-265.0,   0.0, -296.0,

		-265.0,   0.0, -296.0,
		-265.0, 330.0, -296.0,
		-423.0, 330.0, -247.0,
		-423.0,   0.0, -247.0
	};

	GLfloat tallBoxNormalBufferData[60] = {
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,

		0.956, 0.0, 0.295,
		0.956, 0.0, 0.295,
		0.956, 0.0, 0.295,
		0.956, 0.0, 0.295,

		0.302, 0.0, -0.953,
		0.302, 0.0, -0.953,
		0.302, 0.0, -0.953,
		0.302, 0.0, -0.953,

		-0.955, 0.0, -0.293,
		-0.955, 0.0, -0.293,
		-0.955, 0.0, -0.293,
		-0.955, 0.0, -0.293,

		-0.296, 0.0, 0.955,
		-0.296, 0.0, 0.955,
		-0.296, 0.0, 0.955,
		-0.296, 0.0, 0.955,
	};

	GLfloat smallBoxVertexBufferData[60] = {
		-130.0, 165.0,  -65.0,
		 -82.0, 165.0, -225.0,
		-240.0, 165.0, -272.0,
		-290.0, 165.0, -114.0,

		-290.0,   0.0, -114.0,
		-290.0, 165.0, -114.0,
		-240.0, 165.0, -272.0,
		-240.0,   0.0, -272.0,

		-130.0,   0.0,  -65.0,
		-130.0, 165.0,  -65.0,
		-290.0, 165.0, -114.0,
		-290.0,   0.0, -114.0,

		 -82.0,   0.0, -225.0,
		 -82.0, 165.0, -225.0,
		-130.0, 165.0,  -65.0,
		-130.0,   0.0,  -65.0,

		-240.0,   0.0, -272.0,
		-240.0, 165.0, -272.0,
		 -82.0, 165.0, -225.0,
		 -82.0,   0.0, -225.0
	};

	GLfloat smallBoxNormalBufferData[60] = {
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 1.0, 0.0,

		0.953, 0.0, 0.302,
		0.953, 0.0, 0.302,
		0.953, 0.0, 0.302,
		0.953, 0.0, 0.302,

		0.293, 0.0, -0.957,
		0.293, 0.0, -0.957,
		0.293, 0.0, -0.957,
		0.293, 0.0, -0.957,

		-0.958, 0.0, -0.287,
		-0.958, 0.0, -0.287,
		-0.958, 0.0, -0.287,
		-0.958, 0.0, -0.287,

		-0.285, 0.0, 0.958,
		-0.285, 0.0, 0.958,
		-0.285, 0.0, 0.958,
		-0.285, 0.0, 0.958
	};

	GLfloat otherBoxColorBufferData[60] = {
		// Floor
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Ceiling
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Left wall
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Right wall
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		// Back wall
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f
	};


	Skybox skybox;
	skybox.initialize(cameraPosition, glm::vec3(1000, 1000, 1000));  // Scale x,y,z


	CornellBox tallBox, smallBox;
	tallBox.initialize(tallBoxVertexBufferData, tallBoxNormalBufferData, otherBoxColorBufferData);
	smallBox.initialize(smallBoxVertexBufferData, smallBoxNormalBufferData, otherBoxColorBufferData);

	// Setting up the ground
	Heightmap ground;
	ground.initialize();
	//int iterationNum = 0;
	for (int i = 0; i <= MAX_ITER; i++) {
		ground.updateMap();
	}

	Lampost lampost;
	lampost.initialize();


	// Camera setup
	glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	// -----------------------------------------------------------
	// -----------------------------------------------------------

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Getting timing for frames
		float currentTime = static_cast<float>(glfwGetTime());
		deltaTime = currentTime - previousTime;
		previousTime = currentTime;

		// lookAt( where camera is, where its looking at relative to where it is, its up )
		viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraLookVector, cameraUp);
		glm::mat4 vp = projectionMatrix * viewMatrix;


		skybox.position = cameraPosition;
		skybox.render(vp);

		tallBox.render(vp);
		smallBox.render(vp);

		ground.updatePosition(cameraPosition);
		ground.render(vp);

		lampost.render(vp);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	skybox.cleanup();

	tallBox.cleanup();
	smallBox.cleanup();

	ground.cleanup();

	lampost.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		cameraPosition = glm::vec3(150.0f, 150.0f, 150.0f);
		cameraLookVector = glm::vec3(0.0f, 0.0f, -1.0f);
	}

	// number is arbitrarily chosen, replace with what seems fitting
	// DIdnt think It would have to be one so large
	float cameraSpeed = static_cast<float>(5000 * deltaTime);

	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		//	Move the camera in the direction we are looking
		cameraPosition += cameraSpeed * cameraLookVector;
	}

	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		// Move right relative to where we are looking
		cameraPosition -= glm::normalize(glm::cross(cameraLookVector, cameraUp)) * cameraSpeed;
	}

	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		// Move backwards
		cameraPosition -= cameraSpeed * cameraLookVector;
	}

	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		// Left relative to where looking
		cameraPosition += glm::normalize(glm::cross(cameraLookVector, cameraUp)) * cameraSpeed;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}


static void cursor_callback(GLFWwindow* window, double xPos, double yPos) {

	// getting change in where the mouse is
	double changeInX = xPos - previousX;
	double changeInY = previousY - yPos;
	previousX = xPos;
	previousY = yPos;

	// sensitivity is arbitrarily chosen, replace with what seems fitting
	double sensitivity = 0.1f;
	changeInX *= sensitivity;
	changeInY *= sensitivity;

	yaw += changeInX;
	pitch += changeInY;

	// If pitch goes above/ below 90 it goes negative. Dont want that so clamp it to a value just below
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 tempVec(
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	);
	cameraLookVector = glm::normalize(tempVec);
}




/* Files obtained from :

Ground / skybox textures:      https://opengameart.org/

Lampost:   https://www.cgtrader.com/free-3d-models/architectural/lighting/lampposts-with-hanging-lanterns
*/