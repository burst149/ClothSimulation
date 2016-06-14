#include "Cloth.hpp"

Link::Link(Particle* a, Particle* b) : _a(a), _b(b)
{
	vec3 va, vb;
	_a->GetPosition(va);
	_b->GetPosition(vb);

	vec3 v;
	vec3_sub(v, va, vb);
	_dist = vec3_len(v);
}

void Link::Update()
{
	vec3 va, vb;
	_a->GetPosition(va);
	_b->GetPosition(vb);

	vec3 v;
	vec3_sub(v, vb, va);
	float dist = vec3_len(v);

	if (dist == 0)
		dist = 0.0001f;

	vec3 corr;
	vec3_scale(corr, v, 1.0f - (_dist/dist));
	vec3_scale(corr, corr, 0.5f);
	_a->Offset(corr);
	vec3_scale(corr, corr, -1.0f);
	_b->Offset(corr);
}

Cloth::Cloth(float w, float h, unsigned int nX, unsigned int nY, GLuint shader, GLuint texture) : _nX(nX), _nY(nY), _free(true)
{
	_color[0] = 0.0f;
	_color[1] = 0.0f;
	_color[2] = 0.0f;
	
	_pos[0] = 0.0f;
	_pos[1] = 0.0f;
	_pos[2] = 0.0f;

	_shader = shader;
	_texture = texture;

	_polycount = 2 * 2 * (_nX - 1) * (_nY - 1);

	_nodes.clear();
	_links.clear();

	vec3 pos;
	for (unsigned int y = 0; y < _nY; ++y)
		for (unsigned int x = 0; x < _nX; ++x)
		{
			pos[0] = 0;
			pos[1] =  w * (x / (float)_nX);
			pos[2] = -h * (y / (float)_nY);
			_nodes.push_back(Particle());
			_nodes[_nodes.size() - 1].SetPosition(pos);
			_nodes[_nodes.size() - 1].SetMass(w*h/(nX*nY));
		}

	for (unsigned int x = 0; x < _nX; ++x)
		for (unsigned int y = 0; y < _nY; ++y)
		{
			if (x < _nX - 1)
				MakeLink(GetParticle(x, y), GetParticle(x + 1, y));
			if (y < _nY - 1)
				MakeLink(GetParticle(x, y), GetParticle(x, y + 1));
			if (x < _nX - 1 && y < _nY - 1)
			{
				MakeLink(GetParticle(x, y), GetParticle(x + 1, y + 1));
				MakeLink(GetParticle(x + 1, y), GetParticle(x, y + 1));
			}
		}

	for (unsigned int x = 0; x < _nX; ++x)
		for (unsigned int y = 0; y < _nY; ++y)
		{
			if (x < _nX - 2)
				MakeLink(GetParticle(x, y), GetParticle(x + 2, y));
			if (y < _nY - 2)
				MakeLink(GetParticle(x, y), GetParticle(x, y + 2));
			if (x < _nX - 2 && y < _nY - 2)
			{
				MakeLink(GetParticle(x, y), GetParticle(x + 2, y + 2));
				MakeLink(GetParticle(x + 2, y), GetParticle(x, y + 2));
			}
		}

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, (_polycount * 3 * 8) * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

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

	_vertices.resize(_polycount * 3 * 8);
}



Cloth::~Cloth()
{
	glDeleteBuffers(1, &_vbo);
	glDeleteVertexArrays(1, &_vao);
}

void Cloth::Update(float dt)
{
	for (unsigned int i = 0; i < 10; ++i)
		for (unsigned int j = 0; j < _links.size(); ++j)
			_links[j].Update();

	for (unsigned int i = 0; i < _nodes.size(); ++i)
		_nodes[i].Update(dt);

	if (_free)
	{
		vec3 off;
		vec3 origin;
		_nodes[0].GetPosition(origin);
		for (unsigned int i = 0; i < _nodes.size(); ++i)
		{
			_nodes[i].GetPosition(off);
			off[0] -= origin[0];
			off[1] -= origin[1];
			off[2] -= origin[2];
			_nodes[i].SetPosition(off);
		}

		_pos[0] += origin[0];
		_pos[1] += origin[1];
		_pos[2] += origin[2];
	}
}

