#define GLEW_STATIC
#include "glew.h"		// include GL Extension Wrangler

#include "glfw3.h"  // include GLFW helper library

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"
#include "gtx/rotate_vector.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cctype>
#include "main.h"

using namespace glm;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f
#define INPUT_FILE "input_a1.txt"

GLFWwindow* window = 0x00;

GLuint shader_program = 0;

//Default polygon drawing mode
GLint polygonMode = GL_LINE;

GLuint view_matrix_id = 0;
GLuint model_matrix_id = 0;
GLuint proj_matrix_id = 0;

//Stores vertices
std::vector<vec4> vertices;

//Stores indices for triangle vertices
std::vector<GLuint> vertexIndices;

//Stores state information for keyboard keys
bool keys[1024];

//Delta time
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//Last mouse coordinates
GLfloat lastX;
GLfloat lastY;
GLfloat y_offset;

//Default camera position
vec3 cameraPosition = vec3(0.0, 0.0, 1.0);

///Transformations
 mat4 proj_matrix= mat4(1.0);
 mat4 view_matrix = mat4(1.0);
 mat4 model_matrix = mat4(1.0);

mat4 rotation_matrix;
mat4 scale_matrix;
mat4 translation_matrix;

GLuint VBO, VAO, EBO;

GLfloat point_size = 3.0f;


// Calculate triangle indices using the total number of vertices and the number of points in the profile curve
std::vector<GLuint> generateIndices(GLint numVertices, GLint numProfilePoints, int sweepType) {
	std::vector<GLuint> indices = std::vector<GLuint>();
	int numSweeps = (numVertices - 1) / numProfilePoints;
	for (GLint i = 0; i < numVertices - numProfilePoints; ++i) {
		if ((i + 1) % numProfilePoints != 0) {
			indices.push_back(i);
			indices.push_back(i + numProfilePoints + 1);
			indices.push_back(i + numProfilePoints);
			indices.push_back(i);
			indices.push_back(i + numProfilePoints + 1);
			indices.push_back(i + 1);
		}
	}
	// If doing a rotational sweep, connect the last profile curve with the first
	if (sweepType) {
		int j = numSweeps * numProfilePoints;
		for (int i = 0; i < numProfilePoints - 1; i++) {				
			indices.push_back(j + i);
			indices.push_back(i + 1);
			indices.push_back(i);
			indices.push_back(j + i);
			indices.push_back(i + 1);
			indices.push_back(j + i + 1);
		}
	}
	
	return indices;
}



void do_movement() {
	
	GLfloat rotateAngle = 5.0f * deltaTime;

	// Move the model
	if (keys[GLFW_KEY_LEFT]) {
		//Rotate positive Z
		rotation_matrix = rotate(rotation_matrix, rotateAngle, vec3(0.0, 0.0, 1.0));
	}
	if (keys[GLFW_KEY_RIGHT]) {
		//
		rotation_matrix = rotate(rotation_matrix, rotateAngle, vec3(0.0, 0.0, -1.0));
	}
	if (keys[GLFW_KEY_UP]) {
		rotation_matrix = rotate(rotation_matrix, rotateAngle, vec3(1.0, 0.0, 0.0));
	}
	if (keys[GLFW_KEY_DOWN]) {
		rotation_matrix = rotate(rotation_matrix, rotateAngle, vec3(-1.0, 0.0, 0.0));
	}

	
}

// Called to cycle through drawing modes
GLint getPolygonMode() {
	if (polygonMode == GL_FILL) {
		polygonMode = GL_LINE;
		return polygonMode;
	}
	else if (polygonMode == GL_LINE) {
		polygonMode = GL_POINT;
		return polygonMode;
	}
	else {
		polygonMode = GL_FILL;
		return polygonMode;
	}
}

//Initialize Model View and Projection matrices
void setupMVP() {

	view_matrix = lookAt(
		cameraPosition,
		vec3(0.0f, 0.0f, -100.0f),
		vec3(0.0, 1.0, 0.0));

	proj_matrix = perspective(
		90.0f,
		1.0f / 1.0f,
		0.1f,
		100.0f);

	model_matrix = translation_matrix * rotation_matrix * scale_matrix;
}

