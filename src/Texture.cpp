#include "Texture.hpp"

Texture::Texture(const std::string filename)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Open file for loading
	std::ifstream file(filename.c_str(), std::ifstream::binary);

	if (!file.good())
		return;

	// Header data
	char type;
	short int width;
	short int height;
	char bitDepth;

	// Ignore first two bytes
	file.ignore(2);

	// Read image type
	file.read(&type, 1);
	
	if (type != 2 && type != 10)
		return;

	// Read image specification
	file.ignore(9);
	file.read((char*)&width, 2);
	file.read((char*)&height, 2);
	file.read((char*)&bitDepth, 1);
	file.ignore(1);

	int colorMode = bitDepth/8;
	int imageSize = width*height*colorMode;

	if (!file.good() || (colorMode != 3 && colorMode != 4))
		return;

	// Read the image data
	unsigned char *data;
	data = new unsigned char[imageSize];
	if (type == 2)
		file.read((char*)data, imageSize);
	else if (type == 10)
	{
		int count = width*height;
		int pixel = 0;
		int byte = 0;
		unsigned char* buffer = new unsigned char[colorMode];

		// Read compressed data
		while(pixel < count)
		{
			unsigned char header = 0;
			file.read((char*)&header, 1);
			if (header<128)
			{
				// RAW
				header++;
				for (int i = 0; i < header; i++)
				{
					file.read((char*)buffer, colorMode);
					data[byte] = buffer[0];
					data[byte+1] = buffer[1];
					data[byte+2] = buffer[2];
					if (colorMode == 4)
						data[byte+3] = buffer[3];

					byte += colorMode;
					pixel++;
				}
			}
			else
			{
				// RLE
				header -= 127;
				file.read((char*)buffer, colorMode);
				for (int i = 0; i < header; i++)
				{
					data[byte] = buffer[0];
					data[byte+1] = buffer[1];
					data[byte+2] = buffer[2];
					if (colorMode == 4)
						data[byte+3] = buffer[3];

					byte += colorMode;
					pixel++;
				}
			}
		}
		delete[] buffer;
	}

	file.close();

	if (bitDepth == 24)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	else if (bitDepth == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

	delete[] data;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}

Texture::~Texture()
{
	glDeleteTextures(1, &id);
}