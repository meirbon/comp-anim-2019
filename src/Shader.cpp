#include <GL/glew.h>

#include "Shader.h"

#include <string>
#include <vector>

#include "utils/File.h"
#include "utils/Logger.h"

Shader::Shader(const char *vertexPath, const char *fragmentPath)
	: m_VertPath(vertexPath), m_FragPath(fragmentPath)
{
	m_ShaderId = load();
}

GLuint Shader::load()
{
	const GLuint program = glCreateProgram();
	const GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	const GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vert_string = utils::file::read(m_VertPath);
	std::string frag_string = utils::file::read(m_FragPath);

	const char *vert_source = vert_string.c_str();
	const char *frag_source = frag_string.c_str();

	glShaderSource(vert_shader, 1, &vert_source, nullptr);
	glCompileShader(vert_shader);
	checkCompileErrors(vert_shader, "VERTEX");

	glShaderSource(frag_shader, 1, &frag_source, nullptr);
	glCompileShader(frag_shader);
	checkCompileErrors(frag_shader, "FRAGMENT");

	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	return program;
}

void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	std::vector<GLchar> infoLog;
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			infoLog.resize(maxLength);
			glGetShaderInfoLog(shader, infoLog.size(), nullptr, infoLog.data());
			WARNING("Shader compilation error of type: %s\n%s", type.c_str(), infoLog.data());
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLint maxLength = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
			infoLog.resize(maxLength);
			glGetProgramInfoLog(shader, infoLog.size(), nullptr, infoLog.data());
			WARNING("Shader compilation error of type: %s\n%s", type.c_str(), infoLog.data());
		}
	}
}

Shader::~Shader() { glDeleteProgram(m_ShaderId); }