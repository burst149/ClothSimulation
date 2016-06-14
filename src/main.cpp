#include <iostream>
#include <vector>
#include <sstream>

#include <stdlib.h>
#include <time.h>

#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "linmath.h"

#include "shaders.hpp"
#include "Texture.hpp"
#include "Model.hpp"
#include "ParticleSystem.hpp"
#include "Cloth.hpp"

#define SNOW_AMOUNT 2000
#define SMOKE_AMOUNT 16
#define FLAG_AMOUNT 32

// Particle functions

	float smoke_alpha(float t) {return 0.5f - t * 0.5f;}
	float smoke_scale(float t) {return 8.0f * t;}

	float snow_alpha(float t) {return t;}
	float snow_scale(float t) {return 0.2f;}

int main(int argc, char *argv[])
{
	// Get window size

	int win_W = 800;
	if (argc > 1) win_W = atoi(argv[1]);
	int win_H = win_W * 9 / 16.0f;

	// Initialisation

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(win_W, win_H, "LAB3", NULL, NULL);
	glfwMakeContextCurrent(window);

	if (!window) { glfwTerminate(); return -1; }
	if (gl3wInit())  return -1;

	std::cout<<"OPENGL VERSION: "<<glGetString(GL_VERSION)<<std::endl;

	time_t t;
	srand((unsigned) time(&t));

	// Camera setup

	vec3 camera_pos = {8.0f, -8.0f, 1.0f};
	vec4 camera_dir = {0.0f, 1.0f, 0.0f, 1.0f};
	float hAngle = 0.5f;
	float vAngle = 0.1f;

	double mx, my;
	glfwSetCursorPos(window, win_W * 0.5f, win_H * 0.5f);

	// OpenGL setup

	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE); 
	
	// Shader setup

	GLuint modelShader = InitModelShader();
	GLuint particleShader = InitParticleShader();
	

	// Texture loading

	Texture floor_tex("floor.tga");
	Texture bonfire_tex("bonfire.tga");
	Texture sky_tex("sky.tga");
	Texture wall_tex("wall.tga");
	Texture door_tex("door.tga");
	Texture orb_tex("orb.tga");
	Texture pole_tex("pole.tga");

	Texture smoke_tex("smoke.tga");
	Texture snow_tex("flake.tga");

	Texture flag_tex("flag_torn.tga");
	Texture flagL_tex("flag_long.tga");

	// Scene construction
	
		vec3 place;
		mat4x4 rot;
		mat4x4_identity(rot);

		Model sky("sky.obj", modelShader, sky_tex.id);
		place[0] = 0.0f; place[1] = 0.0f; place[2] = -100.0f;
		sky.SetPosition(place);
		sky.SetScale(400.0f);
		mat4x4_rotate_Z(rot, rot, -1.57f);
		sky.SetOrientation(rot);

		// Building

			Model bonfire("bonfire.obj", modelShader, bonfire_tex.id);
			vec3 fire_pos = {4.0f, -4.0f, -0.05f};
			bonfire.SetPosition(fire_pos);
			fire_pos[2] = -0.2f;
			bonfire.SetScale(0.5f);

			Model surface("cube_full.obj", modelShader, floor_tex.id);
			place[0] = 0.0f; place[1] = 0.0f; place[2] = -10.1f;
			surface.SetPosition(place);
			surface.SetScale(20.0f);

			Model wallN("wall.obj", modelShader, wall_tex.id);
			place[0] = 9.5f; place[1] = 0.0f; place[2] = -1.11f;
			wallN.SetPosition(place);
			wallN.SetScale(2.0f);
			mat4x4_identity(rot);
			wallN.SetOrientation(rot);

			Model wallW("wall.obj", modelShader, wall_tex.id);
			place[0] = 0.0f; place[1] = 9.5f; place[2] = -0.1f;
			wallW.SetPosition(place);
			wallW.SetScale(2.0f);
			mat4x4_rotate_Z(rot, rot, 1.57f);
			wallW.SetOrientation(rot);

			Model wallH("wall.obj", modelShader, wall_tex.id);
			place[0] = 0.0f; place[1] = 9.49f; place[2] = 2.9f;
			wallH.SetPosition(place);
			wallH.SetScale(2.0f);
			wallH.SetOrientation(rot);

			Model wallS("wall.obj", modelShader, wall_tex.id);
			place[0] = -9.5f; place[1] = 0.0f; place[2] = -1.11f;
			wallS.SetPosition(place);
			wallS.SetScale(2.0f);
			mat4x4_rotate_Z(rot, rot, 1.57f);
			wallS.SetOrientation(rot);

			Model wallE("wall.obj", modelShader, wall_tex.id);
			place[0] = 0.0f; place[1] = -9.5f; place[2] = -1.10f;
			wallE.SetPosition(place);
			wallE.SetScale(2.0f);
			mat4x4_rotate_Z(rot, rot, 1.57f);
			wallE.SetOrientation(rot);

			Model door("cube_full.obj", modelShader, door_tex.id);
			place[0] = -4.8f; place[1] = 10.9f; place[2] = 1.8f;
			door.SetPosition(place);
			mat4x4_rotate_X(rot, rot, 1.57f);
			door.SetScale(4.0f);
			door.SetOrientation(rot);

			Model orb("uvsphere.obj", modelShader, orb_tex.id);
			place[0] = -8.0f; place[1] = -8.0f; place[2] = 0.9f;
			orb.SetPosition(place);
			orb.SetScale(0.90f);
			bool grabbing_ball = false;

		// Flags
		
			Model pole("pole.obj", modelShader, pole_tex.id);
			place[0] = 3.5f; place[1] = 9.0f; place[2] = 5.1f;
			mat4x4_identity(rot);
			mat4x4_rotate_Y(rot, rot, 3.14f);
			pole.SetOrientation(rot);
			pole.SetPosition(place);

			Cloth flag(4, 4, FLAG_AMOUNT, FLAG_AMOUNT, modelShader, flag_tex.id);
			place[0] = 3.5f; place[1] = 4.0f; place[2] = 5.0f;
			flag.SetPosition(place);
			flag.FreezeCorner(0);
			flag.FreezeCorner(1);
			bool released = false;
			bool grabbing_flag = false;

			Model poleL("pole.obj", modelShader, pole_tex.id);
			place[0] = -7.0f; place[1] = 10.0f; place[2] = 5.0f;
			mat4x4_identity(rot);
			mat4x4_rotate_Y(rot, rot, 3.14f);
			mat4x4_rotate_Z(rot, rot, 1.57f);
			mat4x4_rotate_X(rot, rot, 1.57f);
			poleL.SetOrientation(rot);
			poleL.SetPosition(place);

			Cloth flagL(4, 1, 16, 4, modelShader, flagL_tex.id);
			place[0] = -7.0f; place[1] = 9.9f; place[2] = 10.0f;
			flagL.SetPosition(place);
			flagL.FreezeCorner(0);
			flagL.FreezeCorner(2);

	// Particles
		vec3 lift = {0.0f, 0.0f, 4.0f};
		vec3 wind = {-1.0f, 0.0f, 0.0f};

		// Smoke particles
			ParticleSystem* smoke  = new ParticleSystem(particleShader, smoke_tex.id, smoke_alpha, smoke_scale);
			place[0] = 4.0f; place[1] = -4.0f; place[2] = 0.2f;
			smoke->SetEmitterPosition(place);
			place[0] = 0.1f; place[1] = 0.1f; place[2] = 0.2f;
			smoke->SetEmitterSize(place);
			smoke->SetLifetime(10);
			smoke->AddParticles(SMOKE_AMOUNT);
			smoke->SetParticleMass(10);
			
		// Snow particles
			ParticleSystem* snow  = new ParticleSystem(particleShader, snow_tex.id, snow_alpha, snow_scale);
			place[0] = 0.0f; place[1] = 0.0f; place[2] = 10.0f;
			snow->SetEmitterPosition(place);
			place[0] = 20.0f; place[1] = 20.0f; place[2] = 5.0f;
			snow->SetEmitterSize(place);
			snow->SetLifetime(2);
			snow->AddParticles(SNOW_AMOUNT);
			snow->SetParticleMass(0.5);

	// Timestep

	double dt = 0;
	double phys_accumulator = 0;
	float phys_timestep = 1/60.0f;

	// Main loop

	while(!glfwWindowShouldClose(window))
	{
		// Controls
		
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				break;

			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				if (wind[0] > -20.0f)
					wind[0] -= 5.0f * dt;

			if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
				if (wind[0] < 20.0f)
					wind[0] += 5.0f * dt;

			if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
					wind[0] = 0;

			if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
				released = true;

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				vec3 movement;
				vec3_scale(movement, camera_dir, 5*dt);
				vec3_add(camera_pos, camera_pos, movement);
			}

			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				vec3 right;
				vec3 up = {0.0f, 0.0f, 1.0f};
				vec3_mul_cross(right, camera_dir, up);
				vec3_norm(right, right);

				vec3 movement;
				vec3_scale(movement, right, 5*dt);
				vec3_sub(camera_pos, camera_pos, movement);
			}

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				vec3 movement;
				vec3_scale(movement, camera_dir, 5*dt);
				vec3_sub(camera_pos, camera_pos, movement);
			}

			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				vec3 right;
				vec3 up = {0.0f, 0.0f, 1.0f};
				vec3_mul_cross(right, camera_dir, up);
				vec3_norm(right, right);

				vec3 movement;
				vec3_scale(movement, right, 5*dt);
				vec3_add(camera_pos, camera_pos, movement);
			}

			if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
				vAngle += 0.8f * dt;
			if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
				vAngle -= 0.8f * dt;
			if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
				hAngle += 0.8f * dt;
			if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
				hAngle -= 0.8f * dt;

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !grabbing_flag)
			{
				vec3 ball;
				orb.GetPosition(ball);

				vec3 grab = {0.0f, 0.0f, 0.0f};
				vec3_add(grab, grab, camera_dir);
				vec3_scale(grab, grab, 2.5f);
				vec3_add(grab, camera_pos, grab);

				vec3_sub(ball, ball, grab);
				if (vec3_len(ball) < 2)
				{
					orb.SetPosition(grab);
					grabbing_ball = true;
				}
			}

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && released && !grabbing_ball)
			{
				vec3 flagCorner;
				flag.GetPosition(flagCorner);

				vec3 grab = {0.0f, 0.0f, 0.0f};
				vec3_add(grab, grab, camera_dir);
				vec3_scale(grab, grab, 2.5f);
				vec3_add(grab, camera_pos, grab);

				vec3_sub(flagCorner, flagCorner, grab);
				if (vec3_len(flagCorner) < 2)
				{
					flag.FreezeCorner(0);
					flag.SetPosition(grab);
					grabbing_flag = true;
				}
			}

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
			{
				grabbing_ball = false;
				grabbing_flag = false;
				if (released)
					flag.Release();
			}

		// Camera bounds

		if (camera_pos[0] < -8) camera_pos[0] = -8;
		if (camera_pos[0] > 8) camera_pos[0] = 8;
		if (camera_pos[1] < -8) camera_pos[1] = -8;
		if (camera_pos[1] > 8) camera_pos[1] = 8;
		if (camera_pos[2] < 1) camera_pos[2] = 1;
		if (camera_pos[2] > 6.5f) camera_pos[2] = 6.5f;

		// Mouse look

		glfwGetCursorPos(window, &mx, &my);
		glfwSetCursorPos(window, win_W * 0.5f, win_H * 0.5f);

		hAngle += 0.5f * ((win_W * 0.5f) - mx) * 0.005f;
		vAngle += 0.5f * ((win_H * 0.5f) - my) * 0.005f;

		if (hAngle >  3.14f) hAngle -= 6.28f;
		if (hAngle < -3.14f) hAngle += 6.28f;

		if (vAngle >  1.5f) vAngle =  1.5f;
		if (vAngle < -1.5f) vAngle = -1.5f;

		mat4x4 look;
		mat4x4_identity(look);
		mat4x4_rotate_Z(look, look, hAngle);
		mat4x4_rotate_X(look, look, vAngle);

		vec3 Y = {0.0f, 1.0f, 0.0f};
		mat4x4_mul_vec4(camera_dir, look, Y);
		vec3_norm(camera_dir, camera_dir);

		// View matrix
		mat4x4 view;
		vec3 up = {0.0f, 0.0f, 1.0f};
		vec3 target;
		vec3_add(target, camera_dir, camera_pos);
		mat4x4_look_at(view, camera_pos, target, up);

		// Projection Matrix
		mat4x4 proj;
		mat4x4_perspective(proj, 0.785, 16.0f/9.0f, 0.1f, 1000.0f);

		// Rendering

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Scene rendering

			glUseProgram(modelShader);
			GLint uniView = glGetUniformLocation(modelShader, "view");
			glUniformMatrix4fv(uniView, 1, GL_FALSE, *view);
			GLint uniProj = glGetUniformLocation(modelShader, "proj");
			glUniformMatrix4fv(uniProj, 1, GL_FALSE, *proj);
			
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				
				sky.Render();
				bonfire.Render();
				surface.Render();
				wallN.Render();
				wallS.Render();
				wallW.Render();
				wallE.Render();
				wallH.Render();
				door.Render();
				pole.Render();
				poleL.Render();

				vec3 ball;
				orb.GetPosition(ball);
				orb.Render();

		// Physics update

		while (phys_accumulator >= phys_timestep)
		{
			// Cloth update

			flag.AddGravity();
			flag.AddWind(wind);
			
			flag.CollideSphere(camera_pos, 1.0f);
			flag.CollideSphere(ball, 1.0f);
			flag.CollideSphere(fire_pos, 1.0f);
			flag.CollideBoundary(8.9f);
			flag.Update(phys_timestep);

			flagL.AddGravity();
			flagL.AddWind(wind);
			flagL.CollideSphere(ball, 1.0f);
			flagL.Update(phys_timestep);

			// Particles update
				
				// Smoke update
				smoke->AddForce(lift);
				smoke->AddForce(wind);
				smoke->Update(phys_timestep);

				// Snow update

				place[0] = -wind[0] * 2; place[1] = 0.0f; place[2] = 10.0f;
				snow->SetEmitterPosition(place);
				snow->AddGravity();
				snow->AddForce(wind);
				snow->Update(phys_timestep);
				

			phys_accumulator -= phys_timestep;
		}
		

		// Cloth render

		flag.Render();
		flagL.Render();

		// Particle rendering

		glUseProgram(particleShader);
		uniView = glGetUniformLocation(particleShader, "view");
		glUniformMatrix4fv(uniView, 1, GL_FALSE, *view);
		uniProj = glGetUniformLocation(particleShader, "proj");
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, *proj);
		
		glDepthMask(GL_FALSE);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		snow->Render();
		smoke->Render();

		glDepthMask(GL_TRUE);


		glfwSwapBuffers(window);
		glfwPollEvents();

		// Time keeping

		dt = glfwGetTime();
		glfwSetTime(0);

		if (dt > 0.2)
			dt = 0.2;

		phys_accumulator += dt;

		// Display FPS

			std::stringstream title;
			title << 1/dt;
			glfwSetWindowTitle(window, title.str().c_str());
	}

	// Cleanup

	delete smoke;
	delete snow;

	glDeleteProgram(modelShader);
	glDeleteProgram(particleShader);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

