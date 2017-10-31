#include <vector>
#include <glm\glm.hpp>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <iostream>

#include "vertexObject.h"
#include "objectRegistry.h"

objectRegistry::objectRegistry(){
	staticObjects = std::vector<vertexObject*>();
	dynamicObjects = std::vector<vertexObject*>();
}


objectRegistry::~objectRegistry(){
}

void objectRegistry::registerStaticObject(vertexObject* vo) {
	staticObjects.push_back(vo);
}

void objectRegistry::registerDynamicObject(vertexObject* vo) {
	dynamicObjects.push_back(vo);
}

void objectRegistry::drawStaticObjects() {
	//do something
}

void objectRegistry::drawDynamicObjects(float deltaTime, GLuint vertexbuffer) {
	int i;
	std::vector<glm::vec3> dynamicVertex;
	for (i = 0; i < dynamicObjects.size(); i++) {
		std::vector<glm::vec3> currentVertex = (*dynamicObjects[i]).doPhysicCalc(deltaTime);
		dynamicVertex.insert(dynamicVertex.end(), currentVertex.begin(), currentVertex.end());
	}

	//I will use atlas texture in here
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * dynamicVertex.size() * sizeof(float), dynamicVertex.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
		);

	glDrawArrays(GL_TRIANGLES, 0, 3 * dynamicVertex.size());
	glDisableVertexAttribArray(1);
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