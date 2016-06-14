#include "Model.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

Model::Model(const std::string& filename, GLuint shader, GLuint texture)
{
	// Initialization
	_color[0] = 0.0f;
	_color[1] = 0.0f;
	_color[2] = 0.0f;
	
	_pos[0] = 0.0f;
	_pos[1] = 0.0f;
	_pos[2] = 0.0f;

	mat4x4_identity(_rot);

	_scale = 1.0f;

	_shader = shader;
	_texture = texture;

	std::vector<GLfloat> v;
	std::vector<GLfloat> vt;
	std::vector<GLfloat> vn;
	std::vector<unsigned int> vi;
	std::vector<unsigned int> ti;
	std::vector<unsigned int> ni;

	_polycount = 0;

	// Reading OBJ model file
	std::ifstream file(filename.c_str());
	std::string str;
	while (file.good())
	{
		// Comment
		if (str[0] == '#')
		{
			getline(file, str);
			continue;
		}

		// Vertex
		if (str == "v")
		{
			GLfloat a;
			for (unsigned int i = 0; i < 3; ++i)
			{
				file>>a;
				v.push_back(a);
			}
		}

		// Texture
		if (str == "vt")
		{
			GLfloat a;
			for (unsigned int i = 0; i < 2; ++i)
			{
				file>>a;
				vt.push_back(a);
			}
		}

		//Normal
		if (str == "vn")
		{
			GLfloat a;
			for (unsigned int i = 0; i < 3; ++i)
			{
				file>>a;
				vn.push_back(a);
			}
		}

		// Triangle
		if (str == "f")
		{
			unsigned int index;
			std::string vertex;

			_polycount++;

			for (unsigned int i = 0; i < 3; ++i)
			{
				file>>vertex;

				std::istringstream vs(vertex.substr(0,vertex.find('/')));
				vs >> index;
				vi.push_back(index-1);

				vertex = vertex.substr(vertex.find('/')+1);
				std::istringstream ts(vertex.substr(0,vertex.find('/')));
				ts >> index;
				ti.push_back(index-1);

				vertex = vertex.substr(vertex.find('/')+1);
				std::istringstream ns(vertex.substr(0,vertex.find('/')));
				ns >> index;
				ni.push_back(index-1);
			}
		}

		file >> str;
	}
	file.close();

	std::vector<GLfloat> vertices;

	// Deindexing
	for (unsigned int i = 0; i < vi.size(); ++i)
	{
		vertices.push_back(v[vi[i]*3]);
		vertices.push_back(v[vi[i]*3+1]);
		vertices.push_back(v[vi[i]*3+2]);
		vertices.push_back(vt[ti[i]*2]);
		vertices.push_back(vt[ti[i]*2+1]);
		vertices.push_back(vn[ni[i]*3]);
		vertices.push_back(vn[ni[i]*3+1]);
		vertices.push_back(vn[ni[i]*3+2]);
	}

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	glUseProgram(shader);

	GLint posAttrib = glGetAttribLocation(_shader, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);

	GLint uvAttrib = glGetAttribLocation(_shader, "uv");
	glEnableVertexAttribArray(uvAttrib);
	glVertexAttribPointer(uvAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	GLint norAttrib = glGetAttribLocation(_shader, "normal");
	glEnableVertexAttribArray(norAttrib);
	glVertexAttribPointer(norAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

Model::~Model()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);
}

void Model::Render()
{
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glUseProgram(_shader);
	glBindTexture(GL_TEXTURE_2D, _texture);

	mat4x4 model;
	mat4x4_identity(model);
	
	// Scaling
	mat4x4_scale(model, model, _scale);
	model[3][3] = 1.0f;

	// Rotation
	mat4x4_mul(model, _rot, model);

	// Translation
	mat4x4 trn;
	mat4x4_translate(trn, _pos[0], _pos[1], _pos[2]);
	mat4x4_mul(model, trn, model);

	glUniformMatrix4fv(glGetUniformLocation(_shader, "model"), 1, GL_FALSE, *model);
	glUniform3fv(glGetUniformLocation(_shader, "color"), 1, (GLfloat*)_color);

	glDrawArrays(GL_TRIANGLES, 0, _polycount*3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Model::SetColor(vec3 color)
{
	_color[0] = color[0];
	_color[1] = color[1];
	_color[2] = color[2];
}

void Model::GetColor(vec3 color)
{
	color[0] = _color[0];
	color[1] = _color[1];
	color[2] = _color[2];
}

void Model::SetPosition(vec3 pos)
{
	_pos[0] = pos[0];
	_pos[1] = pos[1];
	_pos[2] = pos[2];
}

void Model::GetPosition(vec3 pos)
{
	pos[0] = _pos[0];
	pos[1] = _pos[1];
	pos[2] = _pos[2];
}

void Model::SetOrientation(mat4x4 rot)
{
	mat4x4_dup(_rot, rot);
}

void Model::GetOrientation(mat4x4 rot)
{
	mat4x4_dup(rot, _rot);
}

void Model::SetScale(float scale)
{
	_scale = scale;
}

float Model::GetScale()
{
	return _scale;
}