#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace px
{
	namespace Shaders
	{
		enum ID
		{
			Phong,
			Debug,
			Grid,
			RenderTexture,
			Outline
		};
	}

	class Shader
	{
	public:
		static void LoadShaders(Shaders::ID id, const char* vertexPath, const char* fragmentPath);

	public:
		static void Use(Shaders::ID id);

	public:
		static void SetBool(Shaders::ID id, const std::string & name, bool value);
		static void SetInt(Shaders::ID id, const std::string & name, int value);
		static void SetFloat(Shaders::ID id, const std::string & name, float value);
		static void SetFloat2v(Shaders::ID id, const std::string & name, glm::vec2 vector);
		static void SetFloat3v(Shaders::ID id, const std::string & name, glm::vec3 vector);
		static void SetFloat4v(Shaders::ID id, const std::string & name, glm::vec4 vector);
		static void SetMatrix3x3(Shaders::ID id, const std::string & name, glm::mat3 matrix);
		static void SetMatrix4x4(Shaders::ID id, const std::string & name, glm::mat4 matrix);

	private:
		static void CreateShader(Shaders::ID id, const char* path, GLenum shaderType);
		static void AttachShader(Shaders::ID id);
		static void CheckCompileErrors(unsigned int shader, std::string type);

	private:
		struct ShaderInfo
		{
			unsigned int id;
			std::vector<unsigned int> shaders;
		};

		static std::map<Shaders::ID, ShaderInfo> m_shaders;
	};

}