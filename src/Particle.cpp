#include "Particle.hpp"

Particle::Particle() : _frozen(false), _mass(1)
{
	_pos[0] = 0;
	_pos[1] = 0;
	_pos[2] = 0;

	_prev[0] = 0;
	_prev[1] = 0;
	_prev[2] = 0;

	_acc[0] = 0;
	_acc[1] = 0;
	_acc[2] = 0;

	_norm[0] = 0;
	_norm[1] = 0;
	_norm[2] = 0;
}

void Particle::SetMass(float mass)
{
	_mass = mass;
}

void Particle::AddForce(vec3 force)
{
	vec3 f = {force[0] / _mass, force[1] / _mass, force[2] / _mass};
	vec3_add(_acc, _acc, f);
}

void Particle::AddGravity()
{
	vec3 g = {0.0f, 0.0f, -9.0f};
	vec3_add(_acc, _acc, g);
}

void Particle::Update(float dt)
{
	if (!_frozen)
	{
		vec3 temp = {0.0f, 0.0f, 0.0f};
		vec3_add(temp, temp, _pos);

		vec3 move = {0.0f, 0.0f, 0.0f};
		vec3_sub(move, _pos, _prev);
		vec3_scale(move, move, (1.0f - DAMPING));

		vec3_scale(_acc, _acc, dt*dt);

		vec3_add(move, move, _acc);

		vec3_add(_pos, _pos, move);

		vec3_scale(_prev, _prev, 0);
		vec3_add(_prev, _prev, temp);

		vec3_scale(_acc, _acc, 0);
	}
}

void Particle::SetPosition(vec3 pos)
{
	_prev[0] += pos[0] - _pos[0];
	_prev[1] += pos[1] - _pos[1];
	_prev[2] += pos[2] - _pos[2];

	_pos[0] = pos[0];
	_pos[1] = pos[1];
	_pos[2] = pos[2];
}

void Particle::GetPosition(vec3 pos)
{
	pos[0] = _pos[0];
	pos[1] = _pos[1];
	pos[2] = _pos[2];
}

void Particle::Offset(vec3 v)
{
	if (!_frozen)
		vec3_add(_pos, _pos, v);
}

void Particle::Freeze()
{
	vec3_scale(_acc, _acc, 0);

	_prev[0] = _pos[0];
	_prev[1] = _pos[1];
	_prev[2] = _pos[2];

	_frozen = true;
}

void Particle::Unfreeze()
{
	_frozen = false;
}

bool Particle::IsFrozen()
{
	return _frozen;
}

void Particle::AddNormal(vec3 n)
{
	vec3_norm(n, n);
	vec3_add(_norm, _norm, n);
}

void Particle::GetNormal(vec3 n)
{
	vec3_scale(n, n, 0);
	vec3_add(n, n, _norm);
	vec3_norm(n, n);
}

void Particle::ResetNormal()
{
	vec3_scale(_norm, _norm, 0);
}
