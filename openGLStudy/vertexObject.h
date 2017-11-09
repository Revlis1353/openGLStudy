#pragma once
class vertexObject
{
private:
	std::vector<glm::vec3> vertexData;
	std::vector<glm::vec2> UVData;
	std::vector<glm::vec3> normals;
	glm::vec3 position;
	glm::vec3 scale;
	char * texturePath;
	glm::vec3 velocity;
	float mass;
	glm::vec3 acceleration;

public:
	vertexObject(const char * objectPath, float Mass);
	vertexObject(const char * objectPath, float Mass, glm::vec3 Position, glm::vec3 Scale);
	~vertexObject();
	std::vector<glm::vec3> doPhysicCalc(float deltaTime);
	std::vector<glm::vec2> getUVData();
	std::vector<glm::vec3> getVertexData();
	std::vector<glm::vec3> getNormalData();
	void applyForce(glm::vec3 force);
	glm::vec3 getPos();
	float getMass();
};