#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

class Model
{
	public:
		Model(const std::string& filename, GLuint shader, GLuint texture);
		~Model();
		void Render();

		void SetColor(vec3 color);
		void GetColor(vec3 color);

		void SetPosition(vec3 pos);
		void GetPosition(vec3 pos);

		void SetOrientation(mat4x4 rot);
		void GetOrientation(mat4x4 rot);

		void SetScale(float scale);
		float GetScale();

	private:
		unsigned int _polycount;

		GLuint _vao;
		GLuint _vbo;
		GLuint _shader;

		GLuint _texture;

		vec3 _color;
		vec3 _pos;
		mat4x4 _rot;
		float _scale;
};

#endif // MODEL_HPP