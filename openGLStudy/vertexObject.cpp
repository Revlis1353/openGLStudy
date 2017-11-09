#include <vector>
#include <glm\glm.hpp>
#include <iostream>

#include "objloader.h"
#include "vertexObject.h"

vertexObject::vertexObject(const char * objectPath, float Mass) {
	if (!loadOBJ(objectPath, vertexData, UVData, normals)) {
		std::cout << "Fail to load object file: "  << objectPath << std::endl;
	}
	mass = Mass;
	position = glm::vec3(0.0f);
	scale = glm::vec3(1.0f);
}

vertexObject::vertexObject(const char * objectPath, float Mass, glm::vec3 Position, glm::vec3 Scale) : vertexObject(objectPath, Mass) {
	position = Position;
	scale = Scale;
}

vertexObject::~vertexObject()
{
}

std::vector<glm::vec3> vertexObject::doPhysicCalc(float deltaTime) {
	int i;
	std::vector<glm::vec3> resultData;
	for (i = 0; i < vertexData.size(); i++) {
		velocity += deltaTime * acceleration;
		position += deltaTime * velocity;
		resultData.push_back(position + scale * vertexData[i]);
	}
	acceleration = glm::vec3(0.0f);
	return resultData;
}

std::vector<glm::vec2> vertexObject::getUVData() {
	return UVData;
}

std::vector<glm::vec3> vertexObject::getVertexData() {
	return vertexData;
}

std::vector<glm::vec3> vertexObject::getNormalData() {
	return normals;
}

void vertexObject::applyForce(glm::vec3 force) {
	acceleration += force / mass;
}

glm::vec3 vertexObject::getPos() {
	return position;
}

float vertexObject::getMass() {
	return mass;
}