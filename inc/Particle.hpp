#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "linmath.h"

#define DAMPING 0.001

class Particle
{
	public:
		Particle();

		void SetMass(float mass);
		void AddForce(vec3 force);
		void AddGravity();
		void Update(float dt);

		void SetPosition(vec3 pos);
		void GetPosition(vec3 pos);
		void Offset(vec3 v);

		void Freeze();
		void Unfreeze();
		bool IsFrozen();

		void AddNormal(vec3 n);
		void GetNormal(vec3 n);
		void ResetNormal();

	private:
		bool _frozen;
		float _mass;

		vec3 _pos;
		vec3 _prev;
		vec3 _acc;
		vec3 _norm;
};

#endif // PARTICLE_HPP