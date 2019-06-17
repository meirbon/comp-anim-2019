#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>

class Shader
{
  public:
	/**
   	 * Initializes a shader program
   	 * @param vertexPath
   	 * @param fragmentPath
   	 */
	explicit Shader(const char *vertexPath, const char *fragmentPath);
	~Shader();

	/**
	 * Returns shader program ID
	 * @return
	 */
	const GLuint &getShaderId() const
	{
		return m_ShaderId;
	}

	/**
	 * Use this shader
	 */
	void bind() const
	{
		glUseProgram(m_ShaderId);
	}

	/**
	 * Unbind any shader
	 */
	void unbind() const
	{
		glUseProgram(0);
	}

	/**
	 * Sets uniform array to float-type value T
	 * @tparam T			Type of value
	 * @param name			Uniform name
	 * @param data			Data to be set
	 * @param transpose		Transpose (only valid in case of matrices)
	 * @param offset		Offset to start loading data to
	 * @param count			Number of elements (automatically retrieved from data if count = 0)
	 */
	template <typename T>
	void setUniformFloatArray(const char *name, std::vector<T> data, GLboolean transpose = false, unsigned int offset = 0, unsigned int count = 0)
	{
		const auto d = (GLfloat *)data.data();
		const auto componentCount = sizeof(T) / 4;
		const auto location = getUniformLocation(name, offset);
		const auto cnt = count > 0 ? count : data.size();

		switch (componentCount)
		{
		case (1):
			glUniform1fv(location, count, d);
			break;
		case (2):
			glUniform2fv(location, count, d);
			break;
		case (3):
			glUniform3fv(location, count, d);
			break;
		case (4):
			glUniform4fv(location, count, d);
			break;
		case (8):
			glUniformMatrix2fv(location, data.size(), transpose, d);
		case (12):
			glUniformMatrix3fv(location, data.size(), transpose, d);
		case (16):
			glUniformMatrix4fv(location, data.size(), transpose, d);
			break;
		default:
			std::cout << "Invalid uniform data size: " << componentCount << std::endl;
			break;
		}
	}

	/**
	 * Sets uniform array to int-type value T
	 * @tparam T			Type of value
	 * @param name			Uniform name
	 * @param data			Data to be set
	 * @param transpose		Transpose (only valid in case of matrices)
	 * @param offset		Offset to start loading data to
	 * @param count			Number of elements (automatically retrieved from data if count = 0)
	 */
	template <typename T>
	void setUniformIntArray(const char *name, std::vector<T> data, GLboolean transpose = false, unsigned int offset = 0, unsigned int count = 0)
	{
		const auto d = (GLint *)data.data();
		const auto componentCount = sizeof(T) / 4;
		const auto location = getUniformLocation(name, offset);
		const auto cnt = count > 0 ? count : data.size();

		switch (componentCount)
		{
		case (1):
			glUniform1iv(location, count, d);
			break;
		case (2):
			glUniform2iv(location, count, d);
			break;
		case (3):
			glUniform3iv(location, count, d);
			break;
		case (4):
			glUniform4iv(location, count, d);
			break;
		default:
			std::cout << "Invalid uniform data size: " << componentCount << std::endl;
			break;
		}
	}
	/**
  	 * Sets uniform array to uint-type value T
  	 * @tparam T			Type of value
  	 * @param name			Uniform name
  	 * @param data			Data to be set
  	 * @param transpose		Transpose (only valid in case of matrices)
  	 * @param offset		Offset to start loading data to
  	 * @param count			Number of elements (automatically retrieved from data if count = 0)
  	 */
	template <typename T>
	void setUniformUnsignedIntArray(const char *name, std::vector<T> data, GLboolean transpose = false, unsigned int offset = 0, unsigned int count = 0)
	{
		const auto d = (GLuint *)data.data();
		const auto componentCount = sizeof(T) / 4;
		const auto location = getUniformLocation(name, offset);
		const auto cnt = count > 0 ? count : data.size();

		switch (componentCount)
		{
		case (1):
			glUniform1uiv(location, count, d);
			break;
		case (2):
			glUniform2uiv(location, count, d);
			break;
		case (3):
			glUniform3uiv(location, count, d);
			break;
		case (4):
			glUniform4uiv(location, count, d);
			break;
		default:
			std::cout << "Invalid uniform data size: " << componentCount << std::endl;
			break;
		}
	}

