#pragma once

#include "Model.hpp"
#include "ResourceIdentifiers.hpp"

namespace px
{
	class Render
	{
	public:
		Render(ModelHolder & model, Models::ID modelID, Shaders::ID shader, std::string name);

	public:
		void Draw();

	public:
		void SetShader(Shaders::ID shader);

	public:
		Shaders::ID GetShader() const;
		Models::ID GetModel() const;
		std::string GetName() const;

	private:
		ModelHolder m_model;
		Shaders::ID m_shader;
		Models::ID m_modelID;
		std::string m_name;
	};
}


