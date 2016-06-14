#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <stdlib.h>
#include <vector>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "Particle.hpp"

#define LIMIT 10000

class ParticleSystem
{
	public:
		ParticleSystem(GLuint shader, GLuint texture, float(*alpha_func)(float), float(*scale_func)(float));
		~ParticleSystem();

		void AddParticles(unsigned int amount);
		void RemoveParticles(unsigned int amount);
		void SetParticleMass(float mass);
		void SetEmitterPosition(vec3 pos);
		void SetEmitterSize(vec3 size);	
		void SetLifetime(float time);
		
		void AddForce(vec3 f);
		void AddGravity();

		void Update(float dt);
		void Render();

	private:
		GLuint _vao;
		GLuint _quadBuffer;
		GLuint _matBuffer;
		GLuint _alphaBuffer;
		GLuint _rotBuffer;

		GLuint _shader;
		GLuint _texture;

		std::vector<Particle> _particles;
		std::vector<float> _times;
		std::vector<float> _alphas;
		std::vector<float> _rotations;
		std::vector<float> _scales;
		std::vector<float> _matrices;

		vec3 _pos;
		vec3 _size;
		vec3 _force;
		float _mass;

		float _lifetime;
		float (*_alpha_func)(float);
		float (*_scale_func)(float);
};

#endif // PARTICLE_SYSTEM_HPP