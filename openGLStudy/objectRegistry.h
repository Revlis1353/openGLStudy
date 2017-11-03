#pragma once
class objectRegistry
{
private:
	std::vector<vertexObject*> staticObjects;
	std::vector<vertexObject*> dynamicObjects;

	std::vector<glm::vec2> dynamicUV;

	GLuint staticTextureID;
	GLuint dynamicTextureID;

	GLuint staticTextureLocation;
	GLuint dynamicTextureLocation;

	GLuint staticVertexbuffer;
	GLuint dynamicVertexbuffer;

	GLuint staticUVbuffer;
	GLuint dynamicUVbuffer;
public:
	objectRegistry(GLuint ProgramID);
	~objectRegistry();
	void registerStaticObject(vertexObject* vo);
	void registerDynamicObject(vertexObject* vo);
	void drawStaticObjects();
	void drawDynamicObjects(float deltaTime);
	void doGravity();
	GLuint loadTexture(std::string filenameString, GLenum minificationFilter = GL_LINEAR, GLenum magnificationFilter = GL_LINEAR);
};

