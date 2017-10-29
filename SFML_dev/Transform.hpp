#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace px
{
	class Transform
	{
	public:
		Transform(glm::vec3 position = glm::vec3(), glm::vec3 scale = glm::vec3(1.f), glm::quat orientation = glm::quat());

	public:
		void SetRotationOnAllAxis(glm::vec3 angles);
		void SetPosition(glm::vec3 position);
		void SetRotation(glm::vec3 rotationAxis, float angle);
		void SetRotation(glm::quat quaternion);
		void SetScale(glm::vec3 scale);
		void SetTransform(glm::vec3 position, glm::quat rotation);
		void SetTransform(glm::mat4 transform);
		void SetTransform();
		void SetIdentity();

	public:
		glm::quat GetOrientation() const;
		glm::vec3 GetRotationAngles() const;
		glm::vec3 GetPosition() const;
		glm::vec3 GetScale() const;
		glm::mat4 GetTransform() const;

	private:
		glm::vec3 m_position;
		glm::vec3 m_scale;
		glm::vec3 m_rotationAngles;
		glm::quat m_orientation;
		glm::mat4 m_world;
	};
}

