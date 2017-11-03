const char * vertexShader1 = "							\
#version 430 core										\n\r\
layout(location = 2) in vec3 vertexPosition_modelspace;	\
layout(location = 3) in vec2 vertexUV;					\
														\
out vec2 UV;											\
														\
uniform mat4 MVP;										\
														\
void main(){											\
	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);	\
	UV = vertexUV;										\
}";