#pragma once

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 900

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace px
{
	enum Camera_Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	//Default camera values
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float SPEED = 100.f;
	const float SENSITIVTY = 0.4f;
	const float FAR_PLANE = 1000.f;
	const float NEAR_PLANE = 1.f;
	const float FOV = 45.f;

	class Camera
	{
	public:
		Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

	public:
		void ProcessKeyboard(Camera_Movement direction, float dt);
		void ProcessMouseMovement(GLFWwindow* window, float xOffset, float yOffset); 

	public:
		void SetPosition(glm::vec3 position);
		void SetFov(float fov);
		void SetWidth(unsigned int width);
		void SetHeight(unsigned int height);

	public:
		glm::mat4 GetProjectionMatrix() const;
		glm::mat4 GetViewMatrix() const;
		glm::vec3 GetPosition() const;
		glm::vec3 GetTarget() const;
		glm::vec3 GetFront() const;
		glm::vec3 GetUp() const;
		float GetFov() const;
		float GetYaw() const;
		float GetPitch() const;
		unsigned int GetWidth() const;
		unsigned int GetHeight() const;
		bool GetFirstMouse() const;

	private:
		void UpdateCameraVectors();

	private:
		glm::vec3 m_position;
		glm::vec3 m_front;
		glm::vec3 m_up;
		glm::vec3 m_right;
		glm::vec3 m_worldUp;

	private:
		float m_fov;
		float m_yaw;
		float m_pitch;
		float m_movementSpeed;
		float m_mouseSensitivity;
		unsigned int m_width;
		unsigned int m_height;
		bool m_firstMouse;
	};
}


