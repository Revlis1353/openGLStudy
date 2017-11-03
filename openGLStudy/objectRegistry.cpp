#include <vector>
#include <glm\glm.hpp>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <iostream>
#include <FreeImage.h>
#include <string>

#include "vertexObject.h"
#include "objectRegistry.h"

objectRegistry::objectRegistry(GLuint ProgramID){
	staticObjects = std::vector<vertexObject*>();
	dynamicObjects = std::vector<vertexObject*>();
	dynamicUV = std::vector<glm::vec2>();
	
	FreeImage_Initialise(true);

	dynamicTextureID = loadTexture("dynamicTexture.png");
	dynamicTextureLocation = glGetUniformLocation(ProgramID, "dynamicTextureSampler");

	glGenBuffers(1, &dynamicVertexbuffer);
	glGenBuffers(1, &dynamicUVbuffer);
}


objectRegistry::~objectRegistry(){
	glDeleteTextures(1, &dynamicTextureID);
	glDeleteBuffers(1, &dynamicVertexbuffer);
	glDeleteBuffers(1, &dynamicUVbuffer);
	FreeImage_DeInitialise();
}

void objectRegistry::registerStaticObject(vertexObject* vo) {
	staticObjects.push_back(vo);
}

void objectRegistry::registerDynamicObject(vertexObject* vo) {
	dynamicObjects.push_back(vo);
	std::vector<glm::vec2> tempUV = vo->getUVData();
	int i;
	for (i = 0;i < tempUV.size(); i++) {
		dynamicUV.push_back(tempUV[i]);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, dynamicUVbuffer);
	glBufferData(GL_ARRAY_BUFFER, 2 * dynamicUV.size() * sizeof(float), dynamicUV.data(), GL_STATIC_DRAW);
}

void objectRegistry::drawStaticObjects() {
	//do something
}

void objectRegistry::drawDynamicObjects(float deltaTime) {
	int i;
	std::vector<glm::vec3> dynamicVertex;
	for (i = 0; i < dynamicObjects.size(); i++) {
		std::vector<glm::vec3> currentVertex = (*dynamicObjects[i]).doPhysicCalc(deltaTime);
		dynamicVertex.insert(dynamicVertex.end(), currentVertex.begin(), currentVertex.end());
	}

	glBindBuffer(GL_ARRAY_BUFFER, dynamicVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * dynamicVertex.size() * sizeof(float), dynamicVertex.data(), GL_DYNAMIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dynamicTextureID);

	glUniform1i(dynamicTextureLocation, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, dynamicVertexbuffer);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
		);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, dynamicUVbuffer);
	glVertexAttribPointer(
		3,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	glDrawArrays(GL_TRIANGLES, 0, 3 * dynamicVertex.size());
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

const double G = 6.67384 * pow(10, -11);

void objectRegistry::doGravity() {
	int i, j;
	for (i = 0; i < dynamicObjects.size(); i++) {
		for (j = i+1; j < dynamicObjects.size(); j++) {
			glm::vec3 direction = (*dynamicObjects[i]).getPos() - (*dynamicObjects[j]).getPos();
			float distance = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;
			double force = G * (*dynamicObjects[i]).getMass() * (*dynamicObjects[j]).getMass() / distance;
			glm::vec3 forceV = (float)force * direction / sqrt(distance);
			(*dynamicObjects[i]).applyForce(-forceV);
			(*dynamicObjects[j]).applyForce(forceV);
		}
	}
}

GLuint objectRegistry::loadTexture(std::string filenameString, GLenum minificationFilter, GLenum magnificationFilter) {
	const char* filename = filenameString.c_str();

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);

	if (format == -1)
	{
		std::cout << "Could not find image: " << filenameString << " - Aborting." << std::endl;
		exit(-1);
	}

	if (format == FIF_UNKNOWN)
	{
		std::cout << "Couldn't determine file format - attempting to get from file extension..." << std::endl;

		format = FreeImage_GetFIFFromFilename(filename);

		if (!FreeImage_FIFSupportsReading(format))
		{
			std::cout << "Detected image format cannot be read!" << std::endl;
			exit(-1);
		}
	}

	FIBITMAP* bitmap = FreeImage_Load(format, filename);

	int bitsPerPixel = FreeImage_GetBPP(bitmap);

	FIBITMAP* bitmap32;
	if (bitsPerPixel == 32)
	{
		std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Skipping conversion." << std::endl;
		bitmap32 = bitmap;
	}
	else
	{
		std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Converting to 32-bit colour." << std::endl;
		bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
	}

	int imageWidth = FreeImage_GetWidth(bitmap32);
	int imageHeight = FreeImage_GetHeight(bitmap32);
	std::cout << "Image: " << filenameString << " is size: " << imageWidth << "x" << imageHeight << "." << std::endl;

	GLubyte* textureData = FreeImage_GetBits(bitmap32);

	// Generate a texture ID and bind to it
	GLuint tempTextureID;
	glGenTextures(1, &tempTextureID);
	glBindTexture(GL_TEXTURE_2D, tempTextureID);

	glTexImage2D(GL_TEXTURE_2D,
		0,                // Mipmap level (0 being the top level i.e. full size)
		GL_RGBA,          // Internal format
		imageWidth,       // Width of the texture
		imageHeight,      // Height of the texture,
		0,                // Border in pixels
		GL_BGRA,          // Data format
		GL_UNSIGNED_BYTE, // Type of texture data
		textureData);     // The image data to use for this texture

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minificationFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnificationFilter);

	if (minificationFilter == GL_LINEAR_MIPMAP_LINEAR ||
		minificationFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minificationFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minificationFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	GLenum glError = glGetError();
	if (glError)
	{
		std::cout << "There was an error loading the texture: " << filenameString << std::endl;

		switch (glError)
		{
		case GL_INVALID_ENUM:
			std::cout << "Invalid enum." << std::endl;
			break;

		case GL_INVALID_VALUE:
			std::cout << "Invalid value." << std::endl;
			break;

		case GL_INVALID_OPERATION:
			std::cout << "Invalid operation." << std::endl;

		default:
			std::cout << "Unrecognised GLenum." << std::endl;
			break;
		}

	}

	FreeImage_Unload(bitmap32);

	if (bitsPerPixel != 32)
	{
		FreeImage_Unload(bitmap);
	}

	return tempTextureID;
}