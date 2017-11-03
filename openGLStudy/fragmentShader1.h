const char * fragmentShader1 = "\
#version 430 core				\n\r\
in vec2 UV;						\
out vec3 color;					\
\
uniform sampler2D dynamicTextureSampler;\
\
void main(){					\
\
	color = texture(dynamicTextureSampler, UV).rgb;\
}";