#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"

#include <memory>
#include <vector>
#include <array>

//Renders scene framebuffer to a texture with 4x MSAA
namespace px
{
	class RenderTexture
	{
	public:
		RenderTexture();

	public:
		void ResizeBuffer(unsigned int x, unsigned int y);
		void BindFrameBuffer();
		void BlitMultiSampledBuffer();
		void UnbindFrameBuffer();

	public:
		unsigned int GetTexture();
		unsigned int GetWidth();
		unsigned int GetHeight();

	private:
		void SetupFrameBuffer();

	private:
		unsigned int m_intermediateFBO;
		unsigned int m_framebuffer;
		unsigned int m_multiSampleTexture;
		unsigned int m_texture;
		unsigned int m_width, m_height;
		unsigned int m_depthbuffer;
		unsigned int m_colorBuffer;
	};

}
