#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>


static GLFWwindow* window;
static int windowWidth = 1024;
static int windowHeight = 768;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
static void cursor_callback(GLFWwindow* window, double xpos, double ypos);

// OpenGL camera view parameters
static glm::vec3 eye_center(150.0f, 150.0f, 150.0f);
static glm::vec3 lookat(150.0f, 150.0f, -150.0f);	// start looking at back wall
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 0.1f;
static float zFar = 1500.0f;


// Lighting control 
const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
static glm::vec3 lightIntensity = 5.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 lightPosition(-275.0f, 500.0f, -275.0f);

// function for loading textures
static GLuint LoadTextureTileBox(const char* texture_file_path) {
	int w, h, channels;
	stbi_set_flip_vertically_on_load(true); // This flips our texture along its x axis. This means reversing the image for the skybox is not flipped
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //Changing this to Clamp to Edge because only small gaps in 1 direction
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);// This or Linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	if (img) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture " << texture_file_path << std::endl;
	}
	stbi_image_free(img);

	return texture;
}

struct Skybox {
	glm::vec3 scale;		// Size of the box in each axis

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

	void initialize(glm::vec3 scale) {
		// Define scale of the building geometry
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
		programID = LoadShadersFromFile("C:/Users/finn_/Desktop/College_Programming/Computer_Graphics/lab2/lab2/box.vert",
			"C:/Users/finn_/Desktop/College_Programming/Computer_Graphics/lab2/lab2/box.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");

		// Load a texture
		textureID = LoadTextureTileBox("C:/Users/finn_/Desktop/College_Programming/Computer_Graphics/lab2/lab2/studio_garden.png");

		// Get a handle to texture sampler 
		textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// Model transform 
		// -----------------------
		glm::mat4 modelMatrix = glm::mat4();

		// Scale the box along each axis
		// TODO : Have the skybox move with the user as they walk around
		modelMatrix = glm::scale(modelMatrix, scale); // Scale the skybox relative to the global eye position attributes
		// -----------------------

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
		programID = LoadShadersFromFile("C:/Users/finn_/Desktop/College_Programming/Computer_Graphics/wonderland/wonderland/wonderland_window.vert", "C:/Users/finn_/Desktop/College_Programming/Computer_Graphics/wonderland/wonderland/wonderland_window.frag");
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
	glEnable(GL_CULL_FACE);


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
	skybox.initialize(glm::vec3(500, 500, 500));  // Scale x,y,z


	CornellBox tallBox, smallBox;
	//Box tallBox, smallBox;
	tallBox.initialize(tallBoxVertexBufferData, tallBoxNormalBufferData, otherBoxColorBufferData);
	smallBox.initialize(smallBoxVertexBufferData, smallBoxNormalBufferData, otherBoxColorBufferData);


	// Camera setup
	glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		skybox.render(vp);

		tallBox.render(vp);
		smallBox.render(vp);


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	skybox.cleanup();

	tallBox.cleanup();
	smallBox.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		eye_center = glm::vec3(150.0f, 150.0f, 150.0f);
		lookat = glm::vec3(150.0f, 150.0f, -150.0f);

	}

	// change where the skybox is centred relative to the person

	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		eye_center.z -= 20.0f;
		// skybox.x -= 20.0f    // Need to make skybox variables global
	}

	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		eye_center.x -= 20.0f;
		lookat.x -= 20.0f;
	}

	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		eye_center.z += 20.0f;
	}

	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		eye_center.x += 20.0f;
		lookat.x += 20.0f;
	}


	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		lookat.y += 20.0f;
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		lookat.y -= 20.0f;
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		lookat.x -= 20.0f;
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		lookat.x += 20.0f;
	}

	/*
	if (key == GLFW_KEY_SPACE && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		saveDepth = true;
	}
	*/

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}


static void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
	/*
	if (xpos < 0 || xpos >= windowWidth || ypos < 0 || ypos > windowHeight)
		return;

	// Normalize to [0, 1] 
	float x = xpos / windowWidth;
	float y = ypos / windowHeight;

	// To [-1, 1] and flip y up 
	x = x * 2.0f - 1.0f;
	y = 1.0f - y * 2.0f;

	const float scale = 250.0f;
	lightPosition.x = x * scale - 278;
	lightPosition.y = y * scale + 278;

	//std::cout << lightPosition.x << " " << lightPosition.y << " " << lightPosition.z << std::endl;
	*/
}


//  For infinite scrollablility just have the skybox be centered on wherever the person is