///Handle the keyboard input
void keyPressed(GLFWwindow *_window, int key, int scancode, int action, int mods) {
	
	// Control the state of keys pressed
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;

	switch (key) {
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(_window, GL_TRUE); break;
		
		// Change between points, lines and fill
		case GLFW_KEY_M: glPolygonMode(GL_FRONT_AND_BACK, getPolygonMode()); 
		
		//Reset camera
		case GLFW_KEY_R: cameraPosition = vec3(0.0, 0.0, 1.0);
					   	 view_matrix = lookAt(
							cameraPosition,
							vec3(0.0f, 0.0f, -100.0f),
							vec3(0.0, 1.0, 0.0)); 
						 break;
		default: break;
	}
	
	return;
}

//Handle mouse movement
void mouseMoved(GLFWwindow *_window, double xpos, double ypos) {
	//If left mouse button is pressed move the camera along the Z axis based on the mouse's movement in the Y-axis
	if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_1)) {
		y_offset = 0.005f * (lastY - ypos);
		lastY = ypos;
		view_matrix = lookAt(
			cameraPosition + vec3(0.0f, 0.0f, y_offset),
			vec3(0.0f, 0.0f, -100.0f),
			vec3(0.0, 1.0, 0.0));
		cameraPosition = cameraPosition + vec3(0.0f, 0.0f, y_offset);
	}
	
	
}

//Handle the mouse buttons
void mouseClicked(GLFWwindow *_window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1) {
		double x, y;
		glfwGetCursorPos(_window, &x, &y);
		lastX = x;
		lastY = y;
	}
}

//Handle window resize. Resize is allowed and the viewport is updated to match the new window dimensions
void windowResized(GLFWwindow *_window, int width, int height) {
	glViewport(0, 0, width, height);
	proj_matrix = perspective(
		90.0f,
		1.0f / 1.0f,
		0.1f,
		100.0f);
}

bool initialize() {
	/// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/// Create a window of size 640x480 and with title "Lecture 2: First Triangle"
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	window = glfwCreateWindow(800, 800, "COMP371: Assignment 1", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	glViewport(0, 0, w, h);
	///Register the keyboard callback function: keyPressed(...)
	glfwSetKeyCallback(window, keyPressed);
	glfwSetCursorPosCallback(window, mouseMoved);
	glfwSetMouseButtonCallback(window, mouseClicked);
	glfwSetWindowSizeCallback(window, windowResized);

	

	/// Initialize GLEW extension handler
	glewExperimental = GL_TRUE;	///Needed to get the latest version of OpenGL
	glewInit();

	/// Get the current OpenGL version
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	/// Enable the depth test i.e. draw a pixel if it's closer to the viewer
	glEnable(GL_DEPTH_TEST); /// Enable depth-testing
	glDepthFunc(GL_LESS);	/// The type of testing i.e. a smaller value as "closer"

	return true;
}

bool cleanUp() {
	glDisableVertexAttribArray(0);
	//Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// Close GL context and any other GLFW resources
	glfwTerminate();

	return true;
}

GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);

	glBindAttribLocation(ProgramID, 0, "in_Position");

	//appearing in the vertex shader.
	glBindAttribLocation(ProgramID, 1, "in_Color");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	//The three variables below hold the id of each of the variables in the shader
	//If you read the vertex shader file you'll see that the same variable names are used.
	view_matrix_id = glGetUniformLocation(ProgramID, "view_matrix");
	model_matrix_id = glGetUniformLocation(ProgramID, "model_matrix");
	proj_matrix_id = glGetUniformLocation(ProgramID, "proj_matrix");

	return ProgramID;
}