void Cloth::Render()
{
	for (unsigned int i = 0; i < _nodes.size(); ++i)
		_nodes[i].ResetNormal();

	for (unsigned int x = 0; x < _nX - 1; ++x)
		for (unsigned int y = 0; y < _nY - 1; ++y)
		{
			vec3 normal;
			CalcNormal(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1), normal);
			GetParticle(x + 1, y)->AddNormal(normal);
			GetParticle(x, y)->AddNormal(normal);
			GetParticle(x, y + 1)->AddNormal(normal);

			CalcNormal(GetParticle(x + 1, y + 1), GetParticle(x + 1, y), GetParticle(x, y + 1), normal);
			GetParticle(x + 1, y + 1)->AddNormal(normal);
			GetParticle(x + 1, y)->AddNormal(normal);
			GetParticle(x, y + 1)->AddNormal(normal);
		}

	unsigned int n = 0;
	for (unsigned int x = 0; x < _nX - 1; ++x)
		for (unsigned int y = 0; y < _nY - 1; ++y)
		{
			UpdateVertex(n++, x + 1, y);
			UpdateVertex(n++,  x, y);
			UpdateVertex(n++, x, y + 1);

			UpdateVertex(n++, x + 1, y + 1);
			UpdateVertex(n++, x + 1, y);
			UpdateVertex(n++, x, y + 1);
		}

	for (unsigned int x = 0; x < _nX - 1; ++x)
		for (unsigned int y = 0; y < _nY - 1; ++y)
		{
			UpdateVertex(n++, x, y + 1);
			UpdateVertex(n++,  x, y);
			UpdateVertex(n++, x + 1, y);

			UpdateVertex(n++, x, y + 1);
			UpdateVertex(n++, x + 1, y);
			UpdateVertex(n++, x + 1, y + 1);
		}

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);

	glUseProgram(_shader);
	glBindTexture(GL_TEXTURE_2D, _texture);

	glBufferSubData(GL_ARRAY_BUFFER, 0, (_polycount * 3 * 8) * sizeof(GLfloat), &_vertices[0]);

	mat4x4 model;
	mat4x4_identity(model);

	mat4x4 trn;
	mat4x4_translate(trn, _pos[0], _pos[1], _pos[2]);
	mat4x4_mul(model, trn, model);

	glUniformMatrix4fv(glGetUniformLocation(_shader, "model"), 1, GL_FALSE, *model);
	glUniform3fv(glGetUniformLocation(_shader, "color"), 1, (GLfloat*)_color);

	glDrawArrays(GL_TRIANGLES, 0, _polycount * 3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void Cloth::AddForce(vec3 f)
{
	for (unsigned int i = 0; i < _nodes.size(); ++i)
		_nodes[i].AddForce(f);
}

void Cloth::AddGravity()
{
	for (unsigned int i = 0; i < _nodes.size(); ++i)
		_nodes[i].AddGravity();
}

void Cloth::AddWind(vec3 w)
{
	for (unsigned int x = 0; x < _nX - 1; ++x)
		for (unsigned int y = 0; y < _nY - 1; ++y)
		{
			vec3 normal, n, f;
			CalcNormal(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1), normal);
			vec3_norm(n, normal);
			vec3_scale(f, normal, vec3_mul_inner(n, w));
			GetParticle(x + 1, y)->AddForce(f);
			GetParticle(x, y)->AddForce(f);
			GetParticle(x, y + 1)->AddForce(f);

			CalcNormal(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1), normal);
			vec3_norm(n, normal);
			vec3_scale(f, normal, vec3_mul_inner(n, w));
			GetParticle(x + 1, y)->AddForce(f);
			GetParticle(x, y)->AddForce(f);
			GetParticle(x, y + 1)->AddForce(f);
		}
}

void Cloth::CollideSphere(vec3 center, float radius)
{
	for (unsigned int i = 0; i < _nodes.size(); ++i)
	{
		vec3 p, v;
		_nodes[i].GetPosition(p);
		vec3_add(p, p, _pos);
		vec3_sub(v, p, center);
		float d = vec3_len(v);
		if (d < radius)
		{
			vec3_norm(v, v);
			vec3_scale(v, v, radius - d);
			_nodes[i].Offset(v);
		}
	}
}