	/**
	 * Sets uniform to float-type value T
	 * @tparam T			Type of value
	 * @param name			Uniform name
	 * @param value			Value to be set
	 * @param transpose		Transpose value (only valid in case of matrix)
	 */
	template <typename T>
	void setUniformFloat(const char *name, const T &value, GLboolean transpose = false)
	{
		const GLfloat *data = (GLfloat *)&value;
		const auto componentCount = sizeof(T) / 4;
		const auto location = getUniformLocation(name);

		switch (componentCount)
		{
		case (1):
			glUniform1fv(location, 1, data);
			break;
		case (2):
			glUniform2fv(location, 1, data);
			break;
		case (3):
			glUniform3fv(location, 1, data);
			break;
		case (4):
			glUniform4fv(location, 1, data);
			break;
		case (8):
			glUniformMatrix2fv(location, 1, transpose, data);
		case (12):
			glUniformMatrix3fv(location, 1, transpose, data);
		case (16):
			glUniformMatrix4fv(location, 1, transpose, data);
			break;
		default:
			std::cout << "Invalid uniform data size: " << componentCount << std::endl;
			break;
		}
	}

	/**
   	 * Sets uniform to int-type value T
   	 * @tparam T			Type of value
   	 * @param name			Uniform name
   	 * @param value			Value to be set
   	 * @param transpose		Transpose value (only valid in case of matrix)
   	 */
	template <typename T>
	void setUniformInt(const char *name, const T &value, GLboolean transpose = false)
	{
		const GLint *data = (GLint *)&value;
		const auto componentCount = sizeof(T) / 4;
		const auto location = getUniformLocation(name);

		switch (componentCount)
		{
		case (1):
			glUniform1iv(location, 1, data);
			break;
		case (2):
			glUniform2iv(location, 1, data);
			break;
		case (3):
			glUniform3iv(location, 1, data);
			break;
		case (4):
			glUniform4iv(location, 1, data);
			break;
		default:
			std::cout << "Invalid uniform data size: " << componentCount << std::endl;
			break;
		}
	}

	/**
  	 * Sets uniform to uint-type value T
  	 * @tparam T			Type of value
  	 * @param name			Uniform name
  	 * @param value			Value to be set
  	 * @param transpose		Transpose value (only valid in case of matrix)
  	 */
	template <typename T>
	void setUniformUnsignedInt(const char *name, const T &value, GLboolean transpose = false)
	{
		const GLuint *data = (GLuint *)&value;
		const auto componentCount = sizeof(T) / 4;
		const auto location = getUniformLocation(name);

		switch (componentCount)
		{
		case (1):
			glUniform1uiv(location, 1, data);
			break;
		case (2):
			glUniform2uiv(location, 1, data);
			break;
		case (3):
			glUniform3uiv(location, 1, data);
			break;
		case (4):
			glUniform4uiv(location, 1, data);
			break;
		default:
			std::cout << "Invalid uniform data size: " << componentCount << std::endl;
			break;
		}
	}

	/**
	 * Return uniform location at 'name'
	 * @param name		Uniform name
	 * @param index		Index of uniform array, -1 if uniform is not an array
	 * @return			Uniform location
	 */
	inline GLint getUniformLocation(const char *name, int index = -1)
	{
		if (index >= 0)
		{
			char n[64]{};
			std::sprintf(n, "%s[%i]", name, index);
			return glGetUniformLocation(m_ShaderId, n);
		}
		else
		{
			return glGetUniformLocation(m_ShaderId, name);
		}
	}

	/**
	 * Return attribute location at 'name'
	 * @param name		Attribute name
	 * @return			Attribute location
	 */
	inline GLint getAttributeLocation(const char *name)
	{
		return glGetAttribLocation(m_ShaderId, name);
	}

  private:
	GLuint m_ShaderId;
	const char *m_VertPath;
	const char *m_FragPath;

	GLuint load();
	void checkCompileErrors(GLuint shader, std::string type);
};