void calculateVertices(std::string input) {
	
	std::ifstream inputData(input);
	GLint sweepType, numSpans, numProfilePoints;
	GLfloat data, x, y, z, x2, y2, z2;
	vertices = std::vector< vec4>();
	std::vector< vec3> trajectoryVectors = std::vector< vec3>();
	
	if (inputData.is_open())
	{
		inputData >> sweepType;

		// For Translational Sweep
		if (sweepType == 0)
		{
			inputData >> numProfilePoints;


			// Input the profile points into vertices vector
			for (int i = 0; i < numProfilePoints; i++)
			{
				inputData >> x >> y >> z;
				vertices.push_back(vec4(x, y, z, 1.0f));
			}

			// Calculate the translation vectors
			int numTranslationPoints;
			inputData >> numTranslationPoints;
			std::vector< vec3> tempPoints = std::vector< vec3>();
			inputData >> x >> y >> z;
			for (int i = 1; i < numTranslationPoints; ++i)
			{
				inputData >> x2 >> y2 >> z2;
				trajectoryVectors.push_back(vec3(x2 - x, y2 - y, z2 - z));
				x = x2;
				y = y2;
				z = z2;
			}



			// Perform translational sweep of vertices using trajectorVectors
			mat4 translation;
			for (int i = 0; i < trajectoryVectors.size(); ++i)
			{

				mat4 translation = translate(mat4(1.0), trajectoryVectors[i]);
				std::cout << "translation column 4 before second for loop: " << translation[3][0] << " " << translation[3][1] << " " << translation[3][2] << " " << translation[3][3] << std::endl;
				for (int j = 0; j < numProfilePoints; j++)
				{

					vertices.push_back(translation * vertices[j + i * numProfilePoints]);

				}

			}
		}
		else {
			// Rotational sweep
			inputData >> numSpans >> numProfilePoints;

			GLfloat degreesPerSpan = 360.0 / (numSpans + 1);
			GLfloat rads = DEG_TO_RAD * degreesPerSpan;
			std::cout << std::endl << "numSpans = " << numSpans << std::endl;
			std::cout << "numProfilePoints = " << numProfilePoints << std::endl;
			std::cout << "degreesPerSpan = " << degreesPerSpan << std::endl;
			mat4 rotateSpan = rotate(mat4(1.0f), degreesPerSpan, vec3(0.0, 0.0, 1.0));

			// Input the profile points into vertices vector
			for (int i = 0; i < numProfilePoints; i++)
			{
				inputData >> x >> y >> z;
				vertices.push_back(vec4(x, y, z, 1.0f));
			}

			for (int i = 0; i < numSpans; ++i) {
				for (int j = 0; j < numProfilePoints; ++j) {
					vertices.push_back(rotateY(vertices[j + i * numProfilePoints], rads));
				}
			}
		}
	}
	// Generate the vertex index array to draw triangles in proper order
	vertexIndices = generateIndices(vertices.size(), numProfilePoints, sweepType);
}



int main() {

	calculateVertices(INPUT_FILE);	

	setupMVP();

	initialize();

	///Load the shaders
	shader_program = loadShaders("COMP371_hw1.vs", "COMP371_hw1.fs");

	// This will identify our vertex buffer
	GLuint vertexbuffer;

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint EBO;
	glGenBuffers(1, &EBO);	

	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof( vec4), &vertices.front(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(GLint), &vertexIndices.front(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		4,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
		);

	glBindVertexArray(0);


	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	while (!glfwWindowShouldClose(window)) {
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
		glPointSize(point_size);

		glUseProgram(shader_program);

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Compute the MVP matrices
		do_movement();
		
		//rotation_matrix = rotateZ() * rotateX();
		model_matrix = translation_matrix * rotation_matrix * scale_matrix;

		//Pass the values of the three matrices to the shaders
		glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE,  value_ptr(proj_matrix));
		glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE,  value_ptr(view_matrix));
		glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE,  value_ptr(model_matrix));

		glBindVertexArray(VAO);
		// Draw the triangle !
	//	glDrawArrays(GL_LINE_STRIP, 0, 7); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDrawElements(GL_TRIANGLES, vertexIndices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// update other events like input handling
		glfwPollEvents();
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	cleanUp();
	return 0;
}