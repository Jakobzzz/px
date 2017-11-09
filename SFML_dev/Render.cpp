#include "Render.hpp"

namespace px
{
	Render::Render(ModelHolder & model, Models::ID modelID, Shaders::ID shader, std::string name) : m_model(model), m_modelID(modelID), 
																									m_shader(shader), m_name(name)
	{
	}

	void Render::Draw()
	{
		//Need to find a way to configure the right shader properties for a shader...
		m_model->Draw(m_modelID, m_shader);
	}

	void Render::SetShader(Shaders::ID shader)
	{
		m_shader = shader;
	}

	void Render::SetName(std::string name)
	{
		m_name = name;
	}

	Shaders::ID Render::GetShader() const
	{
		return m_shader;
	}

	Models::ID Render::GetModel() const
	{
		return m_modelID;
	}

	std::string Render::GetName() const
	{
		return m_name;
	}
}
