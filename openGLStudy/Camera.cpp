#include "./include/glm/gtx/transform.hpp"
#include "./include/GLFW/glfw3.h"
#include <glm\glm.hpp>
#include <iostream>

#include "Camera.h"


Camera::Camera(int initWidth, int initHeight)
{
	Model = glm::mat4(1.0f);
	position = glm::vec3(0.0, 0.0, -6.0);
	speed = 3;
	mouseSpeed = 0.030f;
	FOV = 60;
	verticalAngle = 0;
	horizontalAngle = 0;

	View = glm::lookAt(
		position,
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
	);

	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	width = initWidth;
	height = initHeight;
	Projection = glm::perspective(glm::radians(FOV), (float)width / (float)height, 0.1f, 100.0f);
	refreshMVP();
}


Camera::~Camera()
{
}

glm::mat4 * Camera::getMVP() {
	return &MVP;
}

glm::mat4 * Camera::getView() {
	return &View;
}

glm::mat4 * Camera::getModel() {
	return &Model;
}

void Camera::refreshProjection() {
	Projection = glm::perspective(glm::radians(FOV), (float)width / (float)height, 0.1f, 100.0f);
	refreshMVP();
}

void Camera::refreshMVP() {
	MVP = Projection * View * Model;
}

void Camera::refreshView() {

	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
		);

	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
		);

	View = glm::lookAt(
		position,
		position + direction,
		glm::cross(right, direction)
		);
	refreshMVP();
}

void Camera::changeAngle(float deltaVertical, float deltaHorizontal) {
	verticalAngle += deltaVertical * mouseSpeed;
	horizontalAngle += deltaHorizontal * mouseSpeed;
	if (verticalAngle > 1.50) verticalAngle = 1.50;
	else if (verticalAngle < -1.50) verticalAngle = -1.50;
	refreshView();
}

void Camera::changePosition(glm::vec3 deltaPosition) {
	position += deltaPosition * speed;
	refreshView();
}

glm::vec3 Camera::getPosition() {
	return position;
}
glm::vec3 Camera::getRightVector() {
	return right;
}
glm::vec3 Camera::getDirection() {
	return direction;
}