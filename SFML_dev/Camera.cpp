#include "Camera.hpp"
#include <iostream>

namespace px
{
	Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED),
		m_mouseSensitivity(SENSITIVTY), m_fov(FOV), m_width(WINDOW_WIDTH), m_height(WINDOW_HEIGHT), m_firstMouse(true)
	{
		m_position = position;
		m_worldUp = up;
		m_yaw = yaw;
		m_pitch = pitch;
		UpdateCameraVectors();
	}

	Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
		m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVTY), m_fov(FOV), m_width(WINDOW_WIDTH), m_height(WINDOW_HEIGHT), m_firstMouse(true)
	{
		m_position = glm::vec3(posX, posY, posZ);
		m_worldUp = glm::vec3(upX, upY, upZ);
		m_yaw = yaw;
		m_pitch = pitch;
		UpdateCameraVectors();
	}

	void Camera::ProcessKeyboard(Camera_Movement direction, float dt)
	{
		float velocity = m_movementSpeed * dt;

		if (direction == FORWARD)
			m_position += m_front * velocity;
		if (direction == BACKWARD)
			m_position -= m_front * velocity;
		if (direction == LEFT)
			m_position -= m_right * velocity;
		if (direction == RIGHT)
			m_position += m_right * velocity;
	}

	void Camera::ProcessMouseMovement(GLFWwindow* window, float xOffset, float yOffset)
	{
		xOffset *= m_mouseSensitivity;
		yOffset *= m_mouseSensitivity;

		m_yaw += xOffset;
		m_pitch += yOffset;

		//Restrict pitch bounds
		if (m_pitch > 89.0f)
			m_pitch = 89.0f;
		if (m_pitch < -89.0f)
			m_pitch = -89.0f;

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		{
			m_firstMouse = false;
			UpdateCameraVectors();
		}
		else
			m_firstMouse = true;
	}

	void Camera::SetFov(float fov)
	{
		m_fov = fov;
	}

	void Camera::SetWidth(unsigned int width)
	{
		m_width = width;
	}

	void Camera::SetHeight(unsigned int height)
	{
		m_height = height;
	}

	void Camera::UpdateCameraVectors()
	{
		//Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		front.y = sin(glm::radians(m_pitch));
		front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
		m_front = glm::normalize(front);

		//Also re-calculate the Right and Up vector
		m_right = glm::normalize(glm::cross(m_front, m_worldUp));
		m_up = glm::normalize(glm::cross(m_right, m_front));
	}

	glm::mat4 Camera::GetProjectionMatrix() const
	{
		return glm::perspective(glm::radians(m_fov), (float)m_width / (float)m_height, NEAR_PLANE, FAR_PLANE);
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		return glm::lookAt(m_position, m_position + m_front, m_up);
	}

	glm::vec3 Camera::GetPosition() const
	{
		return m_position;
	}

	glm::vec3 Camera::GetTarget() const
	{
		return m_position + m_front;
	}

	glm::vec3 Camera::GetUp() const
	{
		return m_up;
	}

	float Camera::GetFov() const
	{
		return m_fov;
	}

	unsigned int Camera::GetWidth() const
	{
		return m_width;
	}

	unsigned int Camera::GetHeight() const
	{
		return m_height;
	}

	bool Camera::GetFirstMouse() const
	{
		return m_firstMouse;
	}
}


