#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace px
{
	std::map<Shaders::ID, Shader::ShaderInfo> Shader::m_shaders;

	void Shader::LoadShaders(Shaders::ID id, const char * vertexPath, const char * fragmentPath)
	{
		CreateShader(id, vertexPath, GL_VERTEX_SHADER);
		CreateShader(id, fragmentPath, GL_FRAGMENT_SHADER);

		AttachShader(id);
	}

	void Shader::Use(Shaders::ID id)
	{
		glUseProgram(m_shaders[id].id);
	}

	void Shader::CreateShader(Shaders::ID id, const char* path, GLenum shaderType)
	{
		//Reader shader source
		std::string code;
		std::ifstream shaderFile;
		shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			shaderFile.open(path);
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << shaderFile.rdbuf();

			shaderFile.close();
			code = vShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}

		//Compile shader
		const char* vShaderCode = code.c_str();
		unsigned int shader;

		shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &vShaderCode, NULL);
		glCompileShader(shader);
		CheckCompileErrors(shader, path);

		m_shaders[id].shaders.push_back(shader);
	}

	void Shader::AttachShader(Shaders::ID id)
	{
		m_shaders[id].id = glCreateProgram();

		for (auto shader : m_shaders[id].shaders)
			glAttachShader(m_shaders[id].id, shader);

		glLinkProgram(m_shaders[id].id);
		CheckCompileErrors(m_shaders[id].id, "PROGRAM");

		//Done with shaders
		for (auto shader : m_shaders[id].shaders)
			glDeleteShader(shader);
	}

	void Shader::CheckCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR at: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}

	void Shader::SetBool(Shaders::ID id, const std::string & name, bool value)
	{
		glUniform1i(glGetUniformLocation(m_shaders[id].id, name.c_str()), (int)value);
	}

	void Shader::SetInt(Shaders::ID id, const std::string & name, int value)
	{
		glUniform1i(glGetUniformLocation(m_shaders[id].id, name.c_str()), value);
	}

	void Shader::SetFloat(Shaders::ID id, const std::string & name, float value)
	{
		glUniform1f(glGetUniformLocation(m_shaders[id].id, name.c_str()), value);
	}

	void Shader::SetFloat2v(Shaders::ID id, const std::string & name, glm::vec2 vector)
	{
		glUniform2f(glGetUniformLocation(m_shaders[id].id, name.c_str()), vector.x, vector.y);
	}

	void Shader::SetFloat3v(Shaders::ID id, const std::string & name, glm::vec3 vector)
	{
		glUniform3f(glGetUniformLocation(m_shaders[id].id, name.c_str()), vector.x, vector.y, vector.z);
	}

	void Shader::SetFloat4v(Shaders::ID id, const std::string & name, glm::vec4 vector)
	{
		glUniform4f(glGetUniformLocation(m_shaders[id].id, name.c_str()), vector.x, vector.y, vector.z, vector.w);
	}

	void Shader::SetMatrix3x3(Shaders::ID id, const std::string & name, glm::mat3 matrix)
	{
		unsigned int matrixLoc = glGetUniformLocation(m_shaders[id].id, name.c_str());
		glUniformMatrix3fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::SetMatrix4x4(Shaders::ID id, const std::string & name, glm::mat4 matrix)
	{
		unsigned int matrixLoc = glGetUniformLocation(m_shaders[id].id, name.c_str());
		glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}
