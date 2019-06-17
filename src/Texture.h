#pragma once

#include <GL/glew.h>

#include <string>

class Texture
{
  public:
	/**
  	 * Initializes texture of type TextureTarget
  	 * @param TextureTarget 	Texture type
  	 * @param FileName 			File to load
  	 */
	Texture(GLenum TextureTarget, const std::string &FileName);

	/**
	 * Load texture from filesystem
	 * @return
	 */
	bool load();

	/**
	 * Bind texture
	 */
	void bind();

	/**
     * Bind texture at given texture unit
     * @param TextureUnit 	Texture unit to bind to
     */
	void bind(GLenum TextureUnit);

	/**
	 * Unbind any texture of current type
	 */
	void unbind();

  private:
	std::string m_FileName;
	GLenum m_TextureTarget;
	GLuint m_TextureObj;
};