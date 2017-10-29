#include "Transform.hpp"

namespace px
{
	Transform::Transform(glm::vec3 position, glm::vec3 scale, glm::quat orientation) : m_world(), m_position(position), 
																					   m_scale(scale), m_orientation(orientation), m_rotationAngles(0.f)
	{
	}

	void Transform::SetRotationOnAllAxis(glm::vec3 angles)
	{
		m_rotationAngles = angles;
		m_orientation = glm::angleAxis(angles.x, glm::vec3(1, 0, 0)) * glm::angleAxis(angles.y, glm::vec3(0, 1, 0)) * 
						glm::angleAxis(angles.z, glm::vec3(0, 0, 1));
		m_world = m_world * glm::mat4_cast(m_orientation);
	}

	void Transform::SetPosition(glm::vec3 position)
	{
		m_position = position;
		m_world = glm::translate(m_world, m_position);
	}

	void Transform::SetRotation(glm::vec3 rotationAxis, float angle)
	{
		m_orientation = glm::angleAxis(angle, rotationAxis);
		m_world = m_world * glm::mat4_cast(m_orientation);
	}

	void Transform::SetRotation(glm::quat quaternion)
	{
		m_orientation = quaternion;
		m_world = m_world * glm::mat4_cast(m_orientation);
	}

	void Transform::SetScale(glm::vec3 scale)
	{
		m_scale = scale;
		m_world = glm::scale(m_world, m_scale);
	}

	void Transform::SetTransform(glm::vec3 position, glm::quat rotation)
	{
		m_world = glm::translate(m_world, position);
		m_world = m_world * glm::mat4_cast(rotation);
		m_world = glm::scale(m_world, m_scale);
	}

	void Transform::SetTransform(glm::mat4 transform)
	{
		m_world = transform;
	}

	void Transform::SetTransform()
	{
		m_world = glm::translate(m_world, m_position);
		m_world = m_world * glm::mat4_cast(m_orientation);
		m_world = glm::scale(m_world, m_scale);
	}

	void Transform::SetIdentity()
	{
		m_world = glm::mat4();
	}

	glm::quat Transform::GetOrientation() const
	{
		return m_orientation;
	}

	glm::vec3 Transform::GetRotationAngles() const
	{
		return m_rotationAngles;
	}

	glm::vec3 Transform::GetPosition() const
	{
		return m_position;
	}

	glm::vec3 Transform::GetScale() const
	{
		return m_scale;
	}

	glm::mat4 Transform::GetTransform() const
	{
		return m_world;
	}
}
