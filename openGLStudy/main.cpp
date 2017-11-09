#ifdef __linux__
#include <GL/glew.h>		
#include <GLFW/glfw3.h> 
#include <stdio.h>

#elif _WIN32
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include "Camera.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "fragmentShader1.h"
#include "vertexShader1.h"
#include "vertexObject.h"
#include "objectRegistry.h"

#pragma comment(lib, "./lib/glew32.lib")
#pragma comment(lib, "./lib/glfw3.lib")
#pragma comment(lib, "OpenGL32.lib")

#else
#error Platform not supported
#endif

using namespace std;

void keyInput(Camera &camera, GLFWwindow *window, double deltaTime);
void mouseInput(Camera &camera, GLFWwindow *window, double deltaTime);
void checkFrame(int *nbFrames, double * lastTime, double * currentTime, double * deltaTime);
void setWindowSizeCallback(GLFWwindow* window, int width, int height);
void setKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void checkMatrix(glm::mat4 checkmat);

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
GLuint LoadShadersFromChar(const char * vertex_char, const char * fragment_char);

int main() {
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window;
	window = glfwCreateWindow(1024, 768, "OpenGL", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); //V-Sync enable

	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetWindowSizeCallback(window, setWindowSizeCallback);
	glfwSetKeyCallback(window, setKeyCallback);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	Camera camera(width, height);

	glm::mat4 * mvp = camera.getMVP();

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//GLuint programID = LoadShadersFromChar(vertexShader1, fragmentShader1);
	GLuint programID = LoadShaders("vertshad", "fragshad");
	GLuint DepthprogramID = LoadShaders("DepthVertex.txt", "DepthFragment.txt");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");

	//vertexObject VO1 = vertexObject("cube.obj", 1000, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f));
	vertexObject VO2 = vertexObject("room_thickwalls.obj", 1000, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));

	objectRegistry OR = objectRegistry(programID);
	//OR.registerDynamicObject(&VO1);
	OR.registerDynamicObject(&VO2);	


	/////////////////////////
	GLuint depthMatrixID = glGetUniformLocation(DepthprogramID, "depthMVP");
	GLuint depthBiasID = glGetUniformLocation(programID, "DepthBiasMVP");
	GLuint ShadowMapID = glGetUniformLocation(programID, "shadowMap");
	GLuint lightInvDirID = glGetUniformLocation(programID, "LightInvDirection_worldspace");
	GLuint MV3x3ID = glGetUniformLocation(programID, "MV3x3");
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	/////////////////////////

	//////////////////////////////////////////////////////
	GLuint FramebufferName = 1;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Depth texture. Slower than a depth buffer, but you can sample it later in your shader
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	glDrawBuffer(GL_NONE); // No color buffer is drawn to.

						   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	//////////////////////////////////////////////////////

	double deltaTime = 0, currentTime = glfwGetTime(), lastTime = glfwGetTime();
	int nbFrames = 0;

	do {

		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

		checkFrame(&nbFrames, &lastTime, &currentTime, &deltaTime);

		keyInput(camera, window, deltaTime);
		mouseInput(camera, window, deltaTime);

		

		//////////////////////////////////////////////////////
		
		glViewport(0, 0, 1024, 1024);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(DepthprogramID);

		glm::vec3 lightInvDir = glm::vec3(0.5f, 2, 2);

		glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10, 10, -10, 10, -10, 20);
		glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 depthModelMatrix = glm::mat4(1.0);
		glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &depthMVP[0][0]);
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, OR.getDynamicVertex());

		glVertexAttribPointer(
			0,  // The attribute we want to configure
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		glDrawArrays(GL_TRIANGLES, 0, 3 * OR.getDynamicVerticesSize());

		glDisableVertexAttribArray(0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		// Compute the MVP matrix from the light's point of view

		glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
			);
		glm::mat4 depthBiasMVP = biasMatrix*depthMVP;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(ShadowMapID, 0);

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(depthBiasID, 1, GL_FALSE, &depthBiasMVP[0][0]);
		//////////////////////////////////////////////////////

		glm::mat3 ModelView3x3 = glm::mat3(*camera.getView() * *camera.getModel());
		mvp = camera.getMVP();

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(*mvp)[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &(*camera.getView())[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &(*camera.getModel())[0][0]);
		glUniformMatrix4fv(MV3x3ID, 1, GL_FALSE, &ModelView3x3[0][0]);
		glUniform3f(lightInvDirID, lightInvDir.x, lightInvDir.y, lightInvDir.z);
		//checkMatrix(depthBiasMVP);

		OR.drawDynamicObjects((float)deltaTime);
		OR.doGravity();

		glfwSwapBuffers(window);
		glfwPollEvents();

	} 
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteFramebuffers(1, &FramebufferName);
	glDeleteTextures(1, &depthTexture);
	glDeleteProgram(programID);
	glDeleteProgram(DepthprogramID);

	glfwTerminate();

	//system("pause");
}

