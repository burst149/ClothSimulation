#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <iostream>

#include <fstream>
#include <sstream>
#include <string>

#include "GL/gl3w.h"

class Texture
{
	public:
		Texture(const std::string filename);
		~Texture();

		unsigned int id;
};

#endif // TEXTURE_HPP