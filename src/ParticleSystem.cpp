#include "ParticleSystem.hpp"

ParticleSystem::ParticleSystem(GLuint shader, GLuint texture, float(*alpha_func)(float), float(*scale_func)(float))
{
	_shader = shader;
	_texture = texture;

	_alpha_func = alpha_func;
	_scale_func = scale_func;

	_lifetime = 1.0f;

	_pos[0] = 0.0f;
	_pos[1] = 0.0f;
	_pos[2] = 0.0f;

	_size[0] = 0.0f;
	_size[1] = 0.0f;
	_size[2] = 0.0f;

	_force[0] = 0.0f;
	_force[1] = 0.0f;
	_force[2] = 0.0f;

	_mass = 1;

	GLfloat vertices[] = {
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,

			0.0f, 1.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
		};

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glUseProgram(shader);

	glEnableVertexAttribArray(glGetAttribLocation(_shader, "position"));
	glEnableVertexAttribArray(glGetAttribLocation(_shader, "uv"));
	glEnableVertexAttribArray(glGetAttribLocation(_shader, "alpha"));
	glEnableVertexAttribArray(glGetAttribLocation(_shader, "angle"));
	
	GLuint matAtt = glGetAttribLocation(_shader, "model");
	glEnableVertexAttribArray(matAtt + 0);
	glEnableVertexAttribArray(matAtt + 1);
	glEnableVertexAttribArray(matAtt + 2);
	glEnableVertexAttribArray(matAtt + 3);

	glGenBuffers(1, &_quadBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _quadBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(_shader, "position"), 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribPointer(glGetAttribLocation(_shader, "uv"), 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

	glGenBuffers(1, &_matBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _matBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16 * LIMIT, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(matAtt + 0, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)0);
	glVertexAttribPointer(matAtt + 1, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(4 * sizeof(float)));
	glVertexAttribPointer(matAtt + 2, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(8 * sizeof(float)));
	glVertexAttribPointer(matAtt + 3, 4, GL_FLOAT, GL_FALSE, sizeof(float)*16, (void*)(12 * sizeof(float)));

	glVertexAttribDivisor(matAtt + 0, 1);
	glVertexAttribDivisor(matAtt + 1, 1);
	glVertexAttribDivisor(matAtt + 2, 1);
	glVertexAttribDivisor(matAtt + 3, 1);

	glGenBuffers(1, &_alphaBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _alphaBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * LIMIT, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(_shader, "alpha"), 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glVertexAttribDivisor(glGetAttribLocation(_shader, "alpha"), 1);

	glGenBuffers(1, &_rotBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _rotBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * LIMIT, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(_shader, "angle"), 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glVertexAttribDivisor(glGetAttribLocation(_shader, "angle"), 1);

	glBindVertexArray(0);
}

ParticleSystem::~ParticleSystem()
{
	glDeleteBuffers(1, &_quadBuffer);
	glDeleteVertexArrays(1, &_vao);
}

void ParticleSystem::AddParticles(unsigned int amount)
{
	if (_particles.size() > 2000)
		return;

	vec3 pos;
	for (unsigned int i = 0; i < amount; ++i)
	{
		pos[0] = ((double) rand() / (float)RAND_MAX - 0.5f) * 2 * _size[0];
		pos[1] = ((double) rand() / (float)RAND_MAX - 0.5f) * 2 * _size[0];
		pos[2] = ((double) rand() / (float)RAND_MAX - 0.5f) * 2 * _size[0];
		_particles.push_back(Particle());
		_particles[_particles.size() - 1].SetMass(_mass);
		_particles[_particles.size() - 1].SetPosition(pos);

		float dt = _lifetime * (double) rand() / (float)RAND_MAX;
		_particles[_particles.size() - 1].AddForce(_force);
		_particles[_particles.size() - 1].Update(dt);
		_times.push_back(dt);
		_alphas.push_back(0);
		_scales.push_back(0);
		_rotations.push_back(2 * 3.14 * (double) rand() / (float)RAND_MAX);
		for (unsigned int j = 0; j < 16; ++j)
			_matrices.push_back(0);
	}
}

void ParticleSystem::RemoveParticles(unsigned int amount)
{
	for (unsigned int i = 0; i < amount; ++i)
		if (_particles.size())
		{
			_particles.pop_back();
			_times.pop_back();
			_alphas.pop_back();
			_rotations.pop_back();
			_scales.pop_back();
			for (unsigned int j = 0; j < 16; ++j)
				_matrices.pop_back();
		}
}

void ParticleSystem::SetParticleMass(float mass)
{
	_mass = mass;
	for (unsigned int i = 0; i < _particles.size(); ++i)
		_particles[i].SetMass(mass);
}

void ParticleSystem::SetEmitterPosition(vec3 pos)
{
	vec3 off;
	for (unsigned int i = 0; i < _particles.size(); ++i)
	{
		_particles[i].GetPosition(off);
		off[0] -= pos[0] - _pos[0];
		off[1] -= pos[1] - _pos[1];
		off[2] -= pos[2] - _pos[2];
		_particles[i].SetPosition(off);
	}

	_pos[0] = pos[0];
	_pos[1] = pos[1];
	_pos[2] = pos[2];
}

void ParticleSystem::SetEmitterSize(vec3 size)
{
	_size[0] = size[0];
	_size[1] = size[1];
	_size[2] = size[2];
}

void ParticleSystem::SetLifetime(float time)
{
	_lifetime = time;
}


void ParticleSystem::AddForce(vec3 f)
{
	_force[0] += f[0];
	_force[1] += f[1];
	_force[2] += f[2];
}

void ParticleSystem::AddGravity()
{
	_force[2] -= 9.0f * _mass;
}

void ParticleSystem::Update(float dt)
{
	vec3 pos;
	for (unsigned int i = 0; i < _particles.size(); ++i)
	{
		_times[i] += dt;

		_particles[i].AddForce(_force);
		_particles[i].Update(dt);

		_particles[i].GetPosition(pos);

		if (_times[i] > _lifetime)
		{
			pos[0] = ((double) rand() / (float)RAND_MAX - 0.5f) * 2 * _size[0];
			pos[1] = ((double) rand() / (float)RAND_MAX - 0.5f) * 2 * _size[1];
			pos[2] = ((double) rand() / (float)RAND_MAX - 0.5f) * 2 * _size[2];
			_particles[i].SetPosition(pos);
			_particles[i].Freeze();
			_particles[i].Unfreeze();
			_times[i] -= _lifetime;
		}

		_alphas[i] = (*_alpha_func)(_times[i]/_lifetime);
		_scales[i] = (*_scale_func)(_times[i]/_lifetime);
	}

	vec3_scale(_force, _force, 0);
}

void ParticleSystem::Render()
{
	glBindVertexArray(_vao);

	glUseProgram(_shader);
	glBindTexture(GL_TEXTURE_2D, _texture);

	vec3 pos;
	for (unsigned int i = 0; i < _particles.size(); ++i)
	{
		_particles[i].GetPosition(pos);

		_matrices[i * 16 +  0] = 1/_scales[i];
		_matrices[i * 16 +  1] = 0;
		_matrices[i * 16 +  2] = 0;
		_matrices[i * 16 +  3] = 0;

		_matrices[i * 16 +  4] = 0;
		_matrices[i * 16 +  5] = 1/_scales[i];
		_matrices[i * 16 +  6] = 0;
		_matrices[i * 16 +  7] = 0;

		_matrices[i * 16 +  8] = 0;
		_matrices[i * 16 +  9] = 0;
		_matrices[i * 16 + 10] = 1/_scales[i];
		_matrices[i * 16 + 11] = 0;

		_matrices[i * 16 + 12] = (_pos[0] + pos[0])/_scales[i];
		_matrices[i * 16 + 13] = (_pos[1] + pos[1])/_scales[i];
		_matrices[i * 16 + 14] = (_pos[2] + pos[2])/_scales[i];
		_matrices[i * 16 + 15] = 1/_scales[i];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _matBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 16 * _particles.size(), &_matrices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, _alphaBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * _particles.size(), &_alphas[0]);

	glBindBuffer(GL_ARRAY_BUFFER, _rotBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * _particles.size(), &_rotations[0]);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, _particles.size());

	glBindVertexArray(0);
}