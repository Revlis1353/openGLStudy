#pragma once
class objectRegistry
{
private:
	std::vector<vertexObject*> staticObjects;
	std::vector<vertexObject*> dynamicObjects;

	std::vector<glm::vec3> dynamicVertex;

	std::vector<glm::vec2> dynamicUV;

	std::vector<glm::vec3> dynamicNormal;

	std::vector<glm::vec3> dynamicTangents;
	std::vector<glm::vec3> dynamicBitangents;

	GLuint staticTextureID;
	GLuint dynamicTextureID;

	GLuint staticTextureLocation;
	GLuint dynamicTextureLocation;

	GLuint staticNormalID;
	GLuint dynamicNormalID;

	GLuint staticNormalLocation;
	GLuint dynamicNormalLocation;

	GLuint staticVertexbuffer;
	GLuint dynamicVertexbuffer;

	GLuint staticUVbuffer;
	GLuint dynamicUVbuffer;

	GLuint staticNormalbuffer;
	GLuint dynamicNormalbuffer;

	GLuint dynamicTangentbuffer;
	GLuint dynamicBitangentbuffer;
public:
	objectRegistry(GLuint ProgramID);
	~objectRegistry();
	void registerStaticObject(vertexObject* vo);
	void registerDynamicObject(vertexObject* vo);
	void drawStaticObjects();
	void drawDynamicObjects(float deltaTime);
	void doGravity();
	GLuint getDynamicVertex();
	int getDynamicVerticesSize();
	GLuint loadTexture(std::string filenameString, GLenum minificationFilter = GL_LINEAR, GLenum magnificationFilter = GL_LINEAR);
};

