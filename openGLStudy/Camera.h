#pragma once

class Camera
{
private:
	glm::mat4 MVP;
	glm::mat4 Model;
	glm::mat4 View;
	glm::mat4 Projection;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 right;

	float verticalAngle;
	float horizontalAngle;
	float FOV;
	float speed;
	float mouseSpeed;
	int width, height;
public:
	Camera(int width, int height);
	~Camera();
	glm::mat4 * getMVP();
	void refreshProjection();
	void refreshView();
	void Camera::refreshMVP();
	void changeAngle(float deltaVertical, float deltaHorizontal);
	void Camera::changePosition(glm::vec3);
	glm::vec3 getPosition();
	glm::vec3 getRightVector();
	glm::vec3 getDirection();
};

