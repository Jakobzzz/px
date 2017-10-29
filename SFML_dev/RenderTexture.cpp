#include "RenderTexture.hpp"
#include "Camera.hpp"
#include <iostream>

namespace px
{
	RenderTexture::RenderTexture()
	{
		m_width = WINDOW_WIDTH;
		m_height = WINDOW_HEIGHT;

		SetupFrameBuffer();
	}

	unsigned int RenderTexture::GetTexture()
	{
		return m_texture;
	}

	unsigned int RenderTexture::GetWidth()
	{
		return m_width;
	}

	unsigned int RenderTexture::GetHeight()
	{
		return m_height;
	}

	void RenderTexture::ResizeBuffer(unsigned int x, unsigned int y)
	{
		m_width = x;
		m_height = y;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
	}

	void RenderTexture::BindFrameBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
		glViewport(0, 0, m_width, m_height);
	}

	void RenderTexture::BlitMultiSampledBuffer()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_intermediateFBO);
		glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	void RenderTexture::UnbindFrameBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderTexture::SetupFrameBuffer()
	{	
		//Create 4x MSAA framebuffer object
		glGenFramebuffers(1, &m_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

		//Create multisampled texture
		glGenTextures(1, &m_multiSampleTexture);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_multiSampleTexture);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_multiSampleTexture, 0);

		//Create 4x MSAA for depth and stencil buffer
		glGenRenderbuffers(1, &m_colorBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_colorBuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_colorBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Now create another framebuffer which we can transfer the MSAA one to
		glGenFramebuffers(1, &m_intermediateFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_intermediateFBO);

		//Create color attachment texture
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0); //We only need a color buffer

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Intermediate Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

