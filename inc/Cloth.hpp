#ifndef CLOTH_HPP
#define CLOTH_HPP

#include <vector>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "Particle.hpp"

class Link
{
	public:
		Link(Particle* a, Particle* b);
		
		void Update();

	private:
		Particle* _a;
		Particle* _b;
		float _dist;
};

class Cloth
{
	public:
		Cloth(float w, float h, unsigned int nX, unsigned int nY, GLuint shader, GLuint texture);
		~Cloth();
	
		void Update(float dt);
		void Render();

		void AddForce(vec3 f);
		void AddGravity();
		void AddWind(vec3 w);

		void CollideSphere(vec3 center, float radius);
		void CollideBoundary(float size);

		void SetPosition(vec3 v);
		void GetPosition(vec3 v);

		void Release();
		void FreezeCorner(short corner);
		bool IsReleased();

	private:
		Particle* GetParticle(unsigned int x, unsigned int y);
		void MakeLink(Particle* a, Particle* b);
		void CalcNormal(Particle* a, Particle* b, Particle* c, vec3 n);
		void UpdateVertex(unsigned int n, unsigned int x, unsigned int y);

		unsigned int _nX;
		unsigned int _nY;

		std::vector<Particle> _nodes;
		std::vector<Link> _links;

		unsigned int _polycount;

		std::vector<GLfloat> _vertices;

		bool _free;

		GLuint _vao;
		GLuint _vbo;
		GLuint _shader;

		GLuint _texture;

		vec3 _color;
		vec3 _pos;
};

#endif // CLOTH_HPP
