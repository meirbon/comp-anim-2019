#include <GL/glew.h>

#include "Texture.h"

#include "utils/File.h"
#include "utils/Logger.h"

#include <FreeImage.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

Texture::Texture(GLenum TextureTarget, const std::string &FileName)
	: m_FileName(FileName), m_TextureObj(0), m_TextureTarget(TextureTarget)
{
}

bool Texture::load()
{
	// Generate texture object
	glGenTextures(1, &m_TextureObj);
	glBindTexture(m_TextureTarget, m_TextureObj);

	// Set initial data in case texture loading fails
	constexpr int dummyData = 2147483627 + 20;
	glTexImage2D(m_TextureTarget, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &dummyData);
	unbind();

	// Check if file actually exists
	if (!utils::file::exists(m_FileName))
	{
		DEBUG("Could not open file: %s", m_FileName.c_str());
		return false;
	}

	// Get file type using FreeImage
	const char *filename = m_FileName.c_str();
	FREE_IMAGE_FORMAT type = FIF_UNKNOWN;
	type = FreeImage_GetFileType(filename);
	if (type == FIF_UNKNOWN)
		type = FreeImage_GetFIFFromFilename(filename);

	// Load image from disk and convert to 32 bits
	FIBITMAP *tmp = FreeImage_Load(type, filename);
	FIBITMAP *image = FreeImage_ConvertTo32Bits(tmp);

	// Unload image in original format
	FreeImage_Unload(tmp);

	const auto width = FreeImage_GetWidth(image);
	const auto height = FreeImage_GetHeight(image);

	// Initialize an array of values
	std::vector<unsigned int> colors(width * height);

	// Loop over every pixel
	for (unsigned int y = 0; y < height; y++)
	{
		for (unsigned int x = 0; x < width; x++)
		{
			// Retrieve data from current pixel
			RGBQUAD quad;
			FreeImage_GetPixelColor(image, x, y, &quad);
			const auto red = (unsigned int)(quad.rgbRed);
			const auto blue = (unsigned int)(quad.rgbBlue);
			const auto green = (unsigned int)(quad.rgbGreen);
			const auto alpha = (unsigned int)(quad.rgbReserved);

			// Convert to our own format
			colors[x + y * width] = (red << 0) | (blue << 8) | (green << 16) | (alpha << 24);
		}
	}

	// Unload FreeImage object, not needed anymore
	FreeImage_Unload(image);

	// Bind current texture object
	glBindTexture(m_TextureTarget, m_TextureObj);
	// Update its data to image data
	glTexImage2D(m_TextureTarget, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
	// Generate mip-map levels
	glGenerateMipmap(m_TextureTarget);
	// Set clamping and filtering methods
	glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(m_TextureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(m_TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	unbind();
	// Texture loaded successfully
	return true;
}

void Texture::bind()
{
	glBindTexture(m_TextureTarget, m_TextureObj);
}

void Texture::bind(GLenum TextureUnit)
{
	glActiveTexture(TextureUnit);
	glBindTexture(m_TextureTarget, m_TextureObj);
}

void Texture::unbind()
{
	glBindTexture(m_TextureTarget, 0);
}