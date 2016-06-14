#ifndef SHADERS_HPP
#define SHADERS_HPP

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"

GLuint InitShader(const char* vertexSrc, const char* fragmentSrc)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSrc, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
	glCompileShader(fragmentShader);

	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cout<<"VERTEX SHADER ERROR"<<std::endl;
		char buffer[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
		std::cout<<buffer;
	}

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cout<<"FRAGMENT SHADER ERROR"<<std::endl;
		char buffer[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
		std::cout<<buffer;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");

	glLinkProgram(shaderProgram);

	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);

	return shaderProgram;
}

const char* modelVertexSrc =
	"#version 330 core\n"
	"in vec3 position;"
	"in vec2 uv;"
	"in vec3 normal;"
	"out vec2 UV;"
	"out vec3 Color;"
	"out vec3 Normal;"
	"uniform vec3 color;"
	"uniform mat4 model;"
	"uniform mat4 view;"
	"uniform mat4 proj;"
	"void main() {"
	"	UV = uv;"
	"	Normal = normalize(normal * transpose(mat3(model)));"
	"       vec3 ld = normalize((vec4(-1.0, -1.0, 1.0, 1.0)).xyz);"
	"       Color = vec3(0.5, 0.7, 0.8) + color + vec3(1.0, 0.9, 0.5) * max(dot(Normal, ld), 0.0);"
	"   gl_Position = proj * view * model * vec4(position, 1.0);"
	"}";
	
const char* modelFragmentSrc =
	"#version 330 core\n"
	"in vec3 Color;"
	"in vec2 UV;"
	"in vec3 Normal;"
	"uniform sampler2D TEXTURE;"
	"out vec4 outColor;"
	"void main() {"
	"	vec4 texel = texture(TEXTURE, UV) * vec4(Color, 1.0);"
	//"	texel = vec4(Color * 0.5f, 1.0);"
	"	if(texel.a < 0.4)"
	"		discard;"
		"outColor = texel * vec4(0.8, 0.9, 1.0, 1.0);"
	"}";

GLuint InitModelShader()
{
	return InitShader(modelVertexSrc, modelFragmentSrc);
}

const char* particleVertexSrc =
	"#version 330 core\n"
	"in vec3 position;"
	"in vec2 uv;"
	"in mat4 model;"
	"in float alpha;"
	"in float angle;"
	"uniform mat4 view;"
	"uniform mat4 proj;"
	"out vec2 UV;"
	"out float Alpha;"
	"void main() {"
	"	UV = uv;"
	"	Alpha = alpha;"
	"	"
	"	mat4 MV = view * model;"
	"	"
	"	MV[0][0] = cos(angle); "
	"	MV[0][1] = sin(angle); "
	"	MV[0][2] = 0.0; "
	"	"
	"	MV[1][0] = -sin(angle); "
	"	MV[1][1] = cos(angle); "
	"	MV[1][2] = 0.0; "
	"	"
	"	MV[2][0] = 0.0; "
	"	MV[2][1] = 0.0; "
	"	MV[2][2] = 1.0; "
	"	"
	"	gl_Position = proj * MV * vec4(position - vec3(0.5, 0.5, 0.0), 1.0);"
	"}";
	
const char* particleFragmentSrc =
	"#version 330 core\n"
	"in vec2 UV;"
	"in float Alpha;"
	"uniform sampler2D TEXTURE;"
	"out vec4 outColor;"
	"void main() {"
	"	vec4 texel = texture(TEXTURE, UV) * vec4(1.0f, 1.0f, 1.0f, Alpha);"
	"	if(texel.a < 0.2)"
	"		discard;"
		"outColor = texel * vec4(0.8, 0.9, 1.0, 1.0);"
	"}";

GLuint InitParticleShader()
{
	return InitShader(particleVertexSrc, particleFragmentSrc);
}

#endif // SHADERS_HPP