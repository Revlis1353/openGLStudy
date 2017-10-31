#pragma once
class objectRegistry
{
private:
	std::vector<vertexObject*> staticObjects;
	std::vector<vertexObject*> dynamicObjects;
public:
	objectRegistry();
	~objectRegistry();
	void registerStaticObject(vertexObject* vo);
	void registerDynamicObject(vertexObject* vo);
	void drawStaticObjects();
	void drawDynamicObjects(float deltaTime, GLuint vertexbuffer);
	void doGravity();
};