void Cloth::CollideBoundary(float size)
{
	vec3 v;
	vec3 p;

	for (unsigned int i = 0; i < _nodes.size(); ++i)
	{
		_nodes[i].GetPosition(p);
		vec3_add(p, p, _pos);

		v[0] = 0.0f; v[1] =  0.0f; v[2] =  0.0f;

		if (p[1] > size)
			v[1] = size - p[1];
		if (p[0] < -size)
			v[0] = -size - p[0];
		if (p[0] > size)
			v[0] = size - p[0];
		if (p[1] < -size)
			v[1] = -size - p[1];
		if (p[2] < 0.1f)
			v[2] = 0.1f - p[2];

		_nodes[i].Offset(v);

		if (p[2] < 0.1f && vec3_len(v) > 0)
		{
			_nodes[i].Freeze();
			_nodes[i].Unfreeze();
		}

	}
}

void Cloth::SetPosition(vec3 pos)
{
	vec3 off;
	if (!_free)
		for (unsigned int i = 0; i < _nodes.size(); ++i)
		{
			if (!_nodes[i].IsFrozen())
			{
				_nodes[i].GetPosition(off);
				off[0] -= pos[0] - _pos[0];
				off[1] -= pos[1] - _pos[1];
				off[2] -= pos[2] - _pos[2];
				_nodes[i].SetPosition(off);
			}
		}

	_pos[0] = pos[0];
	_pos[1] = pos[1];
	_pos[2] = pos[2];
}

void Cloth::GetPosition(vec3 pos)
{
	pos[0] = _pos[0];
	pos[1] = _pos[1];
	pos[2] = _pos[2];
}

void Cloth::Release()
{
	for (unsigned int i = 0; i < _nodes.size(); ++i)
		_nodes[i].Unfreeze();

	_free = true;
}

void Cloth::FreezeCorner(short corner)
{
	switch (corner)
	{
		case 0: _nodes[0].Freeze(); _free = false; break;
		case 1: _nodes[_nX - 1].Freeze(); _free = false; break;
		case 2: _nodes[_nodes.size() - _nX].Freeze(); _free = false; break;
		case 3: _nodes[_nodes.size() - 1].Freeze(); _free = false; break;
	};
}

bool Cloth::IsReleased()
{
	for (unsigned int i = 0; i < _nodes.size(); ++i)
		if (_nodes[i].IsFrozen())
			return false;

	return true;
}

Particle* Cloth::GetParticle(unsigned int x, unsigned int y)
{
	if (x >= 0 && x < _nX && y >= 0 && y < _nY)
		return &_nodes[y * _nX + x];
	else
		return 0;
}

void Cloth::MakeLink(Particle* a, Particle* b)
{
	_links.push_back(Link(a, b));
}

void Cloth::CalcNormal(Particle* a, Particle* b, Particle* c, vec3 n)
{
	vec3 va, vb, vc;
	a->GetPosition(va);
	b->GetPosition(vb);
	c->GetPosition(vc);

	vec3 vx, vy;
	vec3_sub(vx, vb, va);
	vec3_sub(vy, vc, va);

	vec3_mul_cross(n, vx, vy);
}

void Cloth::UpdateVertex(unsigned int n, unsigned int x, unsigned int y)
{
	vec3 pos;
	vec3 norm;

	GetParticle(x, y)->GetPosition(pos);
	GetParticle(x, y)->GetNormal(norm);

	if (n > _polycount * 3 * 0.5f - 1)
		vec3_scale(norm, norm, -1.0f);

	_vertices[n * 8 + 0] = pos[0] + x *0.0001f + y *0.0002f;
	_vertices[n * 8 + 1] = pos[1] + x *0.0001f + y *0.0002f;
	_vertices[n * 8 + 2] = pos[2] + x *0.0001f + y *0.0002f;
	_vertices[n * 8 + 3] = x / (float)(_nX - 1);
	_vertices[n * 8 + 4] = 1.0f - (y / (float)(_nY - 1));
	_vertices[n * 8 + 5] = norm[0];
	_vertices[n * 8 + 6] = norm[1];
	_vertices[n * 8 + 7] = norm[2];
}