void checkMatrix(glm::mat4 checkmat) {
	int i, j;
	for (i = 0; i < 4; i++) {
		cout << checkmat[i][0] << " " << checkmat[i][1] << " " << checkmat[i][2] << " " << checkmat[i][3] << endl;
	}
	cout << endl;
}

void keyInput(Camera &camera, GLFWwindow *window, double deltaTime) {
	float FdeltaTime = (float)deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.changePosition(camera.getDirection() * FdeltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.changePosition(-camera.getRightVector() * FdeltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.changePosition(-camera.getDirection() * FdeltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.changePosition(camera.getRightVector() * FdeltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.changePosition(glm::vec3(0, 1, 0)* FdeltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.changePosition(glm::vec3(0, -1, 0)* FdeltaTime);
	}
}

void mouseInput(Camera &camera, GLFWwindow *window, double deltaTime) {
	float FdeltaTime = (float)deltaTime;
	double xpos, ypos;
	int width, height;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwGetWindowSize(window, &width, &height);
	camera.changeAngle((height / 2 - (float)ypos ) * FdeltaTime, (width / 2 - (float)xpos ) * FdeltaTime);
	glfwSetCursorPos(window, width / 2, height / 2);
}

void checkFrame(int *nbFrames, double * lastTime, double * currentTime, double * deltaTime) {
	double tempTime = glfwGetTime();
	*deltaTime = tempTime - *currentTime;
	*currentTime = tempTime;
	(*nbFrames)++;
	if (*currentTime - *lastTime >= 1.0) { 
		//printf("%f ms/frame\n", 1000.0 / double(*nbFrames));
		*nbFrames = 0;
		*lastTime = *currentTime;
	}
}

void setWindowSizeCallback(GLFWwindow* window, int width, int height) {
	cout << "width: " << width << ", height: " << height << endl;
	glViewport(0, 0, width, height);
}

void setKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

string readStringFromFile(const char * file_path) {

	std::ifstream f(file_path);

	if (f.is_open()) {
		 return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	}
	else {
		printf("Impossible to open %s.\n", file_path);
		getchar();
		return "";
	}
}

void compileShader(const char * file_path, string ShaderCode, GLuint ShaderID) {

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", file_path);
	char const * SourcePointer = ShaderCode.c_str();
	glShaderSource(ShaderID, 1, &SourcePointer, NULL);
	glCompileShader(ShaderID);

	// Check Vertex Shader
	glGetShaderiv(ShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ShaderID, InfoLogLength, NULL, &ShaderErrorMessage[0]);
		printf("%s\n", &ShaderErrorMessage[0]);
	}
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode = readStringFromFile(vertex_file_path);
	std::string FragmentShaderCode = readStringFromFile(fragment_file_path);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	compileShader(vertex_file_path, VertexShaderCode, VertexShaderID);
	compileShader(fragment_file_path, FragmentShaderCode, FragmentShaderID);

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint LoadShadersFromChar(const char * vertex_char, const char * fragment_char) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode = vertex_char;
	std::string FragmentShaderCode = fragment_char;

	GLint Result = GL_FALSE;
	int InfoLogLength;

	compileShader("vertexShader", VertexShaderCode, VertexShaderID);
	compileShader("fragmentShader", FragmentShaderCode, FragmentShaderID);

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}