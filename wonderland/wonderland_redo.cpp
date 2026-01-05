#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include <stb/stb_image_write.h>
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
static glm::vec3 cameraPosition(150.0f, 50.0f, 150.0f);
static glm::vec3 cameraLookVector(0, 0, -1.0f);	// start looking at back wall
static glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 0.1f;
static float zFar = 2500.0f;

// initialising variables for mouse camera controls
double yaw = -90.0f;	// has to start at -90.0 degrees so doesnt start pointing right
double pitch = 0.0f;
double previousX = windowWidth / 2.0;
double previousY = windowHeight / 2.0;

// Getting time differences between frames - allows for smoove movement
float deltaTime = 0.0f;	// time between this frame and the previous frame
float previousFrame = 0.0f;	// time of the last frame


// function for loading textures
static GLuint LoadTextureTileBox(const char* texture_file_path) {
	int w, h, channels;
	stbi_set_flip_vertically_on_load(true); // This flips our texture along its x axis. This means reversing the image for the skybox is not flipped
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // This must be added for uneven file sizes to prevent crashes

	// To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //Changing this to Clamp to Edge because only small gaps in 1 direction
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);// This or Linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
		textureID = LoadTextureTileBox("../../../wonderland/Skybox_Files/Skybox_1.png");

		// Get a handle to texture sampler 
		textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glBindVertexArray(vertexArrayID);

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

struct Ground
{
	glm::vec3 position;

	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	GLfloat ground_vertex_buffer_data[12] = {
	-0.5f, 0.0f, -0.5f,
	 0.5f, 0.0f, -0.5f,
	 0.5f, 0.0f,  0.5f,
	-0.5f, 0.0f,  0.5f
	};

	GLfloat ground_uv_buffer_data[8] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f,
	0.0f, 1.0f
	};

	GLuint ground_index_buffer_data[6] = {
	0, 2, 1,
	0, 3, 2
	};


	void initialize(glm::vec3 position)
	{
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertex_buffer_data), ground_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ground_uv_buffer_data), ground_uv_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ground_index_buffer_data), ground_index_buffer_data, GL_STATIC_DRAW);

		programID = LoadShadersFromFile("../../../wonderland/Ground_Files/ground.vert", "../../../wonderland/Ground_Files/ground.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");

		// Load a texture
		textureID = LoadTextureTileBox("../../../wonderland/Ground_Files/IMGP1394.jpg");

		// Get a handle to texture sampler 
		textureSamplerID = glGetUniformLocation(programID, "textureSampler");

		glBindVertexArray(0);
	}

	void render(const glm::mat4& cameraMatrix, float tileSize)
	{
		glUseProgram(programID);
		glBindVertexArray(vertexArrayID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);


		glm::mat4 modelMatrix = glm::mat4();
		modelMatrix = glm::translate(modelMatrix, position);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(tileSize));

		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);

		glDrawElements(GL_TRIANGLES,
			6,
			GL_UNSIGNED_INT,
			(void*)0
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
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
	glEnable(GL_CULL_FACE);

	Skybox skybox;
	skybox.initialize(cameraPosition, glm::vec3(500, 500, 500));  // Scale x,y,z

	// 3x3 grid for our ground so it appears infinite
	int gridSize = 3;
	float tileSize = 500.0f; // The size of each of the individual sections of ground
	std::vector<Ground> groundTiles(gridSize * gridSize);

	int index = 0;
	for (int x = -1; x <= 1; ++x)
	{
		for (int z = -1; z <= 1; ++z)
		{
			glm::vec3 startPosition;
			startPosition.x = x * tileSize;
			startPosition.y = 0.0f;
			startPosition.z = z * tileSize;

			groundTiles[index].initialize(startPosition);
			index++;
		}
	}


	// Camera setup
	glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	// -----------------------------------------------------------
	// -----------------------------------------------------------

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Getting timing for frames
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - previousFrame;
		previousFrame = currentFrame;

		// lookAt( where camera is, where its looking at relative to where it is, its up )
		viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraLookVector, cameraUp);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		skybox.position = cameraPosition;
		skybox.render(vp);

		// For "moving" the ground as the player moves
		int camTileX = static_cast<int>(floor(cameraPosition.x / tileSize));
		int camTileZ = static_cast<int>(floor(cameraPosition.z / tileSize));

		int index = 0;
		for (int x = -1; x <= 1; ++x)
		{
			for (int z = -1; z <= 1; ++z)
			{
				groundTiles[index].position.x = (camTileX + x) * tileSize;
				groundTiles[index].position.z = (camTileZ + z) * tileSize;
				groundTiles[index].position.y = 0.0f;
				index++;
			}
		}
		// ACtually rendering the ground
		for (Ground& g : groundTiles)
		{
			g.render(vp, tileSize);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	skybox.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		cameraPosition = glm::vec3(150.0f, 50.0f, 150.0f);
		cameraLookVector = glm::vec3(0.0f, 0.0f, -1.0f);
	}

	// number is arbitrarily chosen, replace with what seems fitting
	// DIdnt think It would have to be one so large
	float cameraSpeed = static_cast<float>(10000 * deltaTime);